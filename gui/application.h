#pragma once

#include <memory>

#include <QApplication>

#include "mainwindow.h"
#include "downloadmanager.h"
//#include "thumbnailthread.h"
#include "thumbnailcreator.h"
#include "foldershortcuts.h"
#include "pluginmanager.h"
#include "uiimageviewer.h"
#include "QsLog.h"
#include "QsLogDest.h"

class QThread;

namespace chandl {
  class Application : public QApplication {
    Q_OBJECT;
  public:
    Application(int& argc, char* argv[]);

    static Application& instance();

  private:
    QString checkEnvironment();

  private:
    std::shared_ptr<DownloadManager> downloadManager;
    std::shared_ptr<ThumbnailCreator> thumbnailCreator;
    std::shared_ptr<FolderShortcuts> folderShortcuts;
    std::shared_ptr<PluginManager> pluginManager;
    std::shared_ptr<UIImageViewer> imageViewer;
    std::shared_ptr<QFile> fLogFile;
    QTextStream logOutput;
    std::shared_ptr<MainWindow> mainWindow;
    std::unique_ptr<QThread> thumbnailThread;
  };
}