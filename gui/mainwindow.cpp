﻿
#include "application.h"

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(std::shared_ptr<DownloadManager> downloadManager_, 
  std::shared_ptr<ThumbnailCreator> thumbnailCreator_, 
  std::shared_ptr<PluginManager> pluginManager_,
  std::shared_ptr<FolderShortcuts> folderShortcuts_,
  QString updaterFileName_,
  QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    downloadManager(downloadManager_),
    thumbnailCreator(thumbnailCreator_),
    pluginManager(pluginManager_),
    folderShortcuts(folderShortcuts_),
    updaterFileName(updaterFileName_)
{
    uiConfig = new UIConfig(folderShortcuts, this);
    uiInfo = new UIInfo(downloadManager, pluginManager, this);
    threadAdder = new UIThreadAdder(folderShortcuts, this);
    aui = new ApplicationUpdateInterface(this);
    requestHandler = new RequestHandler(downloadManager, this);
    blackList = new BlackList(this);

    thumbnailRemoverThread = new QThread();
    thumbnailRemover = new ThumbnailRemover();
    thumbnailRemover->moveToThread(thumbnailRemoverThread);
    connect(thumbnailRemoverThread, SIGNAL(started()), thumbnailRemover, SLOT(removeOutdated()));
    connect(thumbnailRemover, SIGNAL(filesRemoved()), uiConfig, SLOT(thumbnailDeletionFinished()));
    thumbnailRemoverThread->start(QThread::LowPriority);

    overviewUpdateTimer = new QTimer(this);
    overviewUpdateTimer->setInterval(2354);
    overviewUpdateTimer->setSingleShot(true);
    _updateOverview = false;
    _paused = false;

    runUpdate = false;
    checkUpdaterVersion = false;

    connect(this, SIGNAL(removeFiles(QStringList)), thumbnailRemover, SLOT(removeFiles(QStringList)));
    ui->setupUi(this);

    // Adding actions to menu bar
    ui->menuBar->addAction(ui->actionAdd_Tab);
    ui->menuBar->addAction(ui->actionAddMultipleTabs);
    ui->menuBar->addAction(ui->actionTabOverview);

    ui->menuBar->addAction(ui->actionStart_all);
    ui->menuBar->addAction(ui->actionPauseAll);
    ui->menuBar->addAction(ui->actionStop_all);

    ui->menuBar->addAction(ui->actionOpen_Configuration);
//    ui->actionOpen_Configuration->setCheckable(true);

    historyMenu = new QMenu(ui->menuBar);
    historyMenu->setTitle("History");
    historyMenu->setIcon(QIcon(":/icons/resources/remove.png"));
    ui->menuBar->addMenu(historyMenu);

    //ui->menuBar->addAction(ui->actionGetUpdaterVersion);

    ui->menuBar->addAction(ui->actionShowInfo);

    ui->tabWidget->removeTab(0);
    oldActiveTabIndex = 0;
    pendingThumbnailsChanged(0);

    // Thread overview
//    connect(ui->dockWidget, SIGNAL(visibilityChanged(bool)), ui->actionTabOverview, SLOT(setChecked(bool)));
    connect(ui->actionTabOverview, SIGNAL(triggered()), this, SLOT(scheduleOverviewUpdate()));

    loadOptions();
    restoreWindowSettings();
    updateWidgetSettings();

    connect(requestHandler, SIGNAL(response(QUrl,QByteArray,bool)), this, SLOT(processRequestResponse(QUrl,QByteArray,bool)));
//    connect(requestHandler, SIGNAL(responseError(QUrl,int)), this, SLOT(handleRequestError(QUrl,int)));

    connect(ui->tabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(closeTab(int)));
    connect(uiConfig, SIGNAL(configurationChanged()), this, SLOT(loadOptions()));
    connect(uiConfig, SIGNAL(configurationChanged()), blackList, SLOT(loadSettings()));
    connect(uiConfig, SIGNAL(configurationChanged()), downloadManager.get(), SLOT(loadSettings()));
    connect(uiConfig, SIGNAL(deleteAllThumbnails()), thumbnailRemover, SLOT(removeAll()));
    connect(ui->actionStart_all, SIGNAL(triggered()), this, SLOT(startAll()));
    connect(ui->actionStop_all, SIGNAL(triggered()), this, SLOT(stopAll()));
    connect(ui->actionPauseAll, SIGNAL(triggered()), this, SLOT(pauseAll()));
    connect(threadAdder, SIGNAL(addTab(QString)), this, SLOT(createTab(QString)));
    connect(downloadManager.get(), SIGNAL(error(QString)), ui->statusBar, SLOT(showMessage(QString)));
    connect(downloadManager.get(), SIGNAL(finishedRequestsChanged(int)), this, SLOT(updateDownloadProgress()));
    connect(downloadManager.get(), SIGNAL(totalRequestsChanged(int)), this, SLOT(updateDownloadProgress()));
    connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(addThreadOverviewMark(int)));

    connect(overviewUpdateTimer, SIGNAL(timeout()), this, SLOT(overviewTimerTimeout()));
    connect(historyMenu, SIGNAL(triggered(QAction*)), this, SLOT(restoreFromHistory(QAction*)));

    connect(thumbnailCreator.get(), SIGNAL(pendingThumbnails(int)), ui->pbPendingThumbnails, SLOT(setValue(int)));
    connect(thumbnailCreator.get(), SIGNAL(pendingThumbnails(int)), this, SLOT(pendingThumbnailsChanged(int)));

    connect(aui, SIGNAL(connectionEstablished()), this, SLOT(updaterConnected()));
    connect(aui, SIGNAL(updateFinished()), this, SLOT(updateFinished()));
    connect(aui, SIGNAL(updaterVersionSent(QString)), this, SLOT(setUpdaterVersion(QString)));

//    createSupervisedDownload(QUrl("http://sourceforge.net/projects/fourchan-dl/files/"));
    createComponentList();

#ifdef __DEBUG__
    createSupervisedDownload(QUrl("file:d:/Qt/fourchan-dl/webupdate.xml"));
#else
    createSupervisedDownload(QUrl(QString::fromUtf8("http://www.sourceforge.net/projects/fourchan-dl/files/webupdate/webupdate.xml/download")));
#endif

#ifdef Q_OS_WIN
    win7.init((HWND)this->winId());
#endif

    createTrayIcon();

    autosaveTimer = new QTimer(this);
    autosaveTimer->setInterval(1000*60*10);     // 10 Minutes
    autosaveTimer->setSingleShot(false);
    connect(autosaveTimer, SIGNAL(timeout()), this, SLOT(saveSettings()));

    imageViewer = new UIImageViewer(this);
    connect(this, &MainWindow::settingsSaved, imageViewer.data(), &UIImageViewer::saveSettings);
}

