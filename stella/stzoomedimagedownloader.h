#ifndef STZOOMEDIMAGEDOWNLOADER_H
#define STZOOMEDIMAGEDOWNLOADER_H

#include "ststdafx.h"
#include <QObject>
#include <QPixmap>
#include <QNetworkReply>

class STZoomedImageDownloader : public QObject
{
    Q_OBJECT
public:
    explicit STZoomedImageDownloader(QObject *parent = 0);
    static STZoomedImageDownloader *sharedDownloader();

public slots:
    void requestLoad(QUrl);

signals:
    void imageLoadFailed(QUrl, QString reason);
    void imageLoaded(QUrl, QPixmap img);
    void imageLoadProgress(QUrl, float);
    void imageLoadStarted(QUrl);
private slots:
    void finished();
    void downloadProgress(qint64, qint64);
    void errorReceived(QNetworkReply::NetworkError);
};

#endif // STZOOMEDIMAGEDOWNLOADER_H
