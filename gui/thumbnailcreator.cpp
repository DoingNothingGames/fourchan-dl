

#include <QDebug>
#include <QFile>
#include <QCoreApplication>
#include <QDir>

#include "QsLog.h"

#include "thumbnailcreator.h"

ThumbnailCreator::ThumbnailCreator() :
  QThread(nullptr)
{
  iconSize = new QSize(100, 100);
  settings = new QSettings("settings.ini", QSettings::IniFormat);
}

void ThumbnailCreator::run() {
  QImage original, tn;
  QDir dir;

  while (!stopped) {
    QString currentFilename;

    {
      QMutexLocker lock(&mutex);
      if (list.count() > 0 && !paused) {
        currentFilename = list.front();
        list.pop_front();
      }
      else {
        condition.wait(&mutex);
        continue;
      }
    }

    if (currentFilename.isEmpty()) {
      continue;
    }

    auto cacheFile = getCacheFile(currentFilename);

    bool useCachedThumbnail = false;
    bool image_loaded = false;

    int iconWidth = iconSize->width();
    int iconHeight = iconSize->height();

    bool enlargeThumbnails = settings->value("options/enlarge_thumbnails", false).toBool();
    bool hqRendering = settings->value("options/hq_thumbnails", false).toBool();
    auto useCache = true;

    auto cacheFolder = settings->value("options/thumbnail_cache_folder", QString("%1/%2").arg(QCoreApplication::applicationDirPath())
      .arg("tncache")).toString();

    //        QLOG_ALWAYS() << "ThumbnailCreator :: Using thumbnail folder " << cacheFolder;
    if (useCache && !(dir.exists(cacheFolder))) {
      QLOG_TRACE() << "ThumbnailCreator :: Creating thumbnail cache folder " << cacheFolder;
      dir.mkpath(cacheFolder);
    }

    // Check if thumbnail exists
    if (useCache && QFile::exists(cacheFile)) {
      QLOG_TRACE() << "ThumbnailCreator :: Cached thumbnail available for " << currentFilename;
      tn.load(cacheFile);

      if (tn.width() == iconWidth || tn.height() == iconHeight) {
        useCachedThumbnail = true;
      }
    }

    if (!useCachedThumbnail) {
      QLOG_TRACE() << "ThumbnailCreator :: Creating new thumbnail for " << currentFilename;
      if (original.load(currentFilename)) {
        image_loaded = true;
      }
      else {
        if (QFile::exists(currentFilename)) {
          QLOG_DEBUG() << __func__ << ":: Image" << currentFilename << "cannot be processed. Using default thumbnail instead.";
          if (original.load(":/icons/resources/image-missing.png")) {
            image_loaded = true;

            QLOG_TRACE() << __func__ << ":: Loaded default image";
          }
        }
        else {
          QLOG_ERROR() << __func__ << ":: Image" << currentFilename << " does not exist. Thumbnail not created.";
          image_loaded = false;
        }
      }

      if (image_loaded) {
        QLOG_TRACE() << "ThumbnailCreator :: Loaded original file " << currentFilename;
        if (original.width() < iconWidth
          && original.height() < iconHeight
          && !(enlargeThumbnails)) {
          QLOG_TRACE() << "ThumbnailCreator :: Setting original as thumbnail";
          tn = original;
        }
        else {
          QLOG_TRACE() << "ThumbnailCreator :: Rendering thumbnail";

          if (hqRendering) {
            tn = original.scaled(*iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
          }
          else {
            tn = original.scaled(*iconSize, Qt::KeepAspectRatio, Qt::FastTransformation);
          }
        }

        tn.save(cacheFile, "PNG");
        QLOG_TRACE() << "ThumbnailCreator :: Saving thumbnail as " << cacheFile;
      }
    }

    {
      QMutexLocker lock(&mutex);
      emit pendingThumbnails(list.count());
      if (list.count() == 0) {
        condition.wait(&mutex);
      }
    }
  }
}

ThumbnailCreator::~ThumbnailCreator() {
  stopped = true;
  condition.wakeAll();
  wait();
}

void ThumbnailCreator::setIconSize(QSize s) {
  mutex.lock();
  iconSize->setWidth(s.width());
  iconSize->setHeight(s.height());
  mutex.unlock();
}

QString ThumbnailCreator::addToList(QString s) {
  QString ret;

  mutex.lock();
  if (!list.contains(s)) {
    ret = getCacheFile(s);
    list.append(s);
  }
  mutex.unlock();

  condition.wakeAll();

  return ret;
}

QString ThumbnailCreator::getCacheFile(QString filename) {
  QString tmp, ret;

  auto cacheFolder = settings->value("options/thumbnail_cache_folder", QString("%1/%2").arg(QCoreApplication::applicationDirPath())
    .arg("tncache")).toString();

  tmp = filename;
  tmp.replace(QRegExp("[" + QRegExp::escape("\\/:*?\"<>|") + "]"), QString("_"));
  ret = QString("%1/%2.tn").arg(cacheFolder).arg(tmp);

  return ret;
}

void ThumbnailCreator::stop() {
  stopped = true;
  condition.wakeAll();
}

void ThumbnailCreator::pause()
{
  paused = true;
}

void ThumbnailCreator::resume()
{
  paused = false;
  condition.wakeAll();
}
