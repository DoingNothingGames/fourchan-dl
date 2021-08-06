#pragma once

#include <QSettings>
#include <QSize>

namespace chandl {
  static const QString APP_SETTINGS_DEFAULT_FILENAME = "settings.ini";

  class AppSettings {
  public:
    AppSettings(const QString& filename = APP_SETTINGS_DEFAULT_FILENAME);

    void sync();
    QSettings::Status getStatus() const;

    QString getDefaultDirectory(QString def = QString()) const;
    void setDefaultDirectory(QString value);

    int getTabPosition(int def = 3) const;
    void setTabPosition(int value);

    bool getAutoClose(bool def = false) const;
    void setAutoClose(bool value);

    QSize getThumbnailSize(QSize def = QSize(200, 200)) const;
    void setThumbnailSize(QSize value);

    int getMaxDownloads(int def = 1) const;
    void setMaxDownloads(int value);

    bool getEnlargeThumbnails(bool def = false) const;
    void setEnlargeThumbnails(bool value = true);

    bool getHQThumbnails(bool def = false) const;
    void setHQThumbnails(bool value);

    QString getThumbnailCacheFolder(QString def = QString()) const;
    void setThumbnailCacheFolder(QString value);

    int getThumbnailTTL(int def = 60) const;
    void setThumbnailTTL(int value);

    bool getDefaultOriginalFilename(bool def = false) const;
    void setDefaultOriginalFilename(bool value);

    bool getCloseOverviewThreads(bool def = true) const;
    void setCloseOverviewThreads(bool value);

    bool getUseInternalViewer(bool def = true) const;
    void setUseInternalViewer(bool value);

    int getConcurrentDownloads(int def = 1) const;
    void setConcurrentDownloads(int value);

    int getResheduleInterval(int def = 60) const;
    void setResheduleInterval(int value);

    QStringList getTimeoutValues(QStringList def = QStringList()) const;
    void setTimeoutValues(QStringList values);

    int getDefaultTimeout(int def = 0) const;
    void setDefaultTimeout(int value);

    bool getNetworkUseProxy(bool def = false) const;
    void setNetworkUseProxy(bool value);

    int getNetworkProxyType(int def = 3) const;
    void setNetworkProxyType(int value);

    QString getNetworkProxyHostName(QString def = QString()) const;
    void setNetworkProxyHostName(QString value);

    quint16 getNetworkProxyPort(quint16 def = 0) const;
    void setNetworkProxyPort(quint16 value);

    bool getNetworkProxyAuth(bool def = false) const;
    void setNetworkProxyAuth(bool value);

    QString getNetworkProxyUser(QString def = QString()) const;
    void setNetworkProxyUser(QString value);

    QString getNetworkProxyPass(QString def = QString()) const;
    void setNetworkProxyPass(QString value);

    QString getConsoleVersion(QString def = "0.1.0") const;
    void setConsoleVersion(QString value);

    int getLogLevel(int def = 3) const;
    void setLogLevel(int value);

    int getUpdaterPort(int def = 60000) const;
    void setUpdaterPort(int value);

    int getUpdaterAppPort(int def = 60001) const;
    void setUpdaterAppPort(int value);

    QString getUpdaterVersion(QString def = "unknown") const;
    void setUpdaterVersion(QString value);

    bool getUseBlackList(bool def = true) const;
    void setUseBlackList(bool value);

    int getBlackListCheckInverval(int def = 600) const;
    void setBlackListCheckInverval(int value);

    int getManagerConcurrentDownloads(int def = 20) const;
    void setManagerConcurrentDownloads(int value);

    int getManagerInitialTimeout(int def = 30) const;
    void setManagerInitialTimeout(int value);

    int getRunningTimeout(int def = 2) const;
    void setRunningTimeout(int value);

    bool getUseThreadCache(bool def = false) const;
    void setUseThreadCache(bool value);

    QString getThreadCachePath(QString def = QString()) const;
    void setThreadCachePath(QString value);

    int getDownloadedFilesStatistic(int def = 0) const;
    void setDownloadedFilesStatistic(int value);

    float getDownloadedKBytesStatistic(float def = 0.0) const;
    void setDownloadedKBytesStatistic(float value);

    bool getCompressCacheFile(bool def = true) const;
    void setCompressCacheFile(bool value);

    QString getUserAgent(QString def = QString()) const;
    void setUserAgent(QString value);

    QStringList getShortcuts(QStringList def = QStringList()) const;
    void setShortcuts(QStringList value);

    bool getRememberDirectory(bool def = false) const;
    void setRememberDirectory(bool value);

    QPoint getWindowPosition(QPoint def = QPoint(0, 0)) const;
    void setWindowPosition(QPoint value);

    int getWindowState(int def = 0) const;
    void setWindowState(int value);

    QSize getWindowSize(QSize def = QSize(0, 0)) const;
    void setWindowSize(QSize value);

    QByteArray getWindowWidgetState(QByteArray def = QByteArray()) const;
    void setWindowWidgetState(QByteArray value);

    QPoint getViewerPosition(QPoint def = QPoint()) const;
    void setViewerPosition(QPoint value);

    int getViewerState(int def = 0) const;
    void setViewerState(int value);

    QSize getViewerSize(QSize def = QSize(0, 0)) const;
    void setViewerSize(QSize value);

    bool getViewerFitImage(bool def = false) const;
    void setViewerFitImage(bool value);

    int getViewerSlideshowPause(int def = 3) const;
    void setViewerSlideshowPause(int value);

    bool getThreadOverviewVisible(bool def = true) const;
    void setThreadOverviewVisible(bool value);

    int getThreadOverviewUriWidth(int def = 170);
    void setThreadOverviewUriWidth(int value);

    int getThreadOverviewNameWidth(int def = 190);
    void setThreadOverviewNameWidth(int value);

    int getThreadOverviewImagesWidth(int def = 60);
    void setThreadOverviewImagesWidth(int value);

    int getThreadOverviewStatusWidth(int def = 70);
    void setThreadOverviewStatusWidth(int value);

    int getTabsCount(int def = 0) const;
    QString getTabValues(int tabIndex, QString def = QString()) const;

    void clearTabs();

    void setTabsCount(int value);
    void setTabValues(int tabIndex, QString values);

    bool getResumeSession(bool def = false) const;
    void setResumeSession(bool value);

    bool getCloseToTray(bool def = false) const;
    void setCloseToTray(bool value);

  private:
    QSettings settings;
  };
}