
#include <cassert>

#include <QtDebug>
#include <QFile>
#include <QDateTime>
#include <QThread>

#include "application.h"

namespace chandl {
  namespace {
    bool checkIfNewerVersion(QString _new, QString _old) {
      bool ret;
      QStringList newVersion, oldVersion;

      ret = false;

      newVersion = _new.split(".");
      oldVersion = _old.split(".");

      for (int i = 0; i < newVersion.count(); i++) {
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
  }

  Application::Application(int& argc, char* argv[])
    : QApplication(argc, argv) {
    QSettings settings("settings.ini", QSettings::IniFormat);
    int logLevel;

    //#if QT_VERSION >= 0x050000
    //#ifdef Q_OS_WIN
    //    a.setStyle("windowsvista");
    //#else
    //    a.setStyle("fusion");
    //#endif
    //#else
    //    a.setStyle("plastique");
    //#endif

        // init the logging mechanism
    auto& logger = QsLogging::Logger::instance();
    logger.setLoggingLevel(QsLogging::TraceLevel);

    const QString sLogPath(QDir(applicationDirPath()).filePath("fourchan-dl.log"));

    auto fileDestination = QsLogging::DestinationFactory::MakeFileDestination(sLogPath);
    auto debugDestination = QsLogging::DestinationFactory::MakeDebugOutputDestination();
    logger.addDestination(debugDestination);
    logger.addDestination(fileDestination);

#ifdef __DEBUG__
    logger.setLoggingLevel(QsLogging::TraceLevel);
#else
    logger.setLoggingLevel(QsLogging::WarnLevel);
#endif

    logLevel = settings.value("options/log_level", -1).toInt();
    if (logLevel != -1) {
      logger.setLoggingLevel((QsLogging::Level)logLevel);
      QLOG_ALWAYS() << "APP :: Setting logging level to " << logLevel;
    }

    QLOG_INFO() << "APP :: Program started";
    QLOG_INFO() << "APP :: Built with Qt" << QT_VERSION_STR << "running on" << qVersion();

    auto updaterFileName = checkEnvironment();

    downloadManager = std::make_shared<DownloadManager>();
    folderShortcuts = std::make_shared<FolderShortcuts>();
    pluginManager = std::make_shared<PluginManager>(downloadManager);

    downloadManager->pauseDownloads();  // Do not download anything until we are fully set

    thumbnailCreator = std::make_shared<ThumbnailCreator>();

    downloadManager->resumeDownloads();
    thumbnailCreator->start();

    mainWindow = std::make_shared<MainWindow>(downloadManager, thumbnailCreator, pluginManager, folderShortcuts, updaterFileName);

    mainWindow->show();
    mainWindow->restoreTabs();

    connect(this, SIGNAL(aboutToQuit()), mainWindow.get(), SLOT(aboutToQuit()));
  }

  Application::~Application() = default;

  Application& Application::instance() {
    auto app = qobject_cast<Application*>(QApplication::instance());
    assert(app != nullptr);
    return *app;
  }

  QString Application::checkEnvironment()
  {
    QDir dir;
    QDir updaterDir;
    QDir pluginDir;
    QStringList neededFiles;
    QStringList qt4Files;
    QFile f;
    QSettings settings("settings.ini", QSettings::IniFormat);

    dir.setPath(QApplication::applicationDirPath());
    updaterDir.setPath(dir.path() + "/updater");
    pluginDir.setPath(dir.path() + "/plugins");

#ifdef Q_OS_WIN
#if (QT_VERSION >= 0x050000)
    neededFiles << "Qt5Core.dll" << "Qt5Network.dll" << "libgcc_s_sjlj-1.dll" << "libstdc++-6.dll" << "upd4t3r.exe" << "libwinpthread-1.dll";
    qt4Files << "QtCore4.dll" << "QtGui4.dll" << "libgcc_s_dw2-1.dll" << "QtNetwork4.dll" << "QtXml4.dll" << "imageformats/qgif4.dll";
    qt4Files << "imageformats/qico4.dll" << "imageformats/qjpeg4.dll" << "imageformats/qmng4.dll" << "imageformats/qsvg4.dll";
    qt4Files << "imageformats/qtiff4.dll" << "updater/libgcc_s_dw2-1.dll" << "updater/mingwm10.dll" << "updater/QtCore4.dll" << "updater/QtNetwork4.dll";

    // Clean up qt4 files, but only after the updater is at least version 1.2
    if (checkIfNewerVersion(settings.value("updater/version", "0.0").toString(), "1.1")) {
      foreach(QString filename, qt4Files) {
        if (QFile::exists(QString("%1/%2").arg(dir.absolutePath()).arg(filename))) {
          if (f.remove(QString("%1/%2").arg(dir.absolutePath()).arg(filename))) {
            QLOG_INFO() << "APP :: Deleting Qt4 file " << filename;
          }
        }
      }
    }
#else
    neededFiles << "QtCore4.dll" << "QtNetwork4.dll" << "mingwm10.dll" << "libgcc_s_dw2-1.dll" << "upd4t3r.exe";
#endif
#endif

    // Check for updater folders
    if (!updaterDir.exists()) {
      QLOG_INFO() << "APP :: updater directory does not exists. Creating " << QString("%1/updater").arg(dir.absolutePath());
      dir.mkdir("updater");
    }

    // Check if all files are present
    foreach(QString filename, neededFiles) {
      if (!QFile::exists(QString("%1/%2").arg(updaterDir.absolutePath()).arg(filename))) {
        QLOG_INFO() << "APP :: " << QString("%1/%2").arg(updaterDir.absolutePath()).arg(filename) << "does not exists - copying from application dir";
        if (!f.copy(QString("%1/%2").arg(dir.absolutePath()).arg(filename), QString("%1/%2").arg(updaterDir.absolutePath()).arg(filename))) {
          QLOG_WARN() << "APP :: Copying " << QString("%1/%2").arg(dir.absolutePath()).arg(filename) << ">" << QString("%1/%2").arg(updaterDir.absolutePath()).arg(filename) << "failed";
        }
      }
    }

    QString updaterFileName = "";
    if (QFile::exists("updater/upd4t3r.exe")) {
      // The new updater is already present
      // Use this for updating the components
      // If old updater still exists, delete it because it's not needed anymore
      updaterFileName = "updater/upd4t3r.exe";
      QFile::remove("au.exe");
      QFile::remove("updater/au.exe");
    }
    else if (QFile::exists("updater/au.exe")) {
      updaterFileName = "updater/au.exe";
    }
    else {
      QLOG_WARN() << "APP :: No updater found!";
    }

    // Check for plugin folders
    if (!pluginDir.exists()) {
      QLOG_INFO() << "APP :: Plugin directory does not exists. Creating " << QString("%1/plugins").arg(dir.absolutePath());
      dir.mkdir("plugins");
    }

    return updaterFileName;
  }
}