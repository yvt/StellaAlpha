#include "stusericonmanager.h"
#include <QCryptographicHash>
#include <QMutexLocker>
#include <QThread>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDebug>
#include <QDateTime>

STUserIconManager::STUserIconManager() :
    STRemoteImageManager()
{

}


STUserIconManager *STUserIconManager::sharedManager(){
    static STUserIconManager *manager=NULL;
    if(!manager){
        manager=new STUserIconManager();
    }
    return manager;
}

static QString profileImageUrlForUser(const STObjectManager::UserRef& user){
    QVariant var=user->data["profile_image_url"];
    return var.toString();
}
STRemoteImageManager::TicketRef STUserIconManager::requestUserIcon(const STObjectManager::UserRef &ur){
    return requestUrl(QUrl(profileImageUrlForUser(ur)));
}
