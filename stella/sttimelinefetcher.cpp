#include "sttimelinefetcher.h"
#include <QThread>
#include <QtKOAuth>
#include "stjsonparser.h"
#include "stobjectmanager.h"
#include "staccount.h"
#include <QDebug>
#include "stnetworkreplyerrorparser.h"
#include <QJson/Serializer>

static QThread *fetcherThread(){
    static QThread * volatile thread=NULL;
    static QMutex mutex;
    QMutexLocker locker(&mutex);
    if(!thread){
        thread=new QThread();
        thread->start(QThread::LowPriority);
    }
    return thread;
}

STTimelineFetcher::STTimelineFetcher() :
    QObject()
{
    this->moveToThread(fetcherThread());

}

STTimelineFetcher::~STTimelineFetcher(){
}

void STTimelineFetcher::requestFetch(STTimeline::FetchRequest req, QUrl url,
                                     STAccount *account){
    for(int i=0;i<m_requests.size();i++){
        if(m_requests[i].req.requestId==req.requestId){
            // duplicate
            return;
        }
    }

    Request r;
    r.manager=new KQOAuthManager(this);
    r.request=new KQOAuthRequest(this);
    r.account=account;
    r.req=req;

    KQOAuthParameters param;
    if(req.minId>3) // ooo
        param.insert("since_id", QString::number(req.minId-1));
    if(req.maxId)
        param.insert("max_id", QString::number(req.maxId));
    typedef QPair<QString, QString> StringPair;
    foreach(StringPair pair, url.queryItems()){
        param.insert(pair.first, pair.second);
    }
    qDebug()<<req.minId<<"-"<<req.maxId<<": "<<url;
    url.setQueryItems(QList<QPair<QString, QString> >());

    param.insert("count", "80");

    r.request->initRequest(KQOAuthRequest::AuthorizedRequest, url);
    r.request->setAdditionalParameters(param);
    r.request->setHttpMethod(KQOAuthRequest::GET);
    r.request->setConsumerKey(account->consumerKey());
    r.request->setConsumerSecretKey(account->consumerSecret());
    r.request->setToken(account->accessToken());
    r.request->setTokenSecret(account->accessTokenSecret());

    connect(r.manager, SIGNAL(requestReadyEx(QByteArray,int)),
            this, SLOT(requestDone(QByteArray,int)));

    /*
    connect(r.streamer, SIGNAL(streamingStopped()),
            this, SLOT(streamerDone()));
    connect(r.streamer, SIGNAL(streamingLineReceived(QByteArray)),
            this, SLOT(streamerReceived(QByteArray)));
    connect(r.streamer, SIGNAL(streamingErrorStarting()),
            this, SLOT(streamerStartFailed()));*/

    r.manager->executeRequest(r.request);

    m_requests.push_back(r);

    // TODO: timeout
}

void STTimelineFetcher::abortFetch(quint64 id){
    for(int i=0;i<m_requests.size();i++){
        if(m_requests[i].req.requestId==id){
            m_requests[i].manager->abortCurrentRequest();
            m_requests[i].manager->deleteLater();
            m_requests[i].request->deleteLater();
            m_requests.removeAt(i);
            return;
        }
    }
}

int STTimelineFetcher::indexForObject(KQOAuthManager *obj){
    for(int i=0;i<m_requests.size();i++){
        if(m_requests[i].manager==obj)
            return i;
    }
    return -1;
}

void STTimelineFetcher::requestDone(QByteArray data, int code){
    int index=indexForObject((KQOAuthManager *)sender());
    if(index==-1)return;
    if(code){
        qDebug()<<"STTimelineFetcher::requestDone: HTTP Status: "<<code;
    }

    Request req=m_requests[index];
    m_requests.removeAt(index);
    req.manager->deleteLater();
    req.request->deleteLater();

    if(data.isEmpty()){
        emit fetchFailed(STNetworkReplyErrorParser::parseError((QNetworkReply::NetworkError)code),
                         req.req.requestId);
        return;
    }

    STJsonParser parser;
    QVariant var=parser.parse(data);
    if(var.isNull()){
        qDebug()<<"parse failed: "<<data;
        emit fetchFailed("Couldn't parse the returned data.",
                         req.req.requestId);
        return;
    }
/*
    qDebug()<<" *************************************** ";
    qDebug()<<" *************************************** ";
    qDebug()<<" *************************************** ";
    qDebug()<<" *************************************** ";
    qDebug()<<" *************************************** ";
    qDebug()<<data;
    qDebug()<<"#####################OUT ";
    qDebug()<<QJson::Serializer().serialize(var);
*/
    again:
    if(var.canConvert(QVariant::List)){
        QVariantList lst=var.toList();
        QList<STObjectManager::StatusRef> statuses;
        statuses.reserve(lst.size());
        foreach(const QVariant& var, lst){
            STObjectManager::StatusRef ref=STObjectManager::sharedManager()->status(var, false,
                                                                                                    req.account->userId());
            if(!ref){
                // TODO: something to do this...
                continue;
            }
            statuses.push_back(ref);
        }

        emit fetchCompleted(statuses, req.req.requestId);
    }else if(var.canConvert(QVariant::Map)){
        QVariantMap map=var.toMap();
        if(map.contains("error")){
            if(map["error"].toString()=="Not authorized"){
                emit fetchFailed(tr("Access forbidden."),
                                 req.req.requestId);
                return;
            }
            emit fetchFailed(map["error"].toString(),
                             req.req.requestId);
            return;
        }else if(map.contains("errors")){
            QVariantList lst=map["errors"].toList();
            foreach(const QVariant& var, lst){
                QVariantMap mp=var.toMap();
                if(mp.contains("message")){
                    emit fetchFailed(mp["message"].toString(),
                                     req.req.requestId);
                    return;
                }
            }
            emit fetchFailed("Error returned (2).",
                             req.req.requestId);
            return;
        }else if(map.contains("statuses")){
            var=map["statuses"];
            goto again;
        }else{
            emit fetchFailed("Couldn't parse the returned data (3).",
                             req.req.requestId);
            return;
        }
    }else{
        emit fetchFailed("Couldn't parse the returned data (2).",
                         req.req.requestId);
        return;
    }

}
