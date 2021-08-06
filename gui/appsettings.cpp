
#include <QCoreApplication>

#include "appsettings.h"

namespace chandl {
  AppSettings::AppSettings(const QString& filename) :
    settings(filename, QSettings::IniFormat)
  {

  }

  void AppSettings::sync()
  {
    settings.sync();
  }

  QSettings::Status AppSettings::getStatus() const
  {
    return settings.status();
  }

  QString AppSettings::getDefaultDirectory(QString def) const
  {
    return settings.value("options/default_directory", def).toString();
  }

  void AppSettings::setDefaultDirectory(QString value)
  {
    settings.setValue("options/default_directory", value);
  }

  int AppSettings::getTabPosition(int def) const
  {
    return settings.value("options/tab_position", def).toInt();
  }

  void AppSettings::setTabPosition(int value)
  {
    return settings.setValue("options/tab_position", value);
  }

  bool AppSettings::getAutoClose(bool def) const
  {
    return settings.value("options/automatic_close", def).toBool();
  }

  void AppSettings::setAutoClose(bool value)
  {
    settings.setValue("options/automatic_close", value);
  }

  QSize AppSettings::getThumbnailSize(QSize def) const
  {
    if (settings.contains("options/thumbnail_size"))
      return settings.value("options/thumbnail_size").toSize();

    return QSize(
      settings.value("options/thumbnail_width", def.width()).toInt(),
      settings.value("options/thumbnail_height", def.height()).toInt()
    );
  }

  void AppSettings::setThumbnailSize(QSize value)
  {
    settings.remove("options/thumbnail_width");
    settings.remove("options/thumbnail_height");
    settings.setValue("options/thumbnail_size", value);
  }

  int AppSettings::getMaxDownloads(int def) const
  {
    return settings.value("options/concurrent_downloads", def).toInt();
  }

  void AppSettings::setMaxDownloads(int value)
  {
    settings.setValue("options/concurrent_downloads", value);
  }

  bool AppSettings::getEnlargeThumbnails(bool def) const
  {
    return settings.value("options/enlarge_thumbnails", def).toBool();
  }

  void AppSettings::setEnlargeThumbnails(bool value)
  {
    settings.setValue("options/enlarge_thumbnails", value);
  }

  bool AppSettings::getHQThumbnails(bool def) const
  {
    return settings.value("options/hq_thumbnails", def).toBool();
  }

  void AppSettings::setHQThumbnails(bool value)
  {
    settings.setValue("options/hq_thumbnails", value);
  }

  QString AppSettings::getThumbnailCacheFolder(QString def) const
  {
    if (def.isEmpty())
      def = QString("%1/%2").arg(QCoreApplication::applicationDirPath()).arg("tncache");
    return settings.value("options/thumbnail_cache_folder", def).toString();
  }

  void AppSettings::setThumbnailCacheFolder(QString value)
  {
    settings.setValue("options/thumbnail_cache_folder", value);
  }

  int AppSettings::getThumbnailTTL(int def) const
  {
    return settings.value("options/thumbnail_TTL", def).toInt();
  }

  void AppSettings::setThumbnailTTL(int value)
  {
    settings.value("options/thumbnail_TTL", value);
  }

  bool AppSettings::getDefaultOriginalFilename(bool def) const
  {
    return settings.value("options/default_original_filename", def).toBool();
  }

  void AppSettings::setDefaultOriginalFilename(bool value)
  {
    settings.setValue("options/default_original_filename", value);
  }

  bool AppSettings::getCloseOverviewThreads(bool def) const
  {
    return settings.value("options/close_overview_threads", def).toBool();
  }

  void AppSettings::setCloseOverviewThreads(bool value)
  {
    settings.setValue("options/close_overview_threads", value);
  }

  bool AppSettings::getUseInternalViewer(bool def) const
  {
    return settings.value("options/use_internal_viewer", def).toBool();
  }

  void AppSettings::setUseInternalViewer(bool value)
  {
    settings.setValue("options/use_internal_viewer", value);
  }

  int AppSettings::getConcurrentDownloads(int def) const
  {
    return settings.value("options/concurrent_downloads", def).toInt();
  }

