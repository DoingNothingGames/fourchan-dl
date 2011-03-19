#include "ui4chan.h"
#include "ui_ui4chan.h"

UI4chan::UI4chan(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::UI4chan)
{
    QClipboard *clipboard = QApplication::clipboard();

    pendingThumbnails.clear();

    ui->setupUi(this);
    p = new Parser();
    tnt = new ThumbnailThread();
    timer = new QTimer();
    settings = new QSettings("settings.ini", QSettings::IniFormat);

    setThumbnailSize(QSize(100,100));
    thumbnailsizeLocked = false;

    deleteFileAction = new QAction(QString("Delete File"), this);
    deleteFileAction->setIcon(QIcon(":/icons/resources/remove.png"));
    reloadFileAction = new QAction("Reload File", this);
    reloadFileAction->setIcon(QIcon(":/icons/resources/reload.png"));
    openFileAction = new QAction("Open File", this);
    openFileAction->setIcon(QIcon(":/icons/resources/open.png"));

    ui->progressBar->setMinimum(0);
    ui->progressBar->setMaximum(-1);
    ui->progressBar->setValue(0);
    ui->progressBar->setEnabled(false);
    ui->pbPendingThumbnails->setVisible(false);

    if (clipboard->text().contains("http://boards.4chan.org"))
        ui->leURI->setText(clipboard->text());

    connect(p, SIGNAL(downloadedCountChanged(int)), ui->progressBar, SLOT(setValue(int)));
    connect(p, SIGNAL(totalCountChanged(int)), this, SLOT(setMaxImageCount(int)));
    connect(p, SIGNAL(totalCountChanged(int)), ui->progressBar, SLOT(setMaximum(int)));
    connect(p, SIGNAL(totalCountChanged(int)), ui->pbPendingThumbnails, SLOT(setMaximum(int)));
    connect(p, SIGNAL(finished()), this, SLOT(downloadsFinished()));
    connect(p, SIGNAL(fileFinished(QString)), this, SLOT(createThumbnail(QString)));
    connect(tnt, SIGNAL(thumbnailCreated(QString,QImage)), this, SLOT(addThumbnail(QString,QImage)));
    connect(tnt, SIGNAL(pendingThumbnails(int)), this, SLOT(setPendingThumbnails(int)));

    connect(p, SIGNAL(error(int)), this, SLOT(errorHandler(int)));
    connect(p, SIGNAL(message(QString)), this, SLOT(messageHandler(QString)));
    connect(p, SIGNAL(threadTitleChanged(QString)), ui->lTitle, SLOT(setText(QString)));
    connect(p, SIGNAL(tabTitleChanged(QString)), this, SLOT(setTabTitle(QString)));

    connect(ui->cbOriginalFilename, SIGNAL(stateChanged(int)), p, SLOT(setUseOriginalFilename(int)));

    connect(deleteFileAction, SIGNAL(triggered()), this, SLOT(deleteFile()));
    connect(reloadFileAction, SIGNAL(triggered()), this, SLOT(reloadFile()));
    connect(openFileAction, SIGNAL(triggered()), this, SLOT(openFile()));

    connect(timer, SIGNAL(timeout()), this, SLOT(triggerRescan()));

    setTabTitle("idle");

    loadSettings();
}

UI4chan::~UI4chan()
{
    delete ui;
}

bool UI4chan::setThumbnailSize(QSize s) {
    bool ret;

    ret = false;

    if (!thumbnailsizeLocked) {
        iconSize = s;
        ui->listWidget->setIconSize(iconSize);
        tnt->setIconSize(iconSize);
        ui->listWidget->setGridSize(QSize(iconSize.width()+10,iconSize.height()+20));
        ret = true;
    }

    return ret;
}

