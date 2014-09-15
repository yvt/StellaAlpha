#ifndef STSTATICMAPIMAGEMANAGER_H
#define STSTATICMAPIMAGEMANAGER_H

#include "stremoteimagemanager.h"

class STStaticMapImageManager : public STRemoteImageManager
{
    Q_OBJECT
public:
    explicit STStaticMapImageManager();
    static STStaticMapImageManager *sharedManager();
    
    STRemoteImageManager::TicketRef requestWithObjects(QVariantMap coords, QVariantMap place, QSize);

signals:
    
public slots:
    
};

#endif // STSTATICMAPIMAGEMANAGER_H