  void AppSettings::setConcurrentDownloads(int value)
  {
    settings.setValue("options/concurrent_downloads", value);
  }

  int AppSettings::getResheduleInterval(int def) const
  {
    return settings.value("options/reschedule_interval", def).toInt();
  }

  void AppSettings::setResheduleInterval(int value)
  {
    settings.setValue("options/reschedule_interval", value);
  }

  QStringList AppSettings::getTimeoutValues(QStringList def) const
  {
    if (def.empty())
      def = QStringList() << "300" << "600";
    return settings.value("options/timeout_values", def).toStringList();
  }

  void AppSettings::setTimeoutValues(QStringList values)
  {
    settings.setValue("options/timeout_values", values);
  }

  int AppSettings::getDefaultTimeout(int def) const
  {
    return settings.value("options/default_timeout", def).toInt();
  }

  void AppSettings::setDefaultTimeout(int value)
  {
    settings.value("options/default_timeout", value);
  }

  bool AppSettings::getNetworkUseProxy(bool def) const
  {
    return settings.value("network/use_proxy", def).toBool();
  }

  void AppSettings::setNetworkUseProxy(bool value)
  {
    settings.setValue("network/use_proxy", value);
  }

  int AppSettings::getNetworkProxyType(int def) const
  {
    return settings.value("network/proxy_type", def).toInt();
  }

  void AppSettings::setNetworkProxyType(int value)
  {
    settings.setValue("network/proxy_type", value);
  }

  QString AppSettings::getNetworkProxyHostName(QString def) const
  {
    return settings.value("network/proxy_hostname", def).toString();
  }

  void AppSettings::setNetworkProxyHostName(QString value)
  {
    settings.setValue("network/proxy_hostname", value);
  }

  quint16 AppSettings::getNetworkProxyPort(quint16 def) const
  {
    return settings.value("network/proxy_port", def).toUInt();
  }

  void AppSettings::setNetworkProxyPort(quint16 value)
  {
    settings.setValue("network/proxy_port", value);
  }

  bool AppSettings::getNetworkProxyAuth(bool def) const
  {
    return settings.value("network/proxy_auth", def).toBool();
  }

  void AppSettings::setNetworkProxyAuth(bool value)
  {
    settings.setValue("network/proxy_auth", value);
  }

  QString AppSettings::getNetworkProxyUser(QString def) const
  {
    return settings.value("network/proxy_user", def).toString();
  }

  void AppSettings::setNetworkProxyUser(QString value)
  {
    settings.setValue("network/proxy_user", value);
  }

  QString AppSettings::getNetworkProxyPass(QString def) const
  {
    return settings.value("network/proxy_pass", def).toString();
  }

  void AppSettings::setNetworkProxyPass(QString value)
  {
    settings.setValue("network/proxy_pass", value);
  }

  QString AppSettings::getConsoleVersion(QString def) const
  {
    return settings.value("console/version", def).toString();
  }

  void AppSettings::setConsoleVersion(QString value)
  {
    settings.setValue("console/version", value);
  }

  int AppSettings::getLogLevel(int def) const
  {
    return settings.value("options/log_level", def).toInt();
  }

  void AppSettings::setLogLevel(int value)
  {
    settings.setValue("options/log_level", value);
  }

  int AppSettings::getUpdaterPort(int def) const
  {
    return settings.value("updater/updater_port", def).toInt();
  }

  void AppSettings::setUpdaterPort(int value)
  {
    settings.setValue("updater/updater_port", value);
  }

  int AppSettings::getUpdaterAppPort(int def) const
  {
    return settings.value("updater/application_port", def).toInt();
  }

  void AppSettings::setUpdaterAppPort(int value)
  {
    settings.setValue("updater/application_port", value);
  }

  QString AppSettings::getUpdaterVersion(QString def) const
  {
    return settings.value("updater/version", def).toString();
  }

  void AppSettings::setUpdaterVersion(QString value)
  {
    settings.setValue("updater/version", value);
  }

  bool AppSettings::getUseBlackList(bool def) const
  {
    return settings.value("blacklist/use_blacklist", def).toBool();
  }

  void AppSettings::setUseBlackList(bool value)
  {
    settings.setValue("blacklist/use_blacklist", value);
  }

