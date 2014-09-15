#include "stuserlistmodelloader.h"
#include <QtKOAuth>
#include <QStringBuilder>
#include "stjsonparser.h"
#include "staccount.h"
#include "stnetworkreplyerrorparser.h"
#include <QDebug>


static QThread *fetcherLoader(){
    static QThread * volatile thread=NULL;
    static QMutex mutex;
    QMutexLocker locker(&mutex);
    if(!thread){
        thread=new QThread();
        thread->start(QThread::LowPriority);
    }
    return thread;
}

STUserListModelLoader::STUserListModelLoader(QObject *parent) :
    QObject(parent)
{
    this->moveToThread(fetcherLoader());
}
STUserListModelLoader::~STUserListModelLoader(){

}

void STUserListModelLoader::doRequest(QSet<quint64> userIds, STAccount *account, int pageId){
    if(thread()!=QThread::currentThread()){
        metaObject()->invokeMethod(this, "doRequest", Qt::QueuedConnection,
                                   Q_ARG(QSet<quint64>, userIds), Q_ARG(STAccount*, account),
                                   Q_ARG(int, pageId));
        return;
    }

    // find request with the same pageId
    for(QMap<const KQOAuthManager*, Request>::iterator it=m_requests.begin();
        it!=m_requests.end();it++){
        Request& req=it.value();
        if(req.pageId==pageId){
            // found; abort old one
            req.manager->blockSignals(true);
            req.manager->abortCurrentRequest();
            req.manager->deleteLater();
            req.request->deleteLater();
            m_requests.erase(it);
            break;
        }
    }

    Request r;
    r.manager=new KQOAuthManager(this);
    r.request=new KQOAuthRequest(this);
    r.pageId=pageId;

    QString strs;
    KQOAuthParameters param;

    foreach(quint64 id, userIds){
        if(!strs.isEmpty())
            strs+=',';
        strs+=QString::number(id);
    }

    param.insert("user_id", strs);

    qDebug()<<"loading "<<userIds.count()<<" users for page "<<pageId;

    static const QUrl url("https://api.twitter.com/1.1/users/lookup.json");
    r.request->initRequest(KQOAuthRequest::AuthorizedRequest, url);
    r.request->setAdditionalParameters(param);
    r.request->setHttpMethod(KQOAuthRequest::POST);
    r.request->setConsumerKey(account->consumerKey());
    r.request->setConsumerSecretKey(account->consumerSecret());
    r.request->setToken(account->accessToken());
    r.request->setTokenSecret(account->accessTokenSecret());

    connect(r.manager, SIGNAL(requestReadyEx(QByteArray,int)),
            this, SLOT(requestDone(QByteArray,int)));

    r.manager->executeRequest(r.request);
    if(r.manager->lastError()!=KQOAuthManager::NoError){
        r.manager->blockSignals(true);
        r.manager->abortCurrentRequest();;
        r.request->deleteLater();
        r.manager->deleteLater();
        emit loadFailed(tr("Request failed."), pageId);
        return;
    }

    m_requests.insert(r.manager, r);
}

void STUserListModelLoader::requestDone(QByteArray data, int code){
    KQOAuthManager *manager=static_cast<KQOAuthManager *>(sender());
    Request r=m_requests.take(manager);
    r.manager->deleteLater();;
    r.request->deleteLater();

    if(data.isEmpty()){
        emit loadFailed(STNetworkReplyErrorParser::parseError((QNetworkReply::NetworkError)code),
                        r.pageId);
        return;
    }

    STJsonParser parser;
    QVariant var=parser.parse(data);
    if(var.isNull()){
        emit loadFailed(tr("Couldn't parse the returned data. (%n)",0,1),
                         r.pageId);
        return;
    }
    if(var.canConvert(QVariant::List)){
        QVariantList lst=var.toList();
        QList<STObjectManager::UserRef> users;
        users.reserve(lst.count());
        foreach(const QVariant& v, lst){
            users.append(STObjectManager::sharedManager()->user(v));
        }
        emit loaded(r.pageId, users);
    }else if(var.canConvert(QVariant::Map)){
        QVariantMap map=var.toMap();
        if(map.contains("error")){
            emit loadFailed(tr("Error returned."),
                             r.pageId);
            return;
        }else if(map.contains("errors")){
            QVariantList lst=map["errors"].toList();
            foreach(const QVariant& var, lst){
                QVariantMap mp=var.toMap();
                if(mp.contains("message")){
                    emit loadFailed(mp["message"].toString(),
                                     r.pageId);
                    return;
                }
            }
            emit loadFailed(tr("Error returned."),
                             r.pageId);
            return;
        }else{
            emit loadFailed(tr("Couldn't parse the returned data (%n).",0,3),
                            r.pageId);
            return;
        }
    }else{
        emit loadFailed(tr("Couldn't parse the returned data (%n).",0,2),
                        r.pageId);
        return;
    }

}
