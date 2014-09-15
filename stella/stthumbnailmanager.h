#ifndef STTHUMBNAILMANAGER_H
#define STTHUMBNAILMANAGER_H

#include "stremoteimagemanager.h"

class STThumbnailManager : public STRemoteImageManager
{
    Q_OBJECT
public:
    explicit STThumbnailManager();
    static STThumbnailManager *sharedManager();
signals:
    
public slots:
    
};

#endif // STTHUMBNAILMANAGER_H
