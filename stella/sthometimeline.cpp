#include "sthometimeline.h"
#include "staccount.h"
#include "stuserstreamer.h"
#include <QDebug>
#include <QStringList>
#include "staccountsmanager.h"

STHomeTimeline::STHomeTimeline(QString identifier, QObject *parent) :
    STTimeline(identifier, parent)
{
    QStringList lst=identifier.split(".");
    quint64 uid=lst[0].toULongLong();

    this->setFetchEndpoint(QUrl("https://api.twitter.com/1.1/statuses/home_timeline.json?exclude_replies=false&include_my_retweet=1"));

    STAccount *ac=STAccountsManager::sharedManager()->account(uid);
    Q_ASSERT(ac);

    m_streamer=ac->userStreamer();
    connect(m_streamer, SIGNAL(streamingStarted()),
            this, SLOT(streamStarted()));
    connect(m_streamer, SIGNAL(streamingStopped(QString)),
            this, SLOT(streamStopped(QString)));
    connect(m_streamer, SIGNAL(streamingCouldNotStarted(QString)),
            this, SLOT(streamFailedToStart(QString)));
    connect(m_streamer, SIGNAL(homeNewStatusReceived(STObjectManager::StatusRef)),
            this, SLOT(streamReceived(STObjectManager::StatusRef)));
    connect(m_streamer, SIGNAL(statusRemoved(quint64)),
            this, SLOT(streamRemoved(quint64)));
}

void STHomeTimeline::startStream(){
    if(m_streamer->state()==STStreamer::Streaming){
        streamStarted();
    }
}

void STHomeTimeline::stopStream(){
    qWarning()<<"stopping home timeline not supported";
}