  int AppSettings::getBlackListCheckInverval(int def) const
  {
    return settings.value("blacklist/blacklist_check_interval", def).toInt();
  }

  void AppSettings::setBlackListCheckInverval(int value)
  {
    settings.setValue("blacklist/blacklist_check_interval", value);
  }

  int AppSettings::getManagerConcurrentDownloads(int def) const
  {
    return settings.value("download_manager/concurrent_downloads", def).toInt();
  }

  void AppSettings::setManagerConcurrentDownloads(int value)
  {
    settings.setValue("download_manager/concurrent_downloads", value);
  }

  int AppSettings::getManagerInitialTimeout(int def) const
  {
    return settings.value("download_manager/initial_timeout", def).toInt();
  }

  void AppSettings::setManagerInitialTimeout(int value)
  {
    settings.setValue("download_manager/initial_timeout", value);
  }

  int AppSettings::getRunningTimeout(int def) const
  {
    return settings.value("download_manager/running_timeout", def).toInt();
  }

  void AppSettings::setRunningTimeout(int value)
  {
    settings.setValue("download_manager/running_timeout", value);
  }

  bool AppSettings::getUseThreadCache(bool def) const
  {
    return settings.value("download_manager/use_thread_cache", def).toBool();
  }

  void AppSettings::setUseThreadCache(bool value)
  {
    settings.setValue("download_manager/use_thread_cache", value);
  }

  QString AppSettings::getThreadCachePath(QString def) const
  {
    if (def.isEmpty())
      def = QString("%1/%2").arg(QCoreApplication::applicationDirPath()).arg("thread-cache");
    return settings.value("download_manager/thread_cache_path", def).toString();
  }

  void AppSettings::setThreadCachePath(QString value)
  {
    settings.setValue("download_manager/thread_cache_path", value);
  }

  int AppSettings::getDownloadedFilesStatistic(int def) const
  {
    return settings.value("statistics/downloaded_files", def).toInt();
  }

  void AppSettings::setDownloadedFilesStatistic(int value)
  {
    settings.setValue("statistics/downloaded_files", value);
  }

  float AppSettings::getDownloadedKBytesStatistic(float def) const
  {
    return settings.value("statistics/downloaded_kbytes", def).toFloat();
  }

  void AppSettings::setDownloadedKBytesStatistic(float value)
  {
    settings.setValue("statistics/downloaded_kbytes", value);
  }

  bool AppSettings::getCompressCacheFile(bool def) const
  {
    return settings.value("download_manager/compress_cache_file", def).toBool();
  }

  void AppSettings::setCompressCacheFile(bool value)
  {
    settings.setValue("download_manager/compress_cache_file", value);
  }

  QString AppSettings::getUserAgent(QString def) const
  {
    if (def.isEmpty())
      def = "Wget/1.12";
    return settings.value("options/user-agent", def).toString();
  }

  void AppSettings::setUserAgent(QString value)
  {
    settings.setValue("options/user-agent", value);
  }

  QStringList AppSettings::getShortcuts(QStringList def) const
  {
    return settings.value("shortcuts", def).toStringList();
  }

  void AppSettings::setShortcuts(QStringList value)
  {
    settings.setValue("shortcuts", value);
  }

  bool AppSettings::getRememberDirectory(bool def) const
  {
    return settings.value("options/remember_directory", def).toBool();
  }

  void AppSettings::setRememberDirectory(bool value)
  {
    settings.setValue("options/remember_directory", value);
  }

  QPoint AppSettings::getWindowPosition(QPoint def) const
  {
    return settings.value("window/position", def).toPoint();
  }

  void AppSettings::setWindowPosition(QPoint value)
  {
    settings.setValue("window/position", value);
  }

  int AppSettings::getWindowState(int def) const
  {
    return settings.value("window/state", def).toInt();
  }

  void AppSettings::setWindowState(int value)
  {
    settings.setValue("window/state", value);
  }

  QSize AppSettings::getWindowSize(QSize def) const
  {
    return settings.value("window/size", def).toSize();
  }

  void AppSettings::setWindowSize(QSize value)
  {
    settings.setValue("window/size", value);
  }

