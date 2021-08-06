
#include <optional>

#include "uiconfig.h"
#include "ui_uiconfig.h"

namespace {
  const auto divMin = 60;
  const auto divHrs = 3600;
  const auto divDays = 86400;
  const QString resultFmt = "every %1 %2";

  bool isRounded(int value, int divider) {
    return (value % divider) == 0;
  }

  QString formatTimeout(QString text, int& value) {
    bool ok;
    value = text.toInt(&ok);
    if (!ok)
      return QString();

    if (value > divDays && isRounded(value, divDays))
      return resultFmt.arg(value / divDays).arg("days");

    if (value > divHrs && isRounded(value, divHrs))
      return resultFmt.arg(value / divHrs).arg("hours");

    if (value > divMin && isRounded(value, divMin))
      return resultFmt.arg(value / divMin).arg("minutes");

    return resultFmt.arg(value).arg("seconds");
  }
}

UIConfig::UIConfig(std::shared_ptr<FolderShortcuts> folderShortcuts_, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UIConfig),
    folderShortcuts(folderShortcuts_)
{
    ui->setupUi(this);

    _removing_thumbnails = false;

    timeoutValueEditor = new UIListEditor(this);
    timeoutValueEditor->setModal(true);

    dialogFolderShortcut = new DialogFolderShortcut(folderShortcuts, this);
    dialogFolderShortcut->setModal(true);

    userAgentStrings.insert("Wget", "Wget/1.12");
    userAgentStrings.insert("Opera", "Opera/9.80 (Windows NT 6.0) Presto/2.12.388 Version/12.14");
    userAgentStrings.insert("Firefox", "Mozilla/5.0 (Windows NT 6.1; Win64; x64; rv:25.0) Gecko/20100101 Firefox/25.0");
    userAgentStrings.insert("Chrome", "Mozilla/5.0 (Windows NT 6.2; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/32.0.1667.0 Safari/537.36");

    loadSettings();
    loadShortcuts();

    connect(timeoutValueEditor, SIGNAL(valuesChanged()), this, SLOT(loadSettings()));
    connect(ui->cbUseProxy, SIGNAL(toggled(bool)), this, SLOT(toggleProxy(bool)));
    connect(ui->listWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(editShortcutItem(QListWidgetItem*)));
    connect(folderShortcuts.get(), SIGNAL(shortcutsChanged()), this, SLOT(loadShortcuts()));
    connect(dialogFolderShortcut, SIGNAL(shortcutChanged(QString,QString,QString)), folderShortcuts.get(), SLOT(updateShortcut(QString,QString,QString)));
    connect(dialogFolderShortcut, SIGNAL(editCanceled()), this, SLOT(loadShortcuts()));
    connect(ui->btnDeleteAllThumbnails, SIGNAL(clicked()), this, SIGNAL(deleteAllThumbnails()));
    connect(ui->btnDeleteAllThumbnails, SIGNAL(clicked()), this, SLOT(thumbnailDeletionStarted()));
}

UIConfig::~UIConfig()
{
    delete ui;
}