MainWindow::~MainWindow()
{
    delete ui;
}

QPointer<UIImageOverview> MainWindow::addTab() {
    int ci;
    auto tab = new UIImageOverview(downloadManager, pluginManager, folderShortcuts, thumbnailCreator, this);
    tab->setBlackList(blackList);

    ci = ui->tabWidget->addTab(tab, "no name");
    if (settings.getRememberDirectory())
        tab->setDirectory(defaultDirectory);

    connect(tab, SIGNAL(errorMessage(QString)), this, SLOT(displayError(QString)));
    connect(tab, SIGNAL(tabTitleChanged(UIImageOverview*, QString)), this, SLOT(changeTabTitle(UIImageOverview*, QString)));
    connect(tab, SIGNAL(closeRequest(UIImageOverview*, int)), this, SLOT(processCloseRequest(UIImageOverview*, int)));
    connect(tab, SIGNAL(directoryChanged(QString)), this, SLOT(setDefaultDirectory(QString)));
    connect(tab, SIGNAL(createTabRequest(QString)), this, SLOT(createTab(QString)));
    connect(tab, SIGNAL(removeFiles(QStringList)), this, SIGNAL(removeFiles(QStringList)));
    connect(tab, SIGNAL(changed()), this, SLOT(scheduleOverviewUpdate()));

    changeTabTitle(tab, "idle");

    return tab;
}

QPointer<UIImageOverview> MainWindow::addForegroundTab() {
  QPointer<UIImageOverview> tab;
  QStringList sl;

    if (threadExists(QApplication::clipboard()->text())) {
        tab = addTab();
        sl = tab->getValues().split(";;");
        sl.replace(0, "");
        tab->setValues(sl.join(";;"));
    }
    else {
        tab = addTab();
    }
    ui->tabWidget->setCurrentWidget(tab.data());
    return tab;
}

QPointer<UIImageOverview> MainWindow::getTabOverview(int index)
{
  auto widget = ui->tabWidget->widget(index);
  return qobject_cast<UIImageOverview*>(widget);
}

void MainWindow::createTab(QString values) {
    int index;
    UIImageOverview* w;
    QStringList sl;

    sl = values.split(";;");

    if (!threadExists(sl.at(0))) {
        auto tab = addTab();
        tab->setValues(values);
        tab->setAttribute(Qt::WA_DeleteOnClose, true);
    }
    else {
        QLOG_INFO() << "MainWindow :: Prevented opening of thread" << sl.at(0) << "because it's already open.";
    }
}

void MainWindow::closeTab(int i) {
    UIImageOverview* w;

//    ui->tabWidget->setCurrentIndex(i);
    w = (UIImageOverview*)ui->tabWidget->widget(i);

    addToHistory(w->getValues(), w->getTitle());

    if (w->close()) {
        ui->tabWidget->removeTab(i);
        w->deleteLater();
        QLOG_TRACE() << "MainWindow :: widget" << i << "closed";
    }
    else {
        QLOG_WARN() << "MainWindow :: Close widget event not accepted";
    }

    if (ui->tabWidget->count() == 0) {
        QLOG_TRACE() << __func__ << "Adding new tab, because no tab is left";
        addTab();
    }
}

