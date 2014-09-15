#include "stfavoritetimeline.h"
#include "staccount.h"
#include "staccountsmanager.h"
#include "stuserstreamer.h"

STFavoriteTimeline::STFavoriteTimeline(QString identifier, quint64 targetUserId, QObject *parent) :
    STTimeline(identifier, parent), m_targetUserId(targetUserId)
{
    QString urlStr="https://api.twitter.com/1.1/favorites/list.json?user_id=%1";
    setFetchEndpoint(QUrl(urlStr.arg(targetUserId)));

    STAccount *account=accountForIdentifier(identifier);
    if(account->userId()==targetUserId){
        m_streamer=account->userStreamer();
        connect(m_streamer, SIGNAL(streamingStarted()),
                this, SLOT(streamStarted()));
        connect(m_streamer, SIGNAL(streamingStopped(QString)),
                this, SLOT(streamStopped(QString)));
        connect(m_streamer, SIGNAL(streamingCouldNotStarted(QString)),
                this, SLOT(streamFailedToStart(QString)));
        connect(m_streamer, SIGNAL(statusRemoved(quint64)),
                this, SLOT(streamRemoved(quint64)));
        connect(m_streamer, SIGNAL(accountFavoritedStatus(STObjectManager::StatusRef,STObjectManager::UserRef)),
                this, SLOT(favoritedStatus(STObjectManager::StatusRef)));
        connect(m_streamer, SIGNAL(accountUnfavoritedStatus(STObjectManager::StatusRef,STObjectManager::UserRef)),
                this, SLOT(unfavoritedStatus(STObjectManager::StatusRef)));
    }else{
        m_streamer=NULL;
    }
}

bool STFavoriteTimeline::streamingSupported(){
    return m_streamer!=NULL;
}

void STFavoriteTimeline::startStream(){
    Q_ASSERT(m_streamer);

    if(m_streamer->state()==STStreamer::Streaming){
        streamStarted();
    }
}

void STFavoriteTimeline::stopStream(){

}

void STFavoriteTimeline::favoritedStatus(STObjectManager::StatusRef status){
    streamReceived(status);
}

void STFavoriteTimeline::unfavoritedStatus(STObjectManager::StatusRef status){
    streamRemoved(status->id);
}
