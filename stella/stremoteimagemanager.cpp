#include "stremoteimagemanager.h"
#include <QCryptographicHash>
#include <QMutexLocker>
#include <QThread>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDebug>
#include <QDateTime>



STRemoteImageManager::STRemoteImageManager() :
    QObject()
{
    g_ticketsMutex=new QMutex(QMutex::Recursive);
     m_unusedTicketsLimit=256;
     g_netManager=NULL;

    static QThread *thread=NULL;
    if(!thread){
        thread=new QThread();
        thread->start(QThread::LowPriority);

    }
    this->moveToThread(thread);

}

STRemoteImageManager::~STRemoteImageManager(){
    delete g_ticketsMutex;
}


quint64 STRemoteImageManager::hashForUrl(QUrl urlobj){
    QString url=urlobj.toString();
    QByteArray hash=QCryptographicHash::hash(url.toUtf8(), QCryptographicHash::Md5);
    quint64 outHash=*(const quint64 *)hash.constData();
   // outHash<<=32;
    //outHash^=user->id;
    return outHash;
}

void STRemoteImageManager::purgeOldUnusedTickets(){
    QMutexLocker mutex(g_ticketsMutex);
    while(g_unusedTickets.size()>m_unusedTicketsLimit){
        Ticket *t=g_unusedTickets.last();
        t->mutex.lock();
        if(t->reply){
            t->reply->abort();;
            t->reply->deleteLater();
        }
        g_unusedTickets.removeLast();
        g_tickets.remove(t->hash);
        Q_ASSERT(t->refCount==0);
        delete t;
    }
}

void STRemoteImageManager::Ticket::retain(){
    QMutexLocker locker(&mutex);
    if(refCount==0){
        locker.unlock();
        QMutexLocker mutex(manager->g_ticketsMutex);
        if(unusedListIterator!=manager->g_unusedTickets.end());
            manager->g_unusedTickets.erase(unusedListIterator);
        unusedListIterator=manager->g_unusedTickets.end();
    }
    refCount++;
}

void STRemoteImageManager::Ticket::release(){
    QMutexLocker locker(&mutex);
    Q_ASSERT(refCount>0);
    refCount--;
    if(refCount==0){
        // abort loading
        if(reply){
            reply->abort();
            reply->deleteLater();;
            reply=NULL;
        }

        locker.unlock();
        QMutexLocker mutex(manager->g_ticketsMutex);
        manager->g_unusedTickets.push_front(this);
        unusedListIterator=manager->g_unusedTickets.begin();

        locker.unlock();;
        manager->purgeOldUnusedTickets();
    }

}

STRemoteImageManager::TicketRef STRemoteImageManager::requestUrl(QUrl url){
   // return TicketRef();
    //Q_ASSERT(url.toString()!="http://p.twimg.com/A4hUGxACYAA5f-V.jpg:small");
    QMutexLocker mutex(g_ticketsMutex);
    quint64 hash=hashForUrl(url);
    if(g_tickets.contains(hash)){
        Ticket *t=g_tickets[hash];
        QMutexLocker locker(&(t->mutex));
        if(!t->image.isNull()){
            return TicketRef(t);
        }

        if(QDateTime::currentMSecsSinceEpoch()<t->failedTime+5000){
            // icon that failed to load
            return TicketRef(t);
        }
        if(t->refCount==0){
            // maybe need to start network request
            // because if refCount was 0, doRequest doesn't send the request.
            TicketRef ref(t);

            this->metaObject()->invokeMethod(this, "processRequests", Qt::QueuedConnection);

            return ref;
        }else{
            // being requested.
            return TicketRef(t);
        }
    }else{
        // newbie icon.
        Ticket *t=new Ticket();
        t->hash=hash;
        t->unusedListIterator=g_unusedTickets.end();
        t->url=url;
        t->reply=NULL;
        t->failedTime=0;
        t->manager=this;
        //qDebug()<<"newreq: "<<hash<<" @ "<<url;

        g_tickets.insert(hash, t);

        TicketRef ref(t); // make refCount!=0 so that doRequest actually does the request.

        this->metaObject()->invokeMethod(this, "processRequests", Qt::QueuedConnection);

        return ref;
    }
}

//static quint64 g_currentRequestHash=0;

void STRemoteImageManager::processRequests(){
    QMutexLocker mutex(g_ticketsMutex);

    for(TicketPtrMap::iterator it=g_tickets.begin();it!=g_tickets.end();it++){
        Ticket *t=it.value();
        if(t->url.isEmpty())
            continue;
        if(t->refCount==0){
            continue;
        }
        if(t->image.isNull() && !t->reply && QDateTime::currentMSecsSinceEpoch()>=t->failedTime+5000){
            mutex.unlock();
            //qDebug()<<"PROC: "<<t->url;
            doRequest(t->hash, t->url);
            mutex.relock();
        }
    }

}