void MainWindow::displayError(QString s) {
    ui->statusBar->showMessage(s, 10000);
}

void MainWindow::showInfo(void) {
    uiInfo->show();
}

void MainWindow::showConfiguration(void) {
    uiConfig->show();
}

void MainWindow::setDefaultDirectory(QString d) {
    if (settings.getRememberDirectory())
        defaultDirectory = d;
}

void MainWindow::changeTabTitle(UIImageOverview* w, QString s) {
    int i;

    i = ui->tabWidget->indexOf((QWidget*)w);
    ui->tabWidget->setTabText(i, s);
}

void MainWindow::restoreWindowSettings(void) {
    // Restore window position
    auto pos = settings.getWindowPosition();
    auto size = settings.getWindowSize();
    auto widgetstate = settings.getWindowWidgetState();
    auto state = settings.getWindowState();

    if (pos != QPoint(0,0))
        this->move(pos);

    if (size != QSize(0,0))
        this->resize(size);

    if (state != Qt::WindowNoState)
        this->setWindowState((Qt::WindowState) state);

    if (!widgetstate.isEmpty())
        this->restoreState(widgetstate);

//    ui->threadOverview->setVisible(settings->value("thread_overview/visible", true).toBool());
    ui->actionTabOverview->setChecked(settings.getThreadOverviewVisible());
}

void MainWindow::restoreTabs() {
    int tabCount;

    tabCount = settings.getTabsCount();
    downloadManager->pauseDownloads();

    ui->pbOpenRequests->setMaximum(tabCount);
    ui->pbOpenRequests->setValue(0);
    ui->pbOpenRequests->setFormat("Opening tab %v/%m");
    ui->tabWidget->setVisible(false);
    if (settings.getResumeSession() && tabCount > 0) {
        for (int i = 0; i < tabCount; i++) {
            auto tab = addTab();
            tab->setValues(settings.getTabValues(i));
            ui->pbOpenRequests->setValue((i+1));
        }
    } else {
        addTab();
    }

    ui->tabWidget->setVisible(true);
    ui->pbOpenRequests->setVisible(false);
//    ui->pbOpenRequests->setFormat("%v/%m (%p%) requests finished");
//    ui->pbOpenRequests->setValue(0);
//    ui->pbOpenRequests->setMaximum(0);
    downloadManager->resumeDownloads();
}

void MainWindow::saveSettings(void) {
    QLOG_INFO() << "MainWindow :: Saving settings";
    // Window related stuff
    settings.setWindowPosition(this->pos());
    if (this->windowState() == Qt::WindowNoState)
      settings.setWindowSize(this->size());
    settings.setWindowState(static_cast<int>(this->windowState()));
    settings.setWindowWidgetState(this->saveState());


    // Dock widget
    settings.setThreadOverviewImagesWidth(ui->threadOverview->columnWidth(TOC_IMAGES_WIDTH));
    settings.setThreadOverviewNameWidth(ui->threadOverview->columnWidth(TOC_NAME_WIDTH));
    settings.setThreadOverviewStatusWidth(ui->threadOverview->columnWidth(TOC_STATUS_WIDTH));
    settings.setThreadOverviewUriWidth(ui->threadOverview->columnWidth(TOC_URI_WIDTH));
    settings.setThreadOverviewVisible(ui->threadOverview->isVisible());

    // Active tabs
    settings.clearTabs();
    settings.setTabsCount(ui->tabWidget->count());
    for (int i = 0; i < ui->tabWidget->count(); i++) {
      auto tab = getTabOverview(i);
      settings.setTabValues(i, tab->getValues());
    }

    settings.setDownloadedFilesStatistic(downloadManager->getStatisticsFiles());
    settings.setDownloadedKBytesStatistic(downloadManager->getStatisticsKBytes());

    settings.sync();

    if (settings.getStatus() == QSettings::NoError) {
        QLOG_INFO() << "MainWindow :: Settings saved successfully";
    }
    else {
        QLOG_ERROR() << "MainWindow :: Saving settings failed";
        QLOG_ERROR() << "MainWindow ::  error: " << settings.getStatus();
    }
    emit settingsSaved();
}

void MainWindow::loadOptions(void) {

  defaultDirectory = settings.getDefaultDirectory();
  ui->tabWidget->setTabPosition(static_cast<QTabWidget::TabPosition>(settings.getTabPosition()));
  autoClose = settings.getAutoClose();
  thumbnailSize = settings.getThumbnailSize();
  maxDownloads = settings.getMaxDownloads();

  updateWidgetSettings();

  thumbnailCreator->setIconSize(settings.getThumbnailSize());


  QNetworkProxy proxy;

  if (settings.getNetworkUseProxy()) {
    proxy.setType(static_cast<QNetworkProxy::ProxyType>(settings.getNetworkProxyType()));
    proxy.setHostName(settings.getNetworkProxyHostName());
    proxy.setPort(settings.getNetworkProxyPort());
    if (settings.getNetworkProxyAuth()) {
      proxy.setUser(settings.getNetworkProxyUser());
      proxy.setPassword(settings.getNetworkProxyPass());
    }
  }
  else {
    proxy.setType(QNetworkProxy::NoProxy);
  }

  QNetworkProxy::setApplicationProxy(proxy);

  // Dock widget
  ui->threadOverview->setColumnWidth(TOC_IMAGES_WIDTH, settings.getThreadOverviewImagesWidth());
  ui->threadOverview->setColumnWidth(TOC_NAME_WIDTH, settings.getThreadOverviewNameWidth());
  ui->threadOverview->setColumnWidth(TOC_STATUS_WIDTH, settings.getThreadOverviewStatusWidth());
  ui->threadOverview->setColumnWidth(TOC_URI_WIDTH, settings.getThreadOverviewUriWidth());
}

