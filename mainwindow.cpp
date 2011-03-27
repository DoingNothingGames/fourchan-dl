#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    uiConfig = new UIConfig(this);
    uiInfo = new UIInfo(this);
    aui = new ApplicationUpdateInterface(this);

    ui->setupUi(this);
    settings = new QSettings("settings.ini", QSettings::IniFormat);
    ui->tabWidget->removeTab(0);

    loadOptions();
    restoreWindowSettings();
    updateWidgetSettings();

    connect(ui->tabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(closeTab(int)));
    connect(uiConfig, SIGNAL(configurationChanged()), this, SLOT(loadOptions()));
    connect(uiInfo, SIGNAL(newerVersionAvailable(QString)), this, SLOT(newVersionAvailable(QString)));
}

int MainWindow::addTab() {
    int ci;
    UI4chan* w;

    w = new UI4chan(this);

    ci = ui->tabWidget->addTab(w, "no name");
    if (settings->value("options/remember_directory", false).toBool())
        w->setDirectory(defaultDirectory);

    connect(w, SIGNAL(errorMessage(QString)), this, SLOT(displayError(QString)));
    connect(w, SIGNAL(tabTitleChanged(UI4chan*, QString)), this, SLOT(changeTabTitle(UI4chan*, QString)));
    connect(w, SIGNAL(closeRequest(UI4chan*, int)), this, SLOT(processCloseRequest(UI4chan*, int)));
    connect(w, SIGNAL(directoryChanged(QString)), this, SLOT(setDefaultDirectory(QString)));
    connect(w, SIGNAL(createTabRequest(QString)), this, SLOT(createTab(QString)));

    ui->tabWidget->setCurrentIndex(ci);

    changeTabTitle(w, "idle");

    return ci;
}

void MainWindow::createTab(QString s) {
    int index;

    index = addTab();
    ((UI4chan*)ui->tabWidget->widget(index))->setValues(s);
}

void MainWindow::closeTab(int i) {
    UI4chan* w;

    ui->tabWidget->setCurrentIndex(i);
    w = (UI4chan*)ui->tabWidget->widget(i);
    if (w->close())
        ui->tabWidget->removeTab(i);
    else
        qDebug() << "Close widget event not accepted";

    if (ui->tabWidget->count() == 0) {
        addTab();
    }
}

void MainWindow::displayError(QString s) {
    ui->statusBar->showMessage(s, 5000);
}

void MainWindow::showInfo(void) {
    uiInfo->show();
}

void MainWindow::showConfiguration(void) {
    uiConfig->show();
}

void MainWindow::setDefaultDirectory(QString d) {
    if (settings->value("options/remember_directory", false).toBool())
        defaultDirectory = d;
}

void MainWindow::changeTabTitle(UI4chan* w, QString s) {
    int i;

    i = ui->tabWidget->indexOf((QWidget*)w);
    ui->tabWidget->setTabText(i, s);
}

void MainWindow::restoreWindowSettings(void) {
    // Restore window position
    QPoint p;
    QSize s;
    int state;
    int tabCount;

    settings->beginGroup("window");
        p = settings->value("position",QPoint(0,0)).toPoint();
        state = settings->value("state",0).toInt();
        s = settings->value("size",QSize(0,0)).toSize();
    settings->endGroup();

    if (p != QPoint(0,0))
        this->move(p);

    if (s != QSize(0,0))
        this->resize(s);

    if (state != Qt::WindowNoState)
        this->setWindowState((Qt::WindowState) state);


    // tabs
    tabCount = settings->value("tabs/count",0).toInt();
    if (settings->value("options/resume_session", false).toBool() && tabCount > 0) {
        int ci;

        for (int i=0; i<tabCount; i++) {
            ci = addTab();

            ((UI4chan*)ui->tabWidget->widget(ci))->setValues(
                    settings->value(QString("tabs/tab%1").arg(i), ";;;;0;;every 30 seconds;;0").toString()
                    );
        }
    } else {
        addTab();
    }
}

void MainWindow::saveSettings(void) {
    // Window related stuff
    settings->beginGroup("window");
        settings->setValue("position", this->pos());
        if (this->windowState() == Qt::WindowNoState)
            settings->setValue("size", this->size());
        settings->setValue("state", QString("%1").arg(this->windowState()));
    settings->endGroup();

    // Options
    settings->beginGroup("options");
    settings->endGroup();

    // Active tabs
    settings->beginGroup("tabs");
        settings->setValue("count", ui->tabWidget->count());

        for (int i=0; i<ui->tabWidget->count(); i++) {
            settings->setValue(QString("tab%1").arg(i), ((UI4chan*)ui->tabWidget->widget(i))->getValues());
        }
    settings->endGroup();
    settings->sync();
}

void MainWindow::loadOptions(void) {
    settings->beginGroup("options");
        defaultDirectory = settings->value("default_directory", "").toString();
        ui->tabWidget->setTabPosition((QTabWidget::TabPosition)settings->value("tab_position", 3).toInt());
        autoClose = settings->value("automatic_close", false).toBool();
        thumbnailSize.setWidth(settings->value("thumbnail_width", 200).toInt());
        thumbnailSize.setHeight(settings->value("thumbnail_height", 200).toInt());
        maxDownloads = settings->value("concurrent_downloads", 1).toInt();

        updateWidgetSettings();
    settings->endGroup();
}

void MainWindow::processCloseRequest(UI4chan* w, int reason) {
    int i;
    i = ui->tabWidget->indexOf((QWidget*)w);

    if (reason == 404) {
        if (settings->value("options/automatic_close", false).toBool()) {
            closeTab(i);
        }
    }
    else {
        closeTab(i);
    }
}

void MainWindow::updateWidgetSettings(void) {
    for (int i=0; i<ui->tabWidget->count(); i++) {
        ((UI4chan*)ui->tabWidget->widget(i))->updateSettings();
    }
}

void MainWindow::newVersionAvailable(QString v) {
    QProcess process;
    QFileInfo fi;

    ui->statusBar->showMessage(QString("There is a new version (%1) available to download from sourceforge.").arg(v));
#ifdef USE_UPDATER
    switch (QMessageBox::question(0,"New version available",
                                  QString("There is a new version (%1) available from sourceforge<br><a href=\"http://sourceforge.net/projects/fourchan-dl/files/\">sourceforge.net/projects/fourchan-dl</a><br />Do you want to update now?").arg(v),
                                  QMessageBox::Yes | QMessageBox::No)) {
    case QMessageBox::Ok:
    case QMessageBox::Yes:

        fi.setFile(UPDATER_NAME);

        qDebug() << "Startet updater " << fi.absoluteFilePath();

        if (process.startDetached(QString("\"%1\"").arg(fi.absoluteFilePath()))) {
            ui->statusBar->showMessage("Startet updater");
        }
        else {
            ui->statusBar->showMessage("Unable to start process "+fi.absoluteFilePath()+" ("+process.errorString()+")");
        }
        aui->startUpdate(v);
        break;

    case QMessageBox::No:
    default:
        break;
    }
#else
    QMessageBox::information(0,
                             "New version available",
                             QString("There is a new version (%1) available from sourceforge<br><a href=\"http://sourceforge.net/projects/fourchan-dl/files/\">sourceforge.net/projects/fourchan-dl</a>").arg(v),
                             QMessageBox::Ok);
#endif
}

MainWindow::~MainWindow()
{
    saveSettings();

    delete ui;
}
