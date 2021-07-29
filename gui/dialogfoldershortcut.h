#ifndef DIALOGFOLDERSHORTCUT_H
#define DIALOGFOLDERSHORTCUT_H

#include <QDialog>
#include <QSettings>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QFileDialog>
#include "foldershortcuts.h"

namespace Ui {
    class DialogFolderShortcut;
}

class DialogFolderShortcut : public QDialog
{
    Q_OBJECT

public:
    explicit DialogFolderShortcut(std::shared_ptr<FolderShortcuts> shortcuts, QWidget *parent = nullptr);
    ~DialogFolderShortcut();
    void edit(QString);

private:
    Ui::DialogFolderShortcut *ui;
    QString _originalShortcutName;
    std::shared_ptr<FolderShortcuts> folderShortcuts;

public slots:
    void clear();

private slots:
    void selectShortcutIndex(int);
    void selectShortcut(QString);
    void choosePath();
    void checkValues();
    void accept(void);
    void reject(void);
    void fillShortcutComboBox();

signals:
    void shortcutChanged(QString, QString, QString);
    void editCanceled();
};

#endif // DIALOGFOLDERSHORTCUT_H