void MainWindow::processCloseRequest(UIImageOverview* w, int reason) {
    int i;
    i = ui->tabWidget->indexOf((QWidget*)w);

    QLOG_TRACE() << __func__ << "i: " << i << ", reason " << reason;
    if (reason == 404) {
        if (settings.getAutoClose()) {
            closeTab(i);
        }
    }
    else {
        closeTab(i);
    }
}

void MainWindow::processRequestResponse(QUrl url, QByteArray ba, bool cached) {

    if (url.toString().contains("webupdate.xml")) {
        checkForUpdates(QString(ba));
    }
    else {
        QLOG_WARN() << "MainWindow :: MainWindow should only ask for webupdate.xml but response was for" << url.toString();
    }
}

void MainWindow::updateWidgetSettings(void) {
    for (int i=0; i<ui->tabWidget->count(); i++) {
        ((UIImageOverview*)ui->tabWidget->widget(i))->updateSettings();
    }
}

void MainWindow::askForUpdate() {
#ifdef USE_UPDATER
    QProcess process;
    QFileInfo fi;
#endif

    QString msg;

    msg = "There are new components available:";

    for (int i=0; i<updateableComponents.count(); i++) {
        component_information c;

        c = components.value(updateableComponents.at(i));

        msg.append("<br>");
        msg.append(QString("&nbsp;&nbsp;%1:%2 (installed: %3, available: %4)").arg(c.type).arg(c.componentName).arg(c.version).arg(c.remote_version));
    }
//    ui->statusBar->showMessage(msg);

#ifdef USE_UPDATER
    msg.append("<br>Do you want to update now?");
    auto& app = chandl::Application::instance();

    switch (QMessageBox::question(0,"New version available",
                                  msg,
                                  QMessageBox::Yes | QMessageBox::No)) {
    case QMessageBox::Ok:
    case QMessageBox::Yes:
        saveSettings();
        fi.setFile(updaterFileName);

        QLOG_INFO() << "MainWindow :: Starting updater " << fi.absoluteFilePath();

        if (process.startDetached(QString("\"%1\"").arg(fi.absoluteFilePath()))) {
            ui->statusBar->showMessage("Starting updater");
        }
        else {
            ui->statusBar->showMessage("Unable to start process "+fi.absoluteFilePath()+" ("+process.errorString()+")");
        }
        break;

        ui->menuBar->removeAction(ui->actionAskForUpdate);

    case QMessageBox::No:
    default:
        break;
    }
#else
    msg.append("<br><a href=\"http://sourceforge.net/projects/fourchan-dl/files/\">http://sourceforge.net/projects/fourchan-dl</a>");

    QMessageBox::information(0,
                             "New version available",
                             msg,
                             QMessageBox::Ok);
#endif
}

void MainWindow::getUpdaterVersion() {
#ifdef USE_UPDATER
    auto& app = chandl::Application::instance();
    QProcess process;
    QFileInfo fi;

    fi.setFile(updaterFileName);

    QLOG_INFO() << "MainWindow :: Starting updater " << fi.absoluteFilePath();

    checkUpdaterVersion = true;
    if (process.startDetached(QString("\"%1\"").arg(fi.absoluteFilePath()))) {
        ui->statusBar->showMessage("Starting updater for version check");
    }
    else {
        ui->statusBar->showMessage("Unable to start process "+fi.absoluteFilePath()+" ("+process.errorString()+")");
        checkUpdaterVersion = false;
    }

#endif
}

void MainWindow::getConsoleVersion() {
    QProcess process;

    if (process.startDetached("fourchan-dl-console.exe --version")) {
        ui->statusBar->showMessage("Starting fourchan-dl-console for version check", 2000);
    }
}

void MainWindow::startAll() {
    ui->pbOpenRequests->setFormat("Starting Thread %v/%m (%p%)");
    ui->pbOpenRequests->setMaximum(ui->tabWidget->count());

    for (int i=0; i<ui->tabWidget->count(); i++) {
        ((UIImageOverview*)ui->tabWidget->widget(i))->start();
        ui->pbOpenRequests->setValue((i+1));
    }
}

