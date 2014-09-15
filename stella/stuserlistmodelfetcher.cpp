#include "stuserlistmodelfetcher.h"
#include <QtKOAuth>
#include "stjsonparser.h"
#include "staccount.h"
#include "stnetworkreplyerrorparser.h"
#include <QDebug>


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

STUserListModelFetcher::STUserListModelFetcher() :
    QObject(NULL)
{

    m_nextCursor="-1";
    m_manager=NULL;
    m_request=NULL;

    moveToThread(fetcherThread());


}

STUserListModelFetcher::~STUserListModelFetcher(){
    if(m_manager){
        m_manager->abortCurrentRequest();
    }

}

bool STUserListModelFetcher::hasMore(){
    QMutexLocker locker(&m_mutex);
    return m_nextCursor!="0";
}

void STUserListModelFetcher::doRequest(QUrl url, STAccount *account){
    if(thread()!=QThread::currentThread()){
        metaObject()->invokeMethod(this, "doRequest", Qt::QueuedConnection,
                                   Q_ARG(QUrl, url), Q_ARG(STAccount*, account));
        return;
    }

    if(!m_manager){
        m_manager=new KQOAuthManager(this);
        connect(m_manager, SIGNAL(requestReadyEx(QByteArray,int)),
                this, SLOT(requestDone(QByteArray,int)));
    }

    if(m_request){
        m_request->deleteLater();
    }
    m_request=new KQOAuthRequest(this);

    KQOAuthParameters param;
    param.insert("cursor", m_nextCursor);
    typedef QPair<QString, QString> StringPair;
    foreach(StringPair pair, url.queryItems()){
        param.insert(pair.first, pair.second);
    }
    url.setQueryItems(QList<QPair<QString, QString> >());

    qDebug()<<"cursor: "<<m_nextCursor<<", "<<url;

    m_request->initRequest(KQOAuthRequest::AuthorizedRequest, url);
    m_request->setAdditionalParameters(param);
    m_request->setHttpMethod(KQOAuthRequest::GET);
    m_request->setConsumerKey(account->consumerKey());
    m_request->setConsumerSecretKey(account->consumerSecret());
    m_request->setToken(account->accessToken());
    m_request->setTokenSecret(account->accessTokenSecret());


    m_manager->executeRequest(m_request);

    if(m_manager->lastError()!=KQOAuthManager::NoError){
        m_manager->abortCurrentRequest();
        m_request->deleteLater();
        m_request=NULL;
        emit fetchFailed(tr("Request failed."));
    }
}

void STUserListModelFetcher::requestDone(QByteArray data, int code){
    m_request->deleteLater();
    m_request=NULL;

    QVariant var=STJsonParser().parse(data);

    if(var.canConvert(QVariant::Map)){
        QVariantMap map=var.toMap();
        if(map.contains("error")){
            emit fetchFailed(tr("Error returned. (%n)",0,1));
            return;
        }else if(map.contains("errors")){
            QVariantList lst=map["errors"].toList();
            foreach(const QVariant& var, lst){
                QVariantMap mp=var.toMap();
                if(mp.contains("message")){
                    emit fetchFailed(mp["message"].toString());
                    return;
                }
            }
            emit fetchFailed(tr("Error returned. (%n)",0,2));
            return;
        }else if(map.contains("ids")||map.contains("users")){
            if(map.contains("users")){
                {
                    QMutexLocker locker(&m_mutex);
                    m_nextCursor=map["next_cursor_str"].toString();
                }
                parseReturnedUsers(map["users"].toList());
            }else if(map.contains("ids")){
                {
                    QMutexLocker locker(&m_mutex);
                    m_nextCursor=map["next_cursor_str"].toString();
                }
                parseReturnedIds(map["ids"].toList());
            }else{
                emit fetchFailed(tr("Couldn't parse the returned data (%n).",0,1));
                return;
            }

        }else{
            emit fetchFailed(STNetworkReplyErrorParser::parseError(code));
            return;
        }
    }else{
        emit fetchFailed(STNetworkReplyErrorParser::parseError(code));
        return;
    }

}

void STUserListModelFetcher::parseReturnedIds(QVariantList ids){
    QList<quint64> out;
    out.reserve(ids.count());
    foreach(const QVariant& var, ids){
        out.append(var.toULongLong());
    }
    emit userIdsFetched(out);
}

void STUserListModelFetcher::parseReturnedUsers(QVariantList users){
    QList<STObjectManager::UserRef> out;
    out.reserve(users.count());
    foreach(const QVariant& var, users){
        out.append(STObjectManager::sharedManager()->user(var));
    }
    emit usersFetched(out);
}