  QByteArray AppSettings::getWindowWidgetState(QByteArray def) const
  {
    return settings.value("window/widgetstate", def).toByteArray();
  }

  void AppSettings::setWindowWidgetState(QByteArray value)
  {
    settings.setValue("window/widgetstate", value);
  }

  QPoint AppSettings::getViewerPosition(QPoint def) const
  {
    return settings.value("imageviewer/position", def).toPoint();
  }

  void AppSettings::setViewerPosition(QPoint value)
  {
    settings.setValue("imageviewer/position", value);
  }

  int AppSettings::getViewerState(int def) const
  {
    return settings.value("imageviewer/state", def).toInt();
  }

  void AppSettings::setViewerState(int value)
  {
    settings.setValue("imageviewer/state", value);
  }

  QSize AppSettings::getViewerSize(QSize def) const
  {
    return settings.value("imageviewer/size", def).toSize();
  }

  void AppSettings::setViewerSize(QSize value)
  {
    settings.setValue("imageviewer/size", value);
  }

  bool AppSettings::getViewerFitImage(bool def) const
  {
    return settings.value("imageviewer/fit_image", def).toBool();
  }

  void AppSettings::setViewerFitImage(bool value)
  {
    settings.setValue("imageviewer/fit_image", value);
  }

  int AppSettings::getViewerSlideshowPause(int def) const
  {
    return settings.value("imageviewer/slideshow_pause", def).toInt();
  }

  void AppSettings::setViewerSlideshowPause(int value)
  {
    settings.setValue("imageviewer/slideshow_pause", value);
  }

  bool AppSettings::getThreadOverviewVisible(bool def) const
  {
    return settings.value("thread_overview/visible", def).toBool();
  }

  void AppSettings::setThreadOverviewVisible(bool value)
  {
    settings.setValue("thread_overview/visible", value);
  }

  int AppSettings::getThreadOverviewUriWidth(int def)
  {
    return settings.value("thread_overview/col_uri_width", def).toInt();
  }

  void AppSettings::setThreadOverviewUriWidth(int value)
  {
    settings.setValue("thread_overview/col_uri_width", value);
  }

  int AppSettings::getThreadOverviewNameWidth(int def)
  {
    return settings.value("thread_overview/col_name_width", def).toInt();
  }

  void AppSettings::setThreadOverviewNameWidth(int value)
  {
    settings.setValue("thread_overview/col_name_width", value);
  }

  int AppSettings::getThreadOverviewImagesWidth(int def)
  {
    return settings.value("thread_overview/col_images_width", def).toInt();
  }

  void AppSettings::setThreadOverviewImagesWidth(int value)
  {
    settings.setValue("thread_overview/col_images_width", value);
  }

  int AppSettings::getThreadOverviewStatusWidth(int def)
  {
    return settings.value("thread_overview/col_status_width", def).toInt();
  }

  void AppSettings::setThreadOverviewStatusWidth(int value)
  {
    settings.setValue("thread_overview/col_status_width", value);
  }

  int AppSettings::getTabsCount(int def) const
  {
    return settings.value("tabs/count", def).toInt();
  }

  QString AppSettings::getTabValues(int tabIndex, QString def) const
  {
    if (def.isEmpty()) {
      def = ";;;;0;;every 30 seconds;;0";
    }
    return settings.value(QString("tabs/tab%1").arg(tabIndex), def).toString();
  }

  void AppSettings::clearTabs()
  {
    settings.remove("tabs");
    settings.sync();
  }

  void AppSettings::setTabsCount(int value)
  {
    settings.setValue("tabs/count", value);
  }

  void AppSettings::setTabValues(int tabIndex, QString values)
  {
    settings.setValue(QString("tabs/tab%1").arg(tabIndex), values);
  }

  bool AppSettings::getResumeSession(bool def) const
  {
    return settings.value("options/resume_session", def).toBool();
  }

  void AppSettings::setResumeSession(bool value)
  {
    settings.setValue("options/resume_session", value);
  }

  bool AppSettings::getCloseToTray(bool def) const
  {
    return settings.value("options/close_to_tray", def).toBool();
  }

  void AppSettings::setCloseToTray(bool value)
  {
    settings.setValue("options/close_to_tray", value);
  }
}