void UIConfig::loadSettings(void) {
    ui->leDefaultSavepath->setText(settings.getDefaultDirectory());
    ui->cmbTabPosition->setCurrentIndex(settings.getTabPosition());

    ui->cbAutoClose->setChecked(settings.getAutoClose());
    ui->cbReopenTabs->setChecked(settings.getResumeSession());
    ui->cbEnlargeThumbnails->setChecked(settings.getEnlargeThumbnails());
    ui->cbHQThumbnail->setChecked(settings.getHQThumbnails());
    ui->cbDefaultOriginalFilename->setChecked(settings.getDefaultOriginalFilename());
    ui->cbRememberDirectory->setChecked(settings.getRememberDirectory());

    ui->cbCloseOverviewThreads->setChecked(settings.getCloseOverviewThreads());
    ui->cbUseInternalViewer->setChecked(settings.getUseInternalViewer());

    ui->sbConcurrentDownloads->setValue(settings.getConcurrentDownloads());
    ui->sbRescheduleInterval->setValue(settings.getResheduleInterval());

    ui->sbThumbnailHeight->setValue(settings.getThumbnailSize().width());
    ui->sbThumbnailWidth->setValue(settings.getThumbnailSize().height());

    ui->leThumbnailCacheFolder->setText(settings.getThumbnailCacheFolder());
    ui->sbThumbnailTTL->setValue(settings.getThumbnailTTL());

    {
      auto list = settings.getTimeoutValues();
      ui->cbRescanInterval->clear();
      ui->cbRescanInterval->addItem("Never", 0);
      for (auto& item : list) {
        int value = 0;
        auto text = formatTimeout(item, value);
        if (text.isEmpty())
          continue;

        ui->cbRescanInterval->addItem(text, value);
      }
      auto index = ui->cbRescanInterval->findData(settings.getDefaultTimeout());
      ui->cbRescanInterval->setCurrentIndex(index != -1 ? index : 0);
    }

    ui->cbCloseToTray->setChecked(settings.getCloseToTray());
    ui->cbLoggingLevel->setCurrentIndex(settings.getLogLevel());

    ui->cbUseBlackList->setChecked(settings.getUseBlackList());
    ui->sbBlackListCheckInterval->setValue(settings.getBlackListCheckInverval());

    ui->cbUseProxy->setChecked(settings.getNetworkUseProxy());
    ui->cbProxyAuth->setChecked(settings.getNetworkProxyAuth());
    toggleProxy(settings.getNetworkUseProxy());

    ui->leProxyAddress->setText(settings.getNetworkProxyHostName());
    ui->sbProxyPort->setValue(settings.getNetworkProxyPort());
    ui->leProxyUser->setText(settings.getNetworkProxyUser());
    ui->leProxyPassword->setText(settings.getNetworkProxyPass());

    ui->cbProxyType->setCurrentIndex(qBound(0, 2, settings.getNetworkProxyType()));

    ui->sbConcurrentDownloads->setValue(settings.getManagerConcurrentDownloads());
    ui->sbDownloadTimeoutInitial->setValue(settings.getManagerInitialTimeout());
    ui->sbDownloadTimeoutInbetween->setValue(settings.getRunningTimeout());
    ui->cbKeepLocalHTMLCopy->setChecked(settings.getUseThreadCache());
    ui->leThreadCachePath->setText(settings.getThreadCachePath());
    ui->cbCompressCacheFile->setChecked(settings.getCompressCacheFile());
    ui->leUserAgent->setText(settings.getUserAgent());
    ui->cmbUserAgent->clear();
    ui->cmbUserAgent->addItems(userAgentStrings.keys());

    ui->sbUpdaterPort->setValue(settings.getUpdaterPort());
    ui->sbApplicationPort->setValue(settings.getUpdaterAppPort());

    timeoutValueEditor->loadSettings();
}

void UIConfig::accept(void) {

  settings.setDefaultDirectory(ui->leDefaultSavepath->text());
  settings.setTabPosition(ui->cmbTabPosition->currentIndex());
  settings.setAutoClose(ui->cbAutoClose->isChecked());
  settings.setResumeSession(ui->cbReopenTabs->isChecked());
  settings.setEnlargeThumbnails(ui->cbEnlargeThumbnails->isChecked());
  settings.setHQThumbnails(ui->cbHQThumbnail->isChecked());
  settings.setDefaultOriginalFilename(ui->cbDefaultOriginalFilename->isChecked());
  settings.setRememberDirectory(ui->cbRememberDirectory->isChecked());
  settings.setConcurrentDownloads(ui->sbConcurrentDownloads->value());
  settings.setResheduleInterval(ui->sbRescheduleInterval->value());
  settings.setThumbnailSize(QSize(ui->sbThumbnailWidth->value(), ui->sbThumbnailHeight->value()));
  settings.setDefaultTimeout(ui->cbRescanInterval->currentData().toInt());
  settings.setThumbnailCacheFolder(ui->leThumbnailCacheFolder->text());
  settings.setThumbnailTTL(ui->sbThumbnailTTL->value());
  settings.setCloseOverviewThreads(ui->cbCloseOverviewThreads->isChecked());
  settings.setUseInternalViewer(ui->cbUseInternalViewer->isChecked());
  settings.setCloseToTray(ui->cbCloseToTray->isChecked());
  settings.setLogLevel(ui->cbLoggingLevel->currentIndex());

  settings.setUseBlackList(ui->cbUseBlackList->isChecked());
  settings.setBlackListCheckInverval(ui->sbBlackListCheckInterval->value());

  settings.setNetworkUseProxy(ui->cbUseProxy->isChecked());
  settings.setNetworkProxyAuth(ui->cbProxyAuth->isChecked());
  settings.setNetworkProxyHostName(ui->leProxyAddress->text());
  settings.setNetworkProxyPort(ui->sbProxyPort->value());
  settings.setNetworkProxyUser(ui->leProxyUser->text());
  settings.setNetworkProxyPass(ui->leProxyPassword->text());
  settings.setNetworkProxyType(qBound(0, 2, ui->cbProxyType->currentIndex()));

  settings.setManagerConcurrentDownloads(ui->sbConcurrentDownloads->value());
  settings.setManagerInitialTimeout(ui->sbDownloadTimeoutInitial->value());
  settings.setRunningTimeout(ui->sbDownloadTimeoutInbetween->value());
  settings.setUseThreadCache(ui->cbKeepLocalHTMLCopy->isChecked());
  settings.setThreadCachePath(ui->leThreadCachePath->text());
  settings.setUserAgent(ui->leUserAgent->text());
  settings.setCompressCacheFile(ui->cbCompressCacheFile->isChecked());

  settings.setUpdaterPort(ui->sbUpdaterPort->value());
  settings.setUpdaterAppPort(ui->sbApplicationPort->value());

  settings.sync();

  emit configurationChanged();
  hide();
}

