#ifndef STZOOMEDIMAGEMANAGER_H
#define STZOOMEDIMAGEMANAGER_H

#include "ststdafx.h"
#include <QObject>

class STZoomedImageManager : public QObject
{
    Q_OBJECT
public:
    explicit STZoomedImageManager(QObject *parent = 0);
    static STZoomedImageManager *sharedManager();

    void zoomImageWithUrl(QUrl, bool needsProgressIndicator);
signals:
    
private slots:
    void imageLoadFailed(QUrl, QString reason);
    void imageReceived(QUrl, QPixmap);
    void imageLoadProgress(QUrl, float);

    void imageClosing();
};

#endif // STZOOMEDIMAGEMANAGER_H
