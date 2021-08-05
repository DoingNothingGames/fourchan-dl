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

    int getLogLevel(int def = -1) const;
    int getUpdaterPort(int def = 60000) const;
    int getUpdaterAppPort(int def = 60001) const;

    QString getUpdaterVersion(QString def = "unknown") const;
    void setUpdaterVersion(QString value);

    bool getUseBlackList(bool def = true) const;
    int getBlackListCheckInverval(int def = 600) const;

    int getConcurrentDownloads(int def = 20) const;
    int getInitialTimeout(int def = 30) const;
    int getRunningTimeout(int def = 20) const;

    bool getUseThreadCache(bool def = false) const;
    QString getThreadCachePath(QString def = QString()) const;

    int getDownloadedFilesStatistic(int def = 0) const;
    void setDownloadedFilesStatistic(int value);

    float getDownloadedKBytesStatistic(float def = 0.0) const;
    void setDownloadedKBytesStatistic(float value);

    bool getCompressCacheFile(bool def = true) const;

    QString getUserAgent(QString def = QString()) const;

    QStringList getShortcuts(QStringList def = QStringList()) const;
    void setShortcuts(QStringList value);

    bool getRememberDirectory(bool def = false) const;

    QPoint getWindowPosition(QPoint def = QPoint(0, 0)) const;
    void setWindowPosition(QPoint value);

    int getWindowState(int def = 0) const;
    void setWindowState(int value);

    QSize getWindowSize(QSize def = QSize(0, 0)) const;
    void setWindowSize(QSize value);

    QByteArray getWindowWidgetState(QByteArray def = QByteArray()) const;
    void setWindowWidgetState(QByteArray value);

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

    bool getCloseToTray(bool def = false) const;
    void setCloseToTray(bool value);

  private:
    QSettings settings;
  };
}