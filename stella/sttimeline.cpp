#include "sttimeline.h"
#include <QStringList>
#include "stsimplestreamingtimeline.h"
#include "sthometimeline.h"
#include "stmentionstimeline.h"
#include "sttimelinefetcher.h"
#include <QThread>
#include "staccount.h"
#include "staccountsmanager.h"
#include <QDebug>
#include "stsingleusertimeline.h"
#include "stsearchtimeline.h"
#include "stfavoritetimeline.h"

/*
  Timeline represents a stream of tweets, like home timeline and list timelime.
  It may be always modified by inserting or removing tweets.

 */

STTimeline::STTimeline(QString identifier, QObject *parent) :
    QObject(parent), m_identifier(identifier)
{

    QStringList lst=identifier.split(".");

    m_account=accountForIdentifier(identifier);

    m_running=false;
    m_prefetching=false;
    m_fetcher=new STTimelineFetcher();

    static bool registered=false;
    if(!registered){
        qRegisterMetaType<STTimeline::Row>("STTimeline::Row");
        qRegisterMetaType<STTimeline::FetchRequest>("STTimeline::FetchRequest");
        qRegisterMetaType<STAccount *>("STAccount *");
        qRegisterMetaType<QList<STObjectManager::StatusRef> >("QList<STObjectManager::StatusRef>");
        registered=true;
    }

    connect(m_fetcher, SIGNAL(fetchCompleted(QList<STObjectManager::StatusRef>,quint64)),
            this, SLOT(fetchCompleted(QList<STObjectManager::StatusRef>,quint64)));
    connect(m_fetcher, SIGNAL(fetchFailed(QString,quint64)),
            this, SLOT(fetchFailed(QString,quint64)));

    m_shouldBulkLoad=true;


}

STTimeline::~STTimeline(){
    //QThread *thread=m_fetcher->thread();
    m_fetcher->blockSignals(true);
    m_fetcher->deleteLater();;
   // thread->wait();
  //  delete thread;
}

STAccount *STTimeline::accountForIdentifier(const QString &identifier){
    QStringList bits=identifier.split(".");
    Q_ASSERT(bits.length()>=2);

    quint64 uid=QString(bits[0]).toULongLong();
    STAccount *account=STAccountsManager::sharedManager()->account(uid);
    return account;
}

STTimeline *STTimeline::createTimeline(QString identifier, QObject *parent){
    QStringList bits=identifier.split(".");
    Q_ASSERT(bits.length()>=2);

    STAccount *account=accountForIdentifier(identifier);

    if(bits[1]=="sample"){
        return new STSimpleStreamingTimeline(identifier,
                                             new STSimpleStreamer(account,
                                                                  QUrl("https://stream.twitter.com/1.1/statuses/sample.json"))
                                         ,parent);
    }else if(bits[1]=="home"){
        return account->homeTimeline();
    }else if(bits[1]=="mentions"){
        return account->mentionsTimeline();
    }else if(bits[1]=="user"){
        Q_ASSERT(bits.size()>=3);
        return new STSingleUserTimeline(identifier, bits[2].toULongLong(), parent);
    }else if(bits[1]=="favorite"){
        Q_ASSERT(bits.size()>=3);
        return new STFavoriteTimeline(identifier, bits[2].toULongLong(), parent);
    }else if(bits[1]=="search"){
        Q_ASSERT(bits.size()>=3);
        bits.pop_front(); bits.pop_front();
        return new STSearchTimeline(identifier, bits.join("."), parent);
    }else if(bits[1]=="kichi"){
        return new STSimpleStreamingTimeline(identifier,
                                             new STSimpleStreamer(account,
                                                                  QUrl("https://stream.twitter.com/1.1/statuses/filter.json?follow=700480856,705049645,705076616,706302079,727921934,740980998,740983568"))
                                         ,parent);
    }else{
        Q_ASSERT(false);
    }
    return NULL;
}

bool STTimeline::fetchSupported(){
    return fetchEndpoint().isValid();
}

bool STTimeline::streamingSupported(){
    return false;
}

bool STTimeline::stoppingStreamingSupported(){
    return true;
}

void STTimeline::startStream(){
    Q_ASSERT(false);
}

void STTimeline::stopStream(){
    Q_ASSERT(false);
}



void STTimeline::start(){
    if(m_running)
        return;
    m_running=true;
    if(streamingSupported()){
        startStream();
        // if fetch is supported, starting of streaming triggers prefetch.
    }else{
        Q_ASSERT(fetchSupported());
        if(m_rows.empty()){
            // auto fetch
            Row newRow;
            newRow.id=1;
            newRow.fetching=true;
            m_rows.insert(newRow.id, newRow);
            emit rowAdded(newRow.id);

            FetchRequest req;
            req.minId=0;
            req.maxId=0;
            req.requestId=newRow.id;
            requestFetch(req);
        }else{
            // add fetch button
            QMap<quint64, Row>::iterator it=m_rows.end();
            it--;

            Row& row=*it;
            if(!row.isFetchMore()){
                Row newRow;
                newRow.id=row.id+1;
                newRow.fetching=false;
                m_rows.insert(newRow.id, newRow);
                emit rowAdded(newRow.id);
            }
        }
    }
}

