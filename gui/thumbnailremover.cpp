﻿#include "thumbnailremover.h"

ThumbnailRemover::ThumbnailRemover(QObject *parent) :
    QObject(parent)
{
}

void ThumbnailRemover::removeOutdated() {
    QDateTime date;
    mutex.lock();
    dirName = settings.getThumbnailCacheFolder();
    ttl = settings.getThumbnailTTL();
    mutex.unlock();

    dir.setPath(dirName);
    fileInfoList = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);

    foreach (QFileInfo fi, fileInfoList) {
        date = fi.lastModified();
        if (date.addDays(ttl) < QDateTime::currentDateTime()) {
            QFile::remove(fi.absoluteFilePath());
            QLOG_INFO() << "ThumbnailRemover :: removed " << fi.absoluteFilePath() << " because it was created on " << date.toString("dd.MM.yyyy hh:mm:ss");
        }
    }
    emit filesRemoved();
}

void ThumbnailRemover::removeFiles(QStringList fileList) {
    foreach (QString s, fileList) {
        QFile::remove(s);
    }

    emit filesRemoved();
}

void ThumbnailRemover::removeAll() {
    if (dir.exists())//QDir::NoDotAndDotDot
    {
        fileInfoList = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);

        foreach (QFileInfo fi, fileInfoList) {
            QFile::remove(fi.absoluteFilePath());
        }
    }

    emit filesRemoved();
}

void ThumbnailRemover::stop() {
}