void MainWindow::pauseAll() {
    if (_paused) {
        ui->actionPauseAll->setIcon(QIcon(":/icons/resources/media-playback-pause.png"));
        downloadManager->resumeDownloads();
        thumbnailCreator->resume();
    }
    else {
        ui->actionPauseAll->setIcon(QIcon(":/icons/resources/media-playback-pause-red.png"));
        downloadManager->pauseDownloads();
        thumbnailCreator->pause();
    }

    _paused = !_paused;
}

void MainWindow::stopAll() {
    ui->pbOpenRequests->setFormat("Stopping Thread %v/%m (%p%)");
    ui->pbOpenRequests->setMaximum(ui->tabWidget->count());

    for (int i=0; i<ui->tabWidget->count(); i++) {
        ((UIImageOverview*)ui->tabWidget->widget(i))->stop();
        ui->pbOpenRequests->setValue((i+1));
    }
}

void MainWindow::pendingThumbnailsChanged(int i) {
    if (i > ui->pbPendingThumbnails->maximum())
        ui->pbPendingThumbnails->setMaximum(i);

    if (i == 0) {
        ui->pbPendingThumbnails->setVisible(false);
        ui->pbPendingThumbnails->setMaximum(0);
    }
    else if (ui->pbPendingThumbnails->maximum() > 4)
        ui->pbPendingThumbnails->setVisible(true);

}

void MainWindow::addMultipleTabs() {
    threadAdder->show();
}

void MainWindow::showTab(QTreeWidgetItem* item, int idx) {
    int index;

    index = ui->threadOverview->indexOfTopLevelItem(item);

    if (index != -1) {
        ui->tabWidget->setCurrentIndex(index);
        ((UIImageOverview*)(ui->tabWidget->currentWidget()))->threadViewed();
    }
}

void MainWindow::scheduleOverviewUpdate() {
    if (!overviewUpdateTimer->isActive()) {
        updateThreadOverview();
        overviewUpdateTimer->start();
    }
    else {
         _updateOverview = true;
    }
}

void MainWindow::overviewTimerTimeout() {
    updateThreadOverview();
    if (_updateOverview) {
        _updateOverview = false;
        overviewUpdateTimer->start();
    }
}

void MainWindow::updateThreadOverview() {
//    QList<QTreeWidgetItem *> items;
    QStringList sl;

    if (ui->threadOverview->isVisible()) {
//        QLOG_TRACE() << "MainWindow :: updating thread overview";
//        ui->threadOverview->clear();

        for (int i=0; i<ui->tabWidget->count(); i++) {
            UIImageOverview* tab;
            QTreeWidgetItem* item;
            sl.clear();

            tab = (UIImageOverview*)(ui->tabWidget->widget(i));
            sl << tab->getTitle();
            sl << QString("%1/%2").arg(tab->getDownloadedImagesCount()).arg(tab->getTotalImagesCount());
            sl << tab->getStatus();
            sl << tab->getURI();

            QLOG_DEBUG() << __func__ << "::" << i << ":" << sl;

            if (ui->threadOverview->topLevelItemCount() > i) {                   // If there is an entry for the i-th tab
                item = ui->threadOverview->topLevelItem(i);     //  change its content
                QFont f = item->font(0);
                for (int k=0; k<4; k++) {

                    if (k == 0) {
                        if (tab->hasNewImages()) {
                            item->setTextColor(0, Qt::darkGreen);
                            f.setBold(true);
                            item->setFont(0, f);
                            QLOG_TRACE() << __func__ << "tab" << i << "Changing font to bold/red";
                        }
                        else {
                            item->setTextColor(0, Qt::black);
                            f.setBold(false);
                            item->setFont(0, f);
                            QLOG_TRACE() << __func__ << "tab" << i << "Changing font to normal";
                        }
                    }
                    item->setText(k, sl.at(k));
                }
            }
            else {                                              // Otherwise create a new one and append it
                ui->threadOverview->addTopLevelItem(new QTreeWidgetItem(ui->threadOverview, sl));
            }
        }

        // Remove obsolete rows of overview (if any)
        if (ui->threadOverview->topLevelItemCount() > ui->tabWidget->count()) {
            for (int i=ui->threadOverview->topLevelItemCount(); i>=ui->tabWidget->count(); --i) {
                ui->threadOverview->takeTopLevelItem(i);
            }
        }
//        ui->threadOverview->insertTopLevelItems(0, items);
    }
}

void MainWindow::debugButton() {
    updateThreadOverview();
}

bool MainWindow::threadExists(QString url) {
    bool ret;
    int count;
    QString widgetUri;

    ret = false;
    count = 0;

    for (int i=0; i<ui->tabWidget->count(); i++) {
        widgetUri = ((UIImageOverview*)ui->tabWidget->widget(i))->getURI();

        if ( widgetUri.left(widgetUri.lastIndexOf("#")) == url.left(url.lastIndexOf("#"))) {
            count++;
        }
    }

    if (count > 0)
        ret = true;

    return ret;
}