void UI4chan::start(void) {
    QDir dir;
    QString savepath;

    running = true;

    if (ui->leURI->text() != "") {
        ui->leURI->setEnabled(false);
        p->setURI(ui->leURI->text());
        savepath = ui->leSavepath->text();

        if (savepath.endsWith("\\")) {
            savepath.chop(1);
            ui->leSavepath->setText(savepath);
        }

        dir.setPath(savepath);

        if (!dir.exists()) {
            QDir d;

            d.mkpath(savepath);
        }

        if (dir.exists()) {
            ui->leSavepath->setEnabled(false);
            p->setSavePath(savepath);
            p->start();

            ui->btnStart->setEnabled(false);
            ui->btnStop->setEnabled(true);
            ui->cbRescan->setEnabled(false);
            ui->comboBox->setEnabled(false);
            ui->progressBar->setEnabled(true);
            ui->cbOriginalFilename->setEnabled(false);

            if (ui->cbRescan->isChecked()) {
                timer->setInterval(timeoutValues.at(ui->comboBox->currentIndex())*1000);

                timer->start();
            }
        }
        else
        {
            emit errorMessage("Directory does not exist / Could not be created");
        }
    }
}

void UI4chan::stop(void) {
    running = false;
    p->stop();
    timer->stop();
    ui->btnStart->setEnabled(true);
    ui->btnStop->setEnabled(false);
    ui->leURI->setEnabled(true);
    ui->leSavepath->setEnabled(true);
    ui->cbRescan->setEnabled(true);
    ui->comboBox->setEnabled(true);
    ui->progressBar->setEnabled(false);
    ui->cbOriginalFilename->setEnabled(true);
}

void UI4chan::chooseLocation(void) {
    QString loc;

    loc = QFileDialog::getExistingDirectory(this, "Choose storage directory", ui->leSavepath->text());

    if (loc != "") {
        if (loc.endsWith("\\"))
            loc.chop(1);
        ui->leSavepath->setText(loc);
        emit directoryChanged(loc);
    }
}

void UI4chan::triggerRescan(void) {
    p->start();
    timer->start();

    setTabTitle("rescanning");
}

void UI4chan::setDownloadedCount(int i) {
    if (ui->progressBar->value() < i)
        ui->progressBar->setValue(i);
}

void UI4chan::createThumbnail(QString s) {
    tnt->addToList(s);
    tnt->createThumbnails();
}

void UI4chan::addThumbnail(QString filename, QImage tn) {
    QListWidgetItem* item;
    QPixmap pixmap;

    pixmap.convertFromImage(tn);
    item = new QListWidgetItem(
            QIcon(pixmap),
            filename,
            ui->listWidget);

    ui->listWidget->addItem(item);
    thumbnailsizeLocked = true;
}

void UI4chan::downloadsFinished() {
    setTabTitle("idling");

    if (!ui->cbRescan->isChecked())
        stop();
}

void UI4chan::on_listWidget_customContextMenuRequested(QPoint pos)
{
    QMenu contextMenu(tr("Context menu"), this);

    contextMenu.addAction(openFileAction);
    contextMenu.addAction(deleteFileAction);
    contextMenu.addAction(reloadFileAction);
    contextMenu.exec(ui->listWidget->mapTo(this,mapToGlobal(pos)));
}

void UI4chan::deleteFile(void) {
    QFile f;
    QString filename;

    filename = ui->listWidget->currentItem()->text();
    if (filename != "") {
        f.setFileName(filename);

        if (f.exists()) {
            f.remove();

            ui->listWidget->takeItem(ui->listWidget->currentRow());
        }
    }
}

void UI4chan::reloadFile(void) {
    QString filename;
    QFile f;

    filename = ui->listWidget->currentItem()->text();
    if (filename != "") {
        f.setFileName(filename);

        if (f.exists()) {
            f.remove();

            ui->listWidget->takeItem(ui->listWidget->currentRow());

            p->reloadFile(filename);
        }
    }
}

void UI4chan::openFile(void) {
    QString filename;

    filename = ui->listWidget->currentItem()->text();
    if (filename != "") {
        QDesktopServices::openUrl(QUrl(QString("file:///%1").arg(filename)));
    }
}

void UI4chan::errorHandler(int err) {
    switch (err) {
    case 404:
        stop();

        setTabTitle("Thread 404'ed");
        emit errorMessage("404 - Page not found");
        emit closeRequest(this);

        break;

    default:
        break;
    }
}

void UI4chan::messageHandler(QString m) {
    emit errorMessage(m);
}

void UI4chan::setTabTitle(QString s) {
    emit tabTitleChanged(this, s);
}

void UI4chan::setDirectory(QString d) {
    ui->leSavepath->setText(d);
}