void UIConfig::reject(void) {
    hide();
}

void UIConfig::chooseLocation(void) {
    QString loc;

    loc = QFileDialog::getExistingDirectory(this, "Choose storage directory", ui->leDefaultSavepath->text());
    if (!loc.isEmpty())
        ui->leDefaultSavepath->setText(loc);
}

void UIConfig::chooseThumbnailCacheLocation(void) {
    QString loc;

    loc = QFileDialog::getExistingDirectory(this, "Choose thumbnail cache directory", ui->leThumbnailCacheFolder->text());
    if (!loc.isEmpty())
        ui->leThumbnailCacheFolder->setText(loc);
}

void UIConfig::chooseThreadCacheLocation(void) {
    QString loc;

    loc = QFileDialog::getExistingDirectory(this, "Choose thread cache directory", ui->leThreadCachePath->text());
    if (!loc.isEmpty())
        ui->leThreadCachePath->setText(loc);
}

void UIConfig::editTimeoutValues(void) {
    timeoutValueEditor->show();
}

void UIConfig::toggleProxy(bool b) {
    ui->leProxyAddress->setEnabled(b);
    ui->sbProxyPort->setEnabled(b);
    ui->cbProxyType->setEnabled(b);
    ui->cbProxyAuth->setEnabled(b);

    if (b) {
        if (ui->cbProxyAuth->isChecked()) {
            ui->leProxyUser->setEnabled(b);
            ui->leProxyPassword->setEnabled(b);
        }
    }
    else {
        ui->leProxyUser->setEnabled(b);
        ui->leProxyPassword->setEnabled(b);
    }
}

void UIConfig::addShortcut() {
//    ui->listWidget->addItem("???");
//    editShortcutItem(ui->listWidget->item(ui->listWidget->count()-1));
    editShortcut("");
}

void UIConfig::editShortcut(QString name) {
    dialogFolderShortcut->clear();
    dialogFolderShortcut->edit(name);

    dialogFolderShortcut->show();
}

void UIConfig::editShortcutItem(QListWidgetItem* item) {
    editShortcut(item->text());
}

void UIConfig::loadShortcuts() {
    ui->listWidget->clear();

    ui->listWidget->addItems(folderShortcuts->shortcuts());
}

void UIConfig::deleteShortcut() {
    QString name;

    name = ui->listWidget->currentItem()->text();

    if (name != "???")
        folderShortcuts->deleteShortcut(name);
    else
        ui->listWidget->removeItemWidget(ui->listWidget->currentItem());
}

void UIConfig::toggleLogLevelWarning(QString s) {
    if (s == "Trace") {
        ui->lLogLevelWarning->show();
    }
    else {
        ui->lLogLevelWarning->hide();
    }
}

void UIConfig::thumbnailDeletionFinished() {
    if (_removing_thumbnails) {
        ui->lDeleteThumbnailsStatus->setText("Deleting finished");
        _removing_thumbnails = false;
    }
}

void UIConfig::thumbnailDeletionStarted() {
    ui->lDeleteThumbnailsStatus->setText("Deleting");
    _removing_thumbnails = true;
}

void UIConfig::setUserAgentString() {
    if (ui->cmbUserAgent->currentText() != "---") {
        ui->leUserAgent->setText(userAgentStrings.value(ui->cmbUserAgent->currentText()));
    }
}