void MainWindow::addToHistory(QString s, QString title="") {
    QStringList sl;
    QString key;
    QString actionTitle;
    QAction* a;

    sl = s.split(";;");
    if (sl.count() > 0) {
        key = sl.at(0);
        if (!key.isEmpty()) {
            historyList.insert(key, s);

            if (title.isEmpty())
                actionTitle = QString("%1 -> %2").arg(key).arg(sl.at(1));
            else
                actionTitle = QString("%1 (%2) -> %3").arg(key).arg(title).arg(sl.at(1));

            a = historyMenu->addAction(actionTitle);
            a->setIcon(QIcon(":/icons/resources/reload.png"));
            a->setStatusTip(QString("Reopen thread %1").arg(key));
            a->setToolTip(key); // Abusing the tooltip for saving the historyList key
            QLOG_TRACE() << "MainWindow :: " << QString("Adding (%1, %2) to history").arg(key).arg(s);
        }
    }
}

void MainWindow::removeFromHistory(QString key) {
    historyList.remove(key);
    QLOG_TRACE() << "MainWindow :: " << QString("Removing (%1) from history").arg(key);
}

void MainWindow::restoreFromHistory(QAction* a) {
    QString key;

    key = a->toolTip();

    if (historyList.count(key) > 0)
        createTab(historyList.value(key, ""));

    removeFromHistory(key);
    historyMenu->removeAction(a);
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    QMainWindow::keyPressEvent(event);

    if (event->key() == Qt::Key_W && (event->modifiers() & Qt::ControlModifier) == Qt::ControlModifier) {
        closeTab(ui->tabWidget->currentIndex());
    }
}

void MainWindow::createSupervisedDownload(QUrl url) {
    if (url.isValid()) {
        requestHandler->request(url, 0);
    }
}

void MainWindow::removeSupervisedDownload(QUrl url) {
    requestHandler->cancel(url);
}

void MainWindow::checkForUpdates(QString xml) {
    QRegExp rx(QString("<%1>([\\w\\W]+[^<])+</%1>").arg(UPDATE_TREE), Qt::CaseInsensitive, QRegExp::RegExp2);
    QRegExp rxFile("<file name=\"([^\\\"]+)\" filename=\"([^\\\"]+)\" type=\"([^\\\"]+)\" version=\"([\\w\\.]*)\" source=\"([\\w:\\-\\./\\+]+)\" target=\"([^\\\"]+)\" />", Qt::CaseInsensitive, QRegExp::RegExp2);
    int pos, posFile;
    QStringList res, resFile;
    QMap<QString, component_information> comp;
    component_information c, local, remote;
    QList<QString> foundComponents;

    QLOG_ALWAYS() << __func__ << ":: Checking for updates on release tree " << UPDATE_TREE;

    pos = rx.indexIn(xml);
    res = rx.capturedTexts();

    if (res.count() > 0 && pos != -1) {

        posFile = 0;
        while (posFile != -1) {
            posFile = rxFile.indexIn(res.at(1), posFile+1);
            resFile = rxFile.capturedTexts();

            if (resFile.at(1) != "") {

                c.componentName = resFile.at(1);
                c.filename = resFile.at(2);
                c.type = resFile.at(3);
                c.version = resFile.at(4);
                c.src = resFile.at(5);
                c.target = resFile.at(6);

                if (c.filename == APP_NAME && c.type == "executable") {
                    uiInfo->setCurrentVersion(c.version);
                }

                comp.insert(QString("%1:%2").arg(c.type).arg(c.filename), c);
            }
        }
    }

    foundComponents = comp.keys();

    foreach (QString key, foundComponents) {
        local = components.value(key);
        remote = comp.value(key);

        if (local.filename == remote.filename) {
            if (checkIfNewerVersion(remote.version, local.version)) {
                QLOG_INFO() << "MainWindow :: New version available for " << local.type << ":" << local.filename;
                updateableComponents.append(key);
                local.src = remote.src;
                local.target = remote.target;
                local.remote_version = remote.version;
                components.insert(key,local);
                runUpdate = true;
            }
        }
        else if (remote.filename != "" && local.filename == "") {
            // New component!
            remote.remote_version = remote.version;
            remote.version = "no";
            components.insert(key, remote);
            runUpdate = true;
            updateableComponents.append(key);
        }
    }

    if (runUpdate) {
        ui->menuBar->addAction(ui->actionAskForUpdate);
    }
    QLOG_TRACE() << "MainWindow :: " << xml;
}

bool MainWindow::checkIfNewerVersion(QString _new, QString _old) {
    bool ret;
    QStringList newVersion, oldVersion;

    ret = false;

    newVersion = _new.split(".");
    oldVersion = _old.split(".");

    for (int i=0; i<newVersion.count(); i++) {
        if (newVersion.value(i).toInt() > oldVersion.at(i).toInt()) {
            ret = true;
            break;
        }
        else {
            if (oldVersion.at(i).toInt() > newVersion.value(i).toInt()) {
                break;
            }
        }
    }

    return ret;
}