QString UI4chan::getValues(void) {
    QString ret;
    QStringList list;

    list << ui->leURI->text();
    list << ui->leSavepath->text();
    list << QString("%1").arg(ui->cbRescan->checkState());
    list << QString("%1").arg(ui->comboBox->itemData(ui->comboBox->currentIndex()).toInt());
    list << QString("%1").arg(ui->cbOriginalFilename->checkState());
    list << QString("%1").arg(running);

    ret = list.join(";;");

    return ret;
}

void UI4chan::setValues(QString s) {
    QStringList list;
    int cbIndex;

    list = s.split(";;");

    ui->leURI->setText(list.at(0));
    ui->leSavepath->setText(list.at(1));

    cbIndex = ui->comboBox->findData(list.at(3));
    if (cbIndex != -1)
        ui->comboBox->setCurrentIndex(cbIndex);
    else
        ui->comboBox->setCurrentIndex(0);

    if (list.value(2).toInt() == 0) {
        ui->cbRescan->setChecked(false);
        ui->comboBox->setEnabled(false);
    } else {
        ui->cbRescan->setChecked(true);
        ui->comboBox->setEnabled(true);
    }

    ui->cbOriginalFilename->setCheckState((Qt::CheckState)(list.value(4).toInt()));
    if (list.value(5) == "1")
        start();
}

void UI4chan::setMaxDownloads(int i) {
    p->setMaxDownloads(i);
}

void UI4chan::setMaxImageCount(int i) {
    ui->progressBar->setFormat(QString("%p% (%v/%1)").arg(i));
}

void UI4chan::debugButton(void) {
    tnt->createThumbnails();
}

void UI4chan::closeEvent(QCloseEvent *event)
{
    if (running)
        stop();

    if (tnt->isRunning())
        tnt->terminate();

    p->deleteLater();

    event->accept();
}

void UI4chan::loadSettings() {
    QStringList sl;
    int index, defaultTimeout;

    sl = settings->value("options/timeout_values", (QStringList()<<"30"<<"60"<<"120"<<"300"<<"600")).toStringList();

    ui->comboBox->clear();

    foreach (QString s, sl) {
        int i;
        bool ok;

        i = s.toInt(&ok);
        if (ok) timeoutValues<<i;
    }

    foreach (int i, timeoutValues) {
        int value;
        QString text;

        if (i > 60 && ((i%60) == 0)) {              // Display as minutes
            if (i > 3600 && ((i%3600) == 0)) {      // Display as hours
                if (i > 86400 && ((i%86400)==0)) {  // Display as days
                    value = i/86400;
                    text = "days";
                }
                else {
                    value = i/3600;
                    text = "hours";
                }
            }
            else {
                value = i/60;
                text = "minutes";
            }
        }
        else {
            value = i;
            text = "seconds";
        }

        ui->comboBox->addItem(QString("every %1 %2").arg(value).arg(text), i);
    }

    defaultTimeout = settings->value("options/default_timeout",0).toInt();
    index = ui->comboBox->findData(defaultTimeout);
    if (index != -1) ui->comboBox->setCurrentIndex(index);

    if (defaultTimeout == 0) {
        ui->comboBox->setEnabled(false);
        ui->cbRescan->setChecked(false);
    }
    else {
        ui->comboBox->setEnabled(true);
        ui->cbRescan->setChecked(true);
    }

    setDirectory(settings->value("options/default_directory","").toString());
    useOriginalFilenames(settings->value("options/default_original_filename", false).toBool());
    updateSettings();
}

void UI4chan::updateSettings() {
    setMaxDownloads(settings->value("options/concurrent_downloads",5).toInt());
    setThumbnailSize(QSize(settings->value("options/thumbnail_width",150).toInt(),settings->value("options/thumbnail_height",150).toInt()));
}


void UI4chan::useOriginalFilenames(bool b) {
    ui->cbOriginalFilename->setChecked(b);
}

void UI4chan::setPendingThumbnails(int i) {
    if (i == 0)
        ui->pbPendingThumbnails->setVisible(false);
    else {
        ui->pbPendingThumbnails->setValue(i);
        ui->pbPendingThumbnails->setVisible(true);
    }
}
