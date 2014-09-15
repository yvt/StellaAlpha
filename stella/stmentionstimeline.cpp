#include "stmentionstimeline.h"
#include "staccount.h"
#include "stuserstreamer.h"
#include <QDebug>
#include <QStringList>
#include "staccountsmanager.h"

STMentionsTimeline::STMentionsTimeline(QString identifier, QObject *parent) :
    STTimeline(identifier, parent)
{
    QStringList lst=identifier.split(".");
    quint64 uid=lst[0].toULongLong();

    this->setFetchEndpoint(QUrl("https://api.twitter.com/1.1/statuses/mentions_timeline.json?include_my_retweet=1"));

    STAccount *ac=STAccountsManager::sharedManager()->account(uid);
    Q_ASSERT(ac);

    m_streamer=ac->userStreamer();
    connect(m_streamer, SIGNAL(streamingStarted()),
            this, SLOT(streamStarted()));
    connect(m_streamer, SIGNAL(streamingStopped(QString)),
            this, SLOT(streamStopped(QString)));
    connect(m_streamer, SIGNAL(streamingCouldNotStarted(QString)),
            this, SLOT(streamFailedToStart(QString)));
    connect(m_streamer, SIGNAL(mentionsNewStatusReceived(STObjectManager::StatusRef)),
            this, SLOT(streamReceived(STObjectManager::StatusRef)));
    connect(m_streamer, SIGNAL(statusRemoved(quint64)),
            this, SLOT(streamRemoved(quint64)));
}

void STMentionsTimeline::startStream(){
    if(m_streamer->state()==STStreamer::Streaming){
        streamStarted();
    }
}

void STMentionsTimeline::stopStream(){
    qWarning()<<"stopping mention timeline not supported";
}