void MainWindow::createComponentList() {
    component_information c;
    QStringList plugins;
    QStringList qtFiles;
    QStringList neededLibraries;

#ifdef USE_UPDATER
    QString version;
#endif

#ifdef Q_OS_WIN32
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    qtFiles << "Qt5Core" << "Qt5Gui" << "Qt5Widgets" << "Qt5Network" << "Qt5Xml";
    neededLibraries << "libeay32.dll" << "ssleay32.dll" << "libstdc++-6.dll" << "imageformats/qgif.dll"
                    << "imageformats/qico.dll" << "imageformats/qjpeg.dll" << "imageformats/qsvg.dll"
                    << "libgcc_s_sjlj-1.dll" << "libwinpthread-1.dll" << "platforms/qwindows.dll";

#else
    qtFiles << "QtCore4" << "QtGui4" << "QtNetwork4" << "QtXml4";
    neededLibraries << "libeay32.dll" << "ssleay32.dll" << "imageformats/qgif4.dll"
                    << "imageformats/qico4.dll" << "imageformats/qjpeg4.dll"
                    << "imageformats/qmng4.dll" << "imageformats/qsvg4.dll"
                    << "imageformats/qtiff4.dll";
#endif
#endif

    components.clear();

    c.filename = APP_NAME;
    c.componentName = "Main program";
    c.type = "executable";
    c.version = PROGRAM_VERSION;

    components.insert(QString("%1:%2").arg(c.type).arg(c.filename), c);

    foreach (QString libFile, neededLibraries) {
        c.filename = libFile;
        c.componentName = libFile;
        c.type = "library";
        c.version = "";
        QLOG_ALWAYS() << "Mainwidow :: createComponentList :: Checking for " << QString("%1/%2").arg(QApplication::applicationDirPath()).arg(libFile);
        if (QFile::exists(QString("%1/%2").arg(QApplication::applicationDirPath()).arg(libFile))) {
            components.insert(QString("%1:%2").arg(c.type).arg(c.filename), c);
        }
        else {
            QLOG_WARN() << "Mainwidow :: createComponentList :: Needed library " << libFile << "does not exist.";
        }
    }

    if (QFile::exists(QString("%1/%2").arg(QApplication::applicationDirPath()).arg(CONSOLE_APPNAME))) {
        c.filename = CONSOLE_APPNAME;
        c.componentName = "Console";
        c.type = "executable";
        c.version = settings.getConsoleVersion();
        components.insert(QString("%1:%2").arg(c.type).arg(c.filename), c);
    }

#ifdef USE_UPDATER
    // TODO: use updaterFileName (but without the path)
    c.filename = "upd4t3r.exe";
    c.componentName = "Updater";
    c.type = "executable";
    version = settings.getUpdaterVersion();

    if (version == "unknown" && c.filename.contains("upd4t3r")) {
        // No version information in settings file, but new updater executable present
        // means a freshly updated system. Assume version 1.1
        version = "1.2";
        settings.setUpdaterVersion(version);
    }

    c.version = version;


    components.insert(QString("%1:%2").arg(c.type).arg(c.filename), c);
#endif

    foreach (QString f, qtFiles) {
#ifdef Q_OS_WIN32
        c.filename = QString("%1.dll").arg(f);
#else
#ifdef Q_OS_LINUX
        c.filename = f;
#endif
#endif
        c.componentName = f;
        c.type = "qt";
        c.version = qVersion();

        components.insert(QString("%1:%2").arg(c.type).arg(c.filename), c);
    }

    plugins = pluginManager->getAvailablePlugins();

    foreach (QString pluginName, plugins) {
        c = pluginManager->getInfo(pluginName);
        components.insert(QString("%1:%2").arg(c.type).arg(c.filename), c);
    }

    uiInfo->loadComponentInfo(components);
}

void MainWindow::updaterConnected() {
    // We requested the update executable to start and now we have a connection
    QStringList fileList;
    component_information c;

    if (checkUpdaterVersion) {
        QLOG_INFO() << "Mainwidow :: updaterConnected :: Just checking updater version";
        aui->closeUpdaterExe();
        checkUpdaterVersion = false;
    }
    else {
        if (runUpdate) {
            thumbnailCreator->pause();
            downloadManager->pauseDownloads();

            foreach (QString component, updateableComponents) {
                c = components.value(component);
                fileList.append(QString("%1->%2").arg(c.src).arg(c.target));
            }
            aui->addFiles(fileList);
            aui->startUpdate();
        }
        else {
            QLOG_WARN() << "Mainwidow :: updaterConnected :: Updater connected, but I didn't know what to do";
            aui->closeUpdaterExe();
        }
    }
}

void MainWindow::updateFinished() {
    aui->closeUpdaterExe();
    aui->exchangeFiles();

    // We may have updated versions for the updater.exe or console.exe
    getUpdaterVersion();
    getConsoleVersion();
}

QList<component_information> MainWindow::getComponents() {
    QList<component_information> ret;
    QStringList keys;

    keys = components.keys();

    foreach(QString key, keys) {
        ret.append(components.value(key));
    }

    return ret;
}

