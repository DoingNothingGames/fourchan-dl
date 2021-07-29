#ifndef REQUESTHANDLER_H
#define REQUESTHANDLER_H

#include <memory>

#include <QObject>
#include <QHash>
#include <QUrl>

#include "defines.h"
//#include "downloadmanager.h"

class DownloadManager;

class RequestHandler : public QObject
{
    Q_OBJECT;
public:
    RequestHandler(std::shared_ptr<DownloadManager> downloadManager_, QObject *parent = nullptr);
    void request(QUrl, int priority=-1);
    void cancel(QUrl url);
    void cancelAll();
    void requestFinished(qint64);
    void error(qint64, int);
signals:
    void response(QUrl, QByteArray, bool);
    void responseError(QUrl, int);

private:
  QHash<qint64, QUrl> requests;
  std::shared_ptr<DownloadManager> downloadManager;
};

#endif // REQUESTHANDLER_H
