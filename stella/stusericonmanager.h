#ifndef STUSERICONMANAGER_H
#define STUSERICONMANAGER_H

#include <QObject>
#include "stobjectmanager.h"
#include <QMutex>
#include <QLinkedList>
#include <QPixmap>
#include <QUrl>
#include <QNetworkReply>
#include "stremoteimagemanager.h"

class STUserIconManager : public STRemoteImageManager
{
    Q_OBJECT
public:
    explicit STUserIconManager();
    static STUserIconManager *sharedManager();

    TicketRef requestUserIcon(const STObjectManager::UserRef&);

signals:
private slots:
};

#endif // STUSERICONMANAGER_H