void MainWindow::setUpdaterVersion(QString v) {
  settings.setUpdaterVersion(v);
}

void MainWindow::updateDownloadProgress() {
#ifdef Q_OS_WIN
    int total, finished;

    total = downloadManager->getTotalRequests();
    finished = downloadManager->getFinishedRequests();

    if (finished != total || total != 0) {
        win7.setProgressValue(finished, total);
        win7.setProgressState(win7.Normal);
    }
    else {
        win7.setProgressState(win7.NoProgress);
    }

#endif
}

#ifdef Q_OS_WIN
bool MainWindow::winEvent(MSG *message, long *result)
{
    return win7.winEvent(message, result);
}
#endif

void MainWindow::createTrayActions()
{
    restoreAction = new QAction(tr("&Restore"), this);
    restoreAction->setIcon(QIcon(":/icons/resources/rotateCW.png"));
    connect(restoreAction, SIGNAL(triggered()), this, SLOT(showNormal()));

    quitAction = new QAction(tr("&Quit"), this);
    quitAction->setIcon(QIcon(":/icons/resources/close.png"));
    connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));
}

void MainWindow::createTrayIcon() {
    trayIcon = new QSystemTrayIcon(this);

    if (QSystemTrayIcon::isSystemTrayAvailable() && settings.getCloseToTray()) {
        createTrayActions();

        trayIconMenu = new QMenu(this);
        trayIconMenu->addAction(restoreAction);
        trayIconMenu->addSeparator();
        trayIconMenu->addAction(quitAction);

        trayIcon->setContextMenu(trayIconMenu);

        QApplication::setQuitOnLastWindowClosed(false);

        trayIcon->setIcon(QIcon(":/icons/resources/4chan.ico"));
        trayIcon->show();

        connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(trayIconActivated(QSystemTrayIcon::ActivationReason)));
    }
}

void MainWindow::removeTrayIcon() {
    if (QSystemTrayIcon::isSystemTrayAvailable() && settings.getCloseToTray() && trayIcon) {
        trayIcon->hide();
    }
}

void MainWindow::trayIconActivated(QSystemTrayIcon::ActivationReason ar) {
    if (ar == QSystemTrayIcon::DoubleClick) {
        restoreAction->trigger();
    }
}

void MainWindow::toggleThreadOverview() {
    if (ui->dockWidget->isVisible()) {
        ui->dockWidget->setVisible(false);
    }
    else {
        ui->dockWidget->setVisible(true);
    }
}

void MainWindow::aboutToQuit() {
    downloadManager->pauseDownloads();
    saveSettings();
    removeTrayIcon();
    cleanThreadCache();
    //thumbnailRemover->deleteLater();
    thumbnailRemoverThread->exit(0);

    emit quitAll();
}

void MainWindow::removeThreadOverviewMark() {
    for (int i=0; i<ui->threadOverview->topLevelItemCount(); i++) {
        ui->threadOverview->topLevelItem(i)->setIcon(0, QIcon());
    }
}

void MainWindow::addThreadOverviewMark(QTreeWidgetItem* item) {
    removeThreadOverviewMark();

    item->setIcon(0, QIcon(":/icons/resources/go-next.png"));
}

void MainWindow::addThreadOverviewMark(int index) {
    QList<QTreeWidgetItem*> foundItems;

    if (ui->tabWidget->count() > 0) {
        foundItems = ui->threadOverview->findItems(((UIImageOverview*)(ui->tabWidget->widget(index)))->getURI(), Qt::MatchExactly, 3 );
        if (foundItems.count() == 1) {
            addThreadOverviewMark(foundItems.at(0));
        }
    }
}

void MainWindow::cleanThreadCache() {
    QStringList threadCachesToRemove;
    QStringList dirContents;
    QString cacheFolder;
    QString url;
    QString cacheFile;
    QDir dir;

    if (settings.getUseThreadCache()) {
        cacheFolder = settings.getThreadCachePath();

        if (!cacheFolder.isEmpty()) {
            dir.setPath(cacheFolder);
            if (dir.isReadable()) {
                dirContents = dir.entryList(QStringList() << "*.tcache");
            }
        }

        foreach (cacheFile, dirContents) {
            threadCachesToRemove << QString("%1/%2").arg(cacheFolder, cacheFile);
        }

        for (int i=0; i<ui->tabWidget->count(); i++) {
            url = ((UIImageOverview*)ui->tabWidget->widget(i))->getURI();
            cacheFile = downloadManager->getFilenameForURL(QUrl(url));
            threadCachesToRemove.removeAll(cacheFile);
        }

        foreach (cacheFile, threadCachesToRemove) {
            QFile::remove(cacheFile);
        }
    }
}

void MainWindow::markAllViewed() {
    for (int i=0; i<ui->tabWidget->count(); i++) {
        ((UIImageOverview*)ui->tabWidget->widget(i))->threadViewed();
    }
}
