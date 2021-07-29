#ifndef UIINFO_H
#define UIINFO_H

#include <QDialog>
#include <QUrl>
#include <QtNetwork>
#include <QFile>
#include "downloadmanager.h"
#include "pluginmanager.h"
#include "uipendingrequests.h"

#ifndef PROGRAM_VERSION
#define PROGRAM_VERSION "Cracked"
#endif

#ifndef BUILDDATE
#define BUILDDATE "19.99.9999"
#endif

namespace Ui {
    class UIInfo;
}

class UIInfo : public QDialog
{
    Q_OBJECT

public:
    explicit UIInfo(
      std::shared_ptr<DownloadManager> downloadManager_, 
      std::shared_ptr<PluginManager> pluginManager_,
      QWidget *parent = nullptr);
    ~UIInfo();
    void setCurrentVersion(QString);
    void loadComponentInfo(QMap<QString, component_information> components);

private:
    Ui::UIInfo *ui;
    QTimer* timer;
    QFile* logFile;
    UIPendingRequests* uiPendingRequests;
    std::shared_ptr<DownloadManager> downloadManager;
    std::shared_ptr<PluginManager> pluginManager;

private slots:
    void updateStatistics();
    void updateDebugInformation();
    void updateLogFile();
    void showRequests();
    void reloadRequests();

signals:
};

#endif // UIINFO_H