void STTimeline::stop(){
    if(!stoppingStreamingSupported())
        return;
    if(!m_running)
        return;
    m_running=false;;
    if(streamingSupported()){
        stopStream();
    }

    if(fetchSupported()){
        for(QMap<quint64, Row>::iterator it=m_rows.begin();
            it!=m_rows.end();it++){
            Row& row=*it;
            if(row.fetching){
                fetchAbort(row.id);
                row.fetching=false;
            }
        }
    }
}

bool STTimeline::hasStatusIncludingRetweet(quint64 statusId){
    return m_containingStatuses.contains(statusId);
}

bool STTimeline::insertingStatus(STObjectManager::StatusRef status){
    if(status->retweetedStatus)
        status=status->retweetedStatus;
    if(hasStatusIncludingRetweet(status->id))
        return false;
    m_containingStatuses.insert(status->id);
    return true;
}

void STTimeline::deletingStatus(STObjectManager::StatusRef status){
    if(status->retweetedStatus)
        status=status->retweetedStatus;
    m_containingStatuses.remove(status->id);
}

void STTimeline::streamReceived(const STObjectManager::StatusRef& status){
    // TOOD: pushing to queue until prefetch is done

    if(!status) // null?
        return;

    if(m_prefetching){
        // currently prefetching.
        Q_ASSERT(fetchSupported());
        m_prefetchRows.push_back(status);
        return;
    }

    quint64 id=status->id;
    if(row(id)){
        // already there!
        // TODO: if it should be "Fetch More"...?
        return;
    }

    if(!insertingStatus(status)){
        return;
    }


    Row newRow;
    newRow.id=id;
    newRow.status=status;

    m_rows.insert(id, newRow);

    if(m_shouldBulkLoad){
        // not bulk
        m_shouldBulkLoad=false;
    }

    emit rowAdded(id);

    purgeRows();

}

void STTimeline::streamRemoved(quint64 id){
    QMap<quint64, Row>::iterator it=m_rows.find(id);
    if(it!=m_rows.end()){
        Row& row=it.value();
        if(row.status)
            deletingStatus(row.status);
        m_rows.erase(it);
        emit rowRemoved(id);
    }
}

void STTimeline::streamStarted(){
    if(!streamingSupported())
        return;
    if(fetchSupported()){
        m_prefetchRows.clear();
        m_prefetching=true;

        if(m_rows.empty()){
            // auto fetch
            Row newRow;
            newRow.id=1;
            newRow.fetching=true;
            m_rows.insert(newRow.id, newRow);
            emit rowAdded(newRow.id);

            FetchRequest req;
            req.minId=0;
            req.maxId=0;
            req.requestId=newRow.id;
            m_prefetchId=req.requestId;
            requestFetch(req);
        }else{
            // add fetch button, and auto fetch
            QMap<quint64, Row>::iterator it=m_rows.end();
            it--;

            Row& row=*it;
            if(!row.isFetchMore()){
                Row newRow;
                newRow.id=row.id+1;
                newRow.fetching=true;
                m_rows.insert(newRow.id, newRow);
                emit rowAdded(newRow.id);

                FetchRequest req;
                req.minId=row.id;
                req.maxId=0;
                req.requestId=newRow.id;
                m_prefetchId=req.requestId;
                requestFetch(req);
            }else if(!row.fetching){
                // this is "Fetch More", but now fetching.
                // use this for prefetch.
                row.fetching=true;
                emit fetchRowUpdated(row.id);

                FetchRequest req;
                req.minId=row.id;
                req.maxId=0;
                req.requestId=row.id;
                m_prefetchId=req.requestId;
                requestFetch(req);
            }
        }
    }
}

void STTimeline::streamStopped(QString msg){
    // TODO: error reporting
    // TODO: maybe starting streaming restarting timer?
    //       or streamer automatically restarts??
    qDebug()<<"STTimeline ["<<m_identifier<<"] streamStopped: "<<msg;
}

void STTimeline::streamFailedToStart(QString msg){
    // maybe nothing to do if streamer automatically restarts?
    qDebug()<<"STTimeline ["<<m_identifier<<"] streamFailedToStart: "<<msg;
    m_account->reportError(msg);
}

