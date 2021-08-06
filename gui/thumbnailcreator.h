#ifndef THUMBNAILCREATOR_H
#define THUMBNAILCREATOR_H

#include <QObject>
#include <QImage>
#include <QList>
#include <QStringList>
#include <QTimer>
#include <QSettings>
#include <QMutex>
#include <QWaitCondition>
#include <QThread>

#include "appsettings.h"

class ThumbnailCreator : public QThread
{
  Q_OBJECT
public:
  ThumbnailCreator();
  ~ThumbnailCreator();

  void setIconSize(QSize s);
  QString addToList(QString s);
  QString getCacheFile(QString);

  void stop();
  void pause();
  void resume();

private:
  QStringList list;
  QSize iconSize = QSize(100, 100);
  QAtomicInt stopped = false;
  QAtomicInt paused = false;
  QMutex mutex;
  QWaitCondition condition;
  chandl::AppSettings settings;

signals:
  void pendingThumbnails(int);
  void thumbnailAvailable(QString, QString);
  void thumbnailsAvailable(QString);

private:
  void run() override;
};

#endif // THUMBNAILCREATOR_H
