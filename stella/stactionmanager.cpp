#include "ststdafx.h"
#include "stactionmanager.h"
#include <QThread>
#include <QtKOAuth>
#include <QLinkedList>
#include "staccount.h"
#include "staccountsmanager.h"
#include "stobjectmanager.h"
#include "stjsonparser.h"
#include "stnetworkreplyerrorparser.h"
#include <QDebug>

STActionManager::STActionManager(QObject *parent) :
    QObject(parent)
{
    QThread *thread=new QThread();
    thread->start(QThread::LowPriority);
    moveToThread(thread);

    qRegisterMetaType<STActionManager::StatusAction>("STActionManager::StatusAction");

    connect(STAccountsManager::sharedManager(), SIGNAL(accountBeingRemoved(quint64)),
            this, SLOT(accountWillRemoved(quint64)), Qt::BlockingQueuedConnection);
}

STActionManager *STActionManager::sharedManager(){
    static STActionManager *manager=NULL;
    if(!manager){
        manager=new STActionManager();
    }
    return manager;
}

struct StatusActionRequest{
    KQOAuthManager *manager;
    KQOAuthRequest *request;
    quint64 statusId;
    STActionManager::StatusAction action;
    STAccount* account;
};

static QLinkedList<StatusActionRequest> g_statusRequests;

void STActionManager::statusAction(quint64 statusId, quint64 accountId, StatusAction action){
    if(QThread::currentThread()!=thread()){
        this->metaObject()->invokeMethod(this, "statusAction", Qt::QueuedConnection,
                                         Q_ARG(quint64, statusId), Q_ARG(quint64, accountId),
                                         Q_ARG(STActionManager::StatusAction, action));
        return;
    }

    STAccount *account=STAccountsManager::sharedManager()->account(accountId);

    StatusActionRequest req;
    req.statusId=statusId;
    req.action=action;
    req.account=account;

    QLinkedList<StatusActionRequest>::Iterator it;
    it=g_statusRequests.end();
    while(it!=g_statusRequests.begin()){
        it--;
        StatusActionRequest& req2=*it;
        if(req2.account!=req.account)
            continue;
        switch(action){
        case Favorite:
            if(req2.statusId!=req.statusId)
                continue;
            if(req2.action==Favorite)
                return;
            if(req2.action==Unfavorite)
                break;
            break;
        case Unfavorite:
            if(req2.statusId!=req.statusId)
                continue;
            if(req2.action==Unfavorite)
                return;
            if(req2.action==Favorite)
                break;
            break;
        case Retweet:
            if(req2.statusId!=req.statusId)
                continue;
            if(req2.action==Retweet)
                return;
            break;
        case DeleteStatus:
            if(req2.statusId!=req.statusId)
                continue;
            if(req2.action==DeleteStatus)
                return;
            break;
        }
    }

    req.manager=new KQOAuthManager(this);
    req.request=new KQOAuthRequest(this);

    KQOAuthParameters params;

    switch(action){
    case Favorite:
        req.request->initRequest(KQOAuthRequest::AuthorizedRequest,
                                 QUrl("https://api.twitter.com/1.1/favorites/create.json"));
        req.request->setHttpMethod(KQOAuthRequest::POST);
        params.insert("id", QString::number(statusId));
        break;
    case Unfavorite:
        req.request->initRequest(KQOAuthRequest::AuthorizedRequest,
                                 QUrl("https://api.twitter.com/1.1/favorites/destroy.json"));
        req.request->setHttpMethod(KQOAuthRequest::POST);
        params.insert("id", QString::number(statusId));
        break;
    case Retweet:
        req.request->initRequest(KQOAuthRequest::AuthorizedRequest,
                                 QUrl(QString("https://api.twitter.com/1.1/statuses/retweet/%1.json").arg(statusId)));
        req.request->setHttpMethod(KQOAuthRequest::POST);
        break;
    case DeleteStatus:
        req.request->initRequest(KQOAuthRequest::AuthorizedRequest,
                                 QUrl(QString("https://api.twitter.com/1.1/statuses/destroy/%1.json").arg(statusId)));
        req.request->setHttpMethod(KQOAuthRequest::POST);
        break;
    }

    req.request->setConsumerKey(account->consumerKey());
    req.request->setConsumerSecretKey(account->consumerSecret());
    req.request->setToken(account->accessToken());
    req.request->setTokenSecret(account->accessTokenSecret());

    req.request->setAdditionalParameters(params);

    g_statusRequests.push_back(req);
    connect(req.manager, SIGNAL(requestReadyEx(QByteArray,int)),
            this, SLOT(statusRequestDone(QByteArray,int)));

    req.manager->executeRequest(req.request);
    if(req.manager->lastError()!=KQOAuthManager::NoError){
        account->reportError(tr("Bad request."));
        req.manager->deleteLater();;
        req.request->deleteLater();
        g_statusRequests.pop_back();
    }

}

