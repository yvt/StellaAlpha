#include "stsimplestreamingtimeline.h"
#include <QStringList>
#include "stsimplestreamer.h"
#include "staccountsmanager.h"
#include <QDebug>

STSimpleStreamingTimeline::STSimpleStreamingTimeline(QString identifier,STSimpleStreamer *streamer,QObject *parent) :
    STTimeline(identifier, parent)
{
    QStringList lst=identifier.split(".");
    quint64 uid=lst[0].toULongLong();

    STAccount *ac=STAccountsManager::sharedManager()->account(uid);
    Q_ASSERT(ac);

    m_streamer=streamer;
    connect(m_streamer, SIGNAL(streamingStarted()),
            this, SLOT(streamStarted()));
    connect(m_streamer, SIGNAL(streamingStopped(QString)),
            this, SLOT(streamStopped(QString)));
    connect(m_streamer, SIGNAL(streamingCouldNotStarted(QString)),
            this, SLOT(streamFailedToStart(QString)));
    connect(m_streamer, SIGNAL(statusReceived(STObjectManager::StatusRef)),
            this, SLOT(streamReceived(STObjectManager::StatusRef)));
    connect(m_streamer, SIGNAL(statusRemoved(quint64)),
            this, SLOT(streamRemoved(quint64)));
}

STSimpleStreamingTimeline::~STSimpleStreamingTimeline(){
    m_streamer->deleteLater();
}



void STSimpleStreamingTimeline::received(STObjectManager::StatusRef ){
   /* STObjectManager::StatusRefList lst;
    lst.append(ref);
    this->streamReceived(lst);*/
}

bool STSimpleStreamingTimeline::streamingSupported(){
    return true;
}

void STSimpleStreamingTimeline::startStream(){
    m_streamer->metaObject()->invokeMethod(m_streamer,
                                           "startStreaming", Qt::BlockingQueuedConnection);
}

void STSimpleStreamingTimeline::stopStream(){
    m_streamer->metaObject()->invokeMethod(m_streamer,
                                           "stopStreaming", Qt::BlockingQueuedConnection);
}