void STTimeline::fetchCompleted(const QList<STObjectManager::StatusRef> &statuses, quint64 requestId){
    //  remove "Fetch More" button

    qDebug()<<statuses.length()<<" status fetched";

    if(!row(requestId)){
        qDebug()<<"STTimeline::fetchCompleted: request done, but row" <<requestId<<" doesn't exist";
    }

    bool addFutureFetchMore=false;
    bool topFetchMore=false;
    if(row(requestId) && row(requestId)->isFetchMore()){
        if(m_rows.find(requestId)+1==m_rows.end()){
            // last "Fetch More"
            // it means maxId=inf
            // we have to add "Fetch More" button for future
            topFetchMore=true;
            if(!streamingSupported()){
                bool useCurrentButton=false;
                if(statuses.isEmpty())
                    useCurrentButton=true;
                else if(statuses.count()==1){
                    if(row(statuses[0]->id)){
                        useCurrentButton=true;
                    }
                }
                if(useCurrentButton){
                    // just keep use the current button
                    row(requestId)->fetching=false;
                    emit fetchRowUpdated(requestId);
                    return;
                }

                addFutureFetchMore=true;



            }
        }

        m_rows.remove(requestId);
        emit rowRemoved(requestId);
    }


    if(statuses.empty()){
        if(m_prefetching && m_prefetchId==requestId){
            // streaming prefetch done!
            prefetchFinalize();
        }
        return;
    }

    quint64 minId=statuses[0]->id;
    foreach(STObjectManager::StatusRef status, statuses){
        minId=qMin(minId, status->id);
    }

    bool haveMore=false;
    if(!row(minId)){
        haveMore=true;
    }

    bool bulk=m_shouldBulkLoad;

    for(int i=statuses.size()-1;i>=0;i--){
        // traverse in inverse order in top fetch
        int j=i;
        if((!topFetchMore) || bulk){
            j=statuses.size()-1-i;
        }
        STObjectManager::StatusRef status=statuses[j];
        quint64 id=status->id;
        if(row(id)){
            // already there!
            continue;
        }

        if(!insertingStatus(status))
            continue;

        Row newRow;
        newRow.id=id;
        newRow.status=status;

        m_rows.insert(id, newRow);

        if(m_shouldBulkLoad){
            emit initateBulkLoad();
            m_shouldBulkLoad=false;
        }

        emit rowAdded(id);
        purgeRows();
    }

    if(m_prefetching && m_prefetchId==requestId){
        // streaming prefetch done!
        prefetchFinalize();
    }

    if(haveMore){
        // may not be completely fetched because of too many tweets.
        // add fetch button.

        minId--;

        if(row(minId)){
            // it is not the case.

        }else{

            Row newRow;
            newRow.id=minId;
            newRow.fetching=false;
            m_rows.insert(newRow.id, newRow);
            emit rowAdded(newRow.id);
            purgeRows();

        }
    }

    if(addFutureFetchMore){
        quint64 maxId=statuses[0]->id;
        foreach(STObjectManager::StatusRef status, statuses){
            maxId=qMax(maxId, status->id);
        }

        maxId++;
        Q_ASSERT(!row(maxId));

        Row newRow;
        newRow.id=maxId;
        newRow.fetching=false;
        m_rows.insert(newRow.id, newRow);
        emit rowAdded(newRow.id);
        purgeRows();
    }

}

void STTimeline::fetchFailed(QString ms, quint64 reqId){
    QMap<quint64, Row>::iterator it=m_rows.find(reqId);
    if(it==m_rows.end()){
        qDebug()<<"STTimeline::fetchFailed: row "<<reqId<<" doesn't exist";
    }else{
        Row& row=it.value();
        if(!row.fetching){
            qDebug()<<"STTimeline::fetchFailed: row "<<reqId<< "isn't fetching??";
        }
        row.fetching=false;
        emit fetchRowUpdated(reqId);
    }

    m_account->reportError(ms);

    qWarning()<<"fetchFailed: "<<ms;
    if(reqId==m_prefetchId && m_prefetching){
        prefetchFinalize();
    }
}

void STTimeline::prefetchFinalize(){
    Q_ASSERT(m_prefetching);
    m_prefetching=false;

    bool bulk=m_shouldBulkLoad;
    for(int i=0;i<m_prefetchRows.count();i++){
        int j=i;
        if(bulk)
            j=m_prefetchRows.count()-1-j;
        STObjectManager::StatusRef status=m_prefetchRows[j];
        quint64 id=status->id;
        if(row(id)){
            // already there!
            continue;
        }

        if(!insertingStatus(status))
            continue;

        Row newRow;
        newRow.id=id;
        newRow.status=status;

        m_rows.insert(id, newRow);

        if(m_shouldBulkLoad){
            emit initateBulkLoad();
            m_shouldBulkLoad=false;
        }

        emit rowAdded(id);
        purgeRows();
    }


}

