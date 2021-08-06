#ifndef UICONFIG_H
#define UICONFIG_H

#include <QDialog>
#include <QSettings>
#include <QFileDialog>

#include "appsettings.h"
#include "uilisteditor.h"
#include "dialogfoldershortcut.h"
#include "foldershortcuts.h"
#include "QsLog.h"

namespace Ui {
    class UIConfig;
}

class UIConfig : public QDialog
{
    Q_OBJECT

public:
    explicit UIConfig(std::shared_ptr<FolderShortcuts> folderShortcuts_, QWidget *parent = 0);
    ~UIConfig();

private:
  chandl::AppSettings settings;
    Ui::UIConfig *ui;
    UIListEditor* timeoutValueEditor;
    DialogFolderShortcut* dialogFolderShortcut;
    bool _removing_thumbnails;
    QMap<QString,QString> userAgentStrings;
    std::shared_ptr<FolderShortcuts> folderShortcuts;

public slots:
    void thumbnailDeletionFinished();
private slots:
    void accept(void);
    void reject(void);
    void chooseLocation(void);
    void chooseThumbnailCacheLocation(void);
    void chooseThreadCacheLocation(void);
    void editTimeoutValues(void);
    void loadSettings(void);
    void toggleProxy(bool);
    void addShortcut(void);
    void editShortcut(QString);
    void deleteShortcut();
    void editShortcutItem(QListWidgetItem*);
    void loadShortcuts();
    void toggleLogLevelWarning(QString);
    void thumbnailDeletionStarted();
    void setUserAgentString();
signals:
    void configurationChanged(void);
    void deleteAllThumbnails();
};

#endif // UICONFIG_H
