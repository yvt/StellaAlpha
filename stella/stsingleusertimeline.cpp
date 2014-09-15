#include "stsingleusertimeline.h"
#include "stsingleuserstreamer.h"
#include "staccount.h"
#include "stuserstreamer.h"

STSingleUserTimeline::STSingleUserTimeline(QString identifier, quint64 targetUserId,QObject *parent) :
    STTimeline(identifier,  parent),
    m_targetUserId(targetUserId)
{
    QString url="https://api.twitter.com/1.1/statuses/user_timeline.json?user_id=%1&exclude_replies=false&include_rts=1&include_my_retweet=1";
    setFetchEndpoint(QUrl(url.arg(targetUserId)));

    if(accountForIdentifier(identifier)->userId()==targetUserId){
        m_userStreamer=accountForIdentifier(identifier)->userStreamer();
        connect(m_userStreamer, SIGNAL(statusReceived(STObjectManager::StatusRef)),
                this, SLOT(userStreamReceived(STObjectManager::StatusRef)));
        connect(m_userStreamer, SIGNAL(statusRemoved(quint64)),
                this, SLOT(streamRemoved(quint64)));
        connect(m_userStreamer, SIGNAL(streamingStarted()),
                this, SLOT(streamStarted()));
        connect(m_userStreamer, SIGNAL(streamingStopped(QString)),
                this, SLOT(streamStopped(QString)));
        connect(m_userStreamer, SIGNAL(streamingCouldNotStarted(QString)),
                this, SLOT(streamFailedToStart(QString)));
    }else{
        m_userStreamer=NULL;
    }



    //m_filteredStreamer=accountForIdentifier(identifier)->filteredStreamer();
    /*
    connect(m_filteredStreamer, SIGNAL(streamingStarted()),
            this, SLOT(filteredStreamStarted()));
    connect(m_filteredStreamer, SIGNAL(streamingStopped(QString)),
            this, SLOT(filteredStreamStopped(QString)));
    connect(m_filteredStreamer, SIGNAL(streamingCouldNotStarted(QString)),
            this, SLOT(filteredStreamFailedToStart(QString)));
    connect(m_filteredStreamer, SIGNAL(statusReceived(STObjectManager::StatusRef)),
            this, SLOT(filteredStreamReceived(STObjectManager::StatusRef)));
    connect(m_filteredStreamer, SIGNAL(statusRemoved(quint64)),
            this, SLOT(streamRemoved(quint64)));
*/
    //m_filterHandle=STFilteredStreamer::FilterHandle(m_filteredStreamer);

}

void STSingleUserTimeline::startStream(){
    Q_ASSERT(m_userStreamer);

    if(m_userStreamer->state()==STStreamer::Streaming){
        streamStarted();
    }

    /*
    STFilteredStreamer::Filter flt;
    flt.type=STFilteredStreamer::User;
    flt.value=QString::number(m_targetUserId);
    m_filterHandle.setFilter(flt);*/
}

void STSingleUserTimeline::stopStream(){
    //m_filterHandle.clear();
}

void STSingleUserTimeline::userStreamReceived(STObjectManager::StatusRef status){
    if(status->user->id!=m_targetUserId)
        return;
    if(!isRunning())
        return;
    this->streamReceived(status);
}


void STSingleUserTimeline::filteredStreamReceived(STObjectManager::StatusRef status){
    if(status->user->id!=m_targetUserId)
        return;
    if(!isRunning())
        return;
    this->streamReceived(status);
}

void STSingleUserTimeline::filteredStreamStarted(){
    if(isRunning())
        this->streamStarted();
}

void STSingleUserTimeline::filterAdded(STFilteredStreamer::Filter ){

}

void STSingleUserTimeline::filteredStreamStopped(QString ){
    //this->streamStopped(msg);
}

void STSingleUserTimeline::filteredStreamFailedToStart(QString ){
    //this->streamFailedToStart(msg);
}