void STTimeline::requestFetch(const FetchRequest &req){
    Q_ASSERT(fetchSupported());
    m_fetcher->metaObject()->invokeMethod(m_fetcher, "requestFetch",
                                          Q_ARG(STTimeline::FetchRequest, req),
                                          Q_ARG(QUrl, fetchEndpoint()),
                                          Q_ARG(STAccount *, m_account));

}

void STTimeline::fetchAbort(quint64 id){
    m_fetcher->metaObject()->invokeMethod(m_fetcher, "abortFetch",
                                          Q_ARG(quint64, id));
}

void STTimeline::purgeRows(){
    // TODO: variable max rows
    while(m_rows.size()>1000){
        QMap<quint64, Row>::iterator it;
        it=m_rows.begin();

        Row& row=*it;
        quint64 id=row.id;
        if(row.isFetchMore()){
            fetchAbort(row.id);
        }else{
            deletingStatus(row.status);
        }

        m_rows.erase(it);

        emit rowRemoved(id);
    }
}

STTimeline::Row *STTimeline::row(quint64 id){
    QMap<quint64, Row>::iterator it=m_rows.find(id);
    if(it==m_rows.end())return NULL;
    return &(it.value());
}


void STTimeline::fetchMore(quint64 id){
    QMap<quint64, Row>::iterator it=m_rows.find(id);
    if(it==m_rows.end()){
        qDebug()<<"STTimeline::fetchMore: row "<<id<<" doesn't exist";
        return;
    }

    Row *row=&(it.value());
    if(!row->isFetchMore()){
        qDebug()<<"STTimeline::fetchMore: row "<<id<<" isn't fetch more";
        return;
    }
    if(row->fetching){
        qDebug()<<"STTimeline::fetchMore: row "<<id<<" is already fetching";
        return;
    }

    FetchRequest req;
    req.requestId=id;

    if(it==m_rows.begin()){
        req.minId=0;
    }else{
        QMap<quint64, Row>::iterator it2=it;
        it2--;
        Q_ASSERT(!it2.value().isFetchMore());
        req.minId=it2.value().id;
    }

    QMap<quint64, Row>::iterator it2=it;
    it2++;
    if(it2==m_rows.end()){
        req.maxId=0;
    }else{
        Q_ASSERT(!it2.value().isFetchMore());
        req.maxId=it2.value().id-1;
    }

    row->fetching=true;
    requestFetch(req);

    emit fetchRowUpdated(id);
}

QList<quint64> STTimeline::allRowIds(){
    return m_rows.keys();
}

bool STTimeline::loadFromSTONVariant(QVariant var){
    QVariantMap map=var.toMap();
    if(!map.contains("account")){
        qWarning()<<"STTimeline::loadFromSTONVariant: no acount info";
        return false;
    }

    QVariantMap accountInfo=map["account"].toMap();
    if(!accountInfo.contains("user_id")){
        qWarning()<<"STTimeline::loadFromSTONVariant: no user_id";
        return false;
    }

    quint64 userId=accountInfo["user_id"].toULongLong();
    if(userId!=m_account->userId()){
        qWarning()<<"STTimeline::loadFromSTONVariant: user_id mismatch ("<<userId<<" read, "<<m_account->userId()<<" expected)";
        return false;
    }

    QString identifier=map["identifier"].toString();
    if(identifier!=m_identifier){
        qWarning()<<"STTimeline::loadFromSTONVariant: identifier mismatch ("<<identifier<<" read, "<<m_identifier<<" expected)";
        return false;
    }

    QVariantList rows=map["rows"].toList();
    if(rows.count()%2){
        qWarning()<<"STTimeline::loadFromSTONVariant: rows count strange";
        return false;
    }

    m_rows.clear();
    m_containingStatuses.clear();
    m_shouldBulkLoad=true;
    for(int i=0;i<rows.count();i+=2){
        quint64 id=rows[i].toULongLong();
        STObjectManager::StatusRef status=rows[i+1].value<STObjectManager::StatusRef>();
        if(!status || insertingStatus(status)){
            Row r;
            r.fetching=false;
            r.id=id;
            r.status=status;
            if(r.status)
                m_shouldBulkLoad=false;
            m_rows.insert(id, r);
        }
    }

    qDebug()<<"STTimeline::loadFromSTONVariant: "<<m_rows.count()<<" rows loaded for "<<m_identifier;

    return true;
}

QVariant STTimeline::toSTONVariant(){
    QVariantMap map;
    QVariantMap accInfo;
    accInfo.insert("user_id", m_account->userId());
    map.insert("account", accInfo);
    map.insert("identifier", m_identifier);

    QVariantList lst;
    for(QMap<quint64, Row>::iterator it=m_rows.begin();
        it!=m_rows.end();it++){
        Row& row=it.value();
        lst.append(row.id);
        if(row.status)
            lst.append(row.status->toRefVariant());
        else
            lst.append(QVariant());
    }

    map.insert("rows", lst);

    return map;
}