void STActionManager::accountWillRemoved(quint64 accountId){
    QVector<QLinkedList<StatusActionRequest>::iterator> trash;
    for(QLinkedList<StatusActionRequest>::iterator it=g_statusRequests.begin();
        it!=g_statusRequests.end();it++){
        StatusActionRequest& req=*it;
        if(req.account->userId()==accountId){
            req.manager->blockSignals(true);
            req.manager->abortCurrentRequest();
            req.manager->deleteLater();;
            req.request->deleteLater();
            trash<<it;
        }
    }
    foreach(QLinkedList<StatusActionRequest>::iterator it, trash){
        g_statusRequests.erase(it);
    }
}

void STActionManager::statusRequestDone(QByteArray data, int code){
    KQOAuthManager *manager=dynamic_cast<KQOAuthManager *>(sender());
    Q_ASSERT(manager);

    for(QLinkedList<StatusActionRequest>::iterator it=g_statusRequests.begin();
        it!=g_statusRequests.end();it++){

        if(it->manager==manager){
            StatusActionRequest req=*it;
            g_statusRequests.erase(it);

            STAccount *account=req.account;
            req.manager->deleteLater();;
            req.request->deleteLater();

            QVariantMap map=STJsonParser().parse(data).toMap();
            if(map.isEmpty()){
                account->reportError(STNetworkReplyErrorParser::parseError(code));
                emit statusActionFailed(req.statusId, req.account->userId(),
                                        req.action);
                return;
            }

            if(map.contains("error")){
                // maybe something like "already favorited"?
                return;
            }

            if(map.contains("errors")){
                QVariantList lst=map["errors"].toList();
                foreach(const QVariant& var, lst){
                    QVariantMap mp=var.toMap();
                    if(mp.contains("message")){
                        QString msg=mp["message"].toString();
                        if(msg.contains("already favorited")){
                            return;
                        }
                        if(msg.contains("already retwe")){
                            return;
                        }

                        account->reportError(mp["message"].toString());
                        emit statusActionFailed(req.statusId, req.account->userId(),
                                                req.action);
                        return;
                    }
                }

                account->reportError(tr("Unknown error."));
                emit statusActionFailed(req.statusId, req.account->userId(),
                                        req.action);
                return;
            }

            STObjectManager::StatusRef status;
            status=STObjectManager::sharedManager()->status(map, false, account->userId());

            switch(req.action){
            case Favorite:
                status->setFavorited(req.account->userId(), true);
                break;
            case Unfavorite:
                status->setFavorited(req.account->userId(), false);
                break;
            case Retweet:
                if(!status->retweetedStatus){
                    qWarning()<<"STActionManager::statusRequestDone: incorrect retweet response";
                    account->reportError(tr("Incorrect retweet response."));
                    return;
                }
                status->retweetedStatus->setRetweetStatusId(req.account->userId(), status->id);
                break;
            case DeleteStatus:
                if(status->retweetedStatus){
                    status->retweetedStatus->setRetweetStatusId(req.account->userId(), 0);
                }
                break;
            }

            emit statusActionDone(status, req.account->userId(), req.action);
            return;
        }
    }

}