bool STRemoteImageManager::doRequest(quint64 hash, QUrl url){
    // first, make sure this icon is still needed
    // because the call to doRequest may be delayed (QueuedConnection).
    {
        QMutexLocker mutex(g_ticketsMutex);
        if(!g_tickets.contains(hash))
            return false;
        Ticket *t=g_tickets[hash];
        mutex.unlock();
        QMutexLocker locker(&(t->mutex));
        if(!t->image.isNull())
            return false;
        if(t->refCount==0)
            return false;
    }

    // do request.
    if(!g_netManager){
        g_netManager=new QNetworkAccessManager(this);
    }

    QNetworkRequest req;
    req.setUrl(url);
    req.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute,true);

    QNetworkReply *reply=g_netManager->get(req);
    if(!reply->isRunning()){
        qWarning()<<"STRemoteImageManager: request error: "<<reply->errorString();
        reply->deleteLater();

        {
            QMutexLocker mutex(g_ticketsMutex);
            if(!g_tickets.contains(hash))
                return false;
            Ticket *t=g_tickets[hash];
            mutex.unlock();
            QMutexLocker locker(&(t->mutex));
            t->failedTime=QDateTime::currentMSecsSinceEpoch();
            t->reply=NULL;

        }

        return false;
    }else{
        {
            QMutexLocker mutex(g_ticketsMutex);
            if(!g_tickets.contains(hash)){
                reply->deleteLater();
                return false;
            }
            Ticket *t=g_tickets[hash];
            mutex.unlock();
            QMutexLocker locker(&(t->mutex));
            t->failedTime=0;
            t->reply=reply;
        }
    }
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(slotError()));
    connect(reply, SIGNAL(finished()),
            this, SLOT(readyRead()));
    Q_ASSERT(!reply->isFinished());

    return true;
}

void STRemoteImageManager::slotError(){
    QMutexLocker mutex(g_ticketsMutex);
    QNetworkReply *reply=(QNetworkReply *)sender();
    bool canceled=reply->error()==QNetworkReply::OperationCanceledError;
    if(!canceled)
    qWarning()<<"STRemoteImageManager: error reply: "<<reply->errorString();
    reply->deleteLater();


    for(TicketPtrMap::iterator it=g_tickets.begin();it!=g_tickets.end();it++){
        Ticket *t=it.value();
        if(t->reply==reply){
            QMutexLocker locker(&(t->mutex));
            if(canceled)
                t->failedTime=0;
            else
            t->failedTime=QDateTime::currentMSecsSinceEpoch();
            t->reply=NULL;
            break;
        }
    }


}

void STRemoteImageManager::readyRead(){
    QNetworkReply *reply=(QNetworkReply *)sender();
    if(!reply) // already deleted
        return;

    QVariant redirectTarget=reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
    if(!redirectTarget.isNull()){
         reply->deleteLater();

        QUrl target=redirectTarget.toUrl();
        QString targetStr=target.toString();
        QMutexLocker mutex(g_ticketsMutex);

        // yfrog work-around
        {
            if(targetStr.startsWith("http://yfrog.com/sclaed/")){
                targetStr.data()[19]=QChar('a');
                targetStr.data()[20]=QChar('l');
                target=QUrl(targetStr);
            }
        }

        for(TicketPtrMap::iterator it=g_tickets.begin();it!=g_tickets.end();it++){
            Ticket *t=it.value();
            if(t->reply==reply){
                QMutexLocker locker(&(t->mutex));

                if(t->lastRedirectTargets.contains(targetStr) || t->lastRedirectTargets.count()>16){
                    // redirecion loop/over-limit detected
                    // treat as error
                    t->failedTime=QDateTime::currentMSecsSinceEpoch();
                    t->reply=NULL;
                    qWarning()<<"STRemoteImageManager: redirection loop "<<targetStr<<": "<<t->url;
                    return;
                }

                t->lastRedirectTargets.insert(targetStr);
                t->reply=NULL;

                QNetworkRequest req;
                req.setUrl(target);
                req.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute,true);

                QNetworkReply *reply=g_netManager->get(req);

                if(!reply->isRunning()){
                    qWarning()<<"STRemoteImageManager: request error: "<<reply->errorString();
                    reply->deleteLater();
                    t->failedTime=QDateTime::currentMSecsSinceEpoch();
                    return;
                }else{
                    t->failedTime=0;
                }

                connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
                        this, SLOT(slotError()));
                connect(reply, SIGNAL(finished()),
                        this, SLOT(readyRead()));
                Q_ASSERT(!reply->isFinished());
                t->reply=reply;

                break;
            }
        }

        return;
    }

    QByteArray data=reply->readAll();
    QString err=reply->errorString();
    reply->deleteLater();


    QImage img;
    img.loadFromData(data);

    // now loaded.

    QMutexLocker mutex(g_ticketsMutex);
    for(TicketPtrMap::iterator it=g_tickets.begin();it!=g_tickets.end();it++){
        Ticket *t=it.value();
        if(t->reply==reply){
            QMutexLocker locker(&(t->mutex));

            if(img.isNull()){
                qWarning()<<"STRemoteImageManager: image not returned: "<<reply->errorString()<<", url: "<<reply->request().url();
                t->failedTime=QDateTime::currentMSecsSinceEpoch();;
            }else{
                t->image=img;
                t->failedTime=0;
                emit iconReady(t->hash);
            }
            t->reply=NULL;

            break;
        }
    }


}
