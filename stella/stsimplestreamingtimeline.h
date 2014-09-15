#ifndef STSIMPLESTREAMINGTIMELINE_H
#define STSIMPLESTREAMINGTIMELINE_H

#include "sttimeline.h"
#include "stobjectmanager.h"
#include "stsimplestreamer.h"

class STSimpleStreamer;

class STSimpleStreamingTimeline : public STTimeline
{
    Q_OBJECT
    STSimpleStreamer *m_streamer;
public:
    explicit STSimpleStreamingTimeline(QString idt, STSimpleStreamer *streamer, QObject *parent = 0);
    virtual ~STSimpleStreamingTimeline();

    virtual bool streamingSupported();
protected:
    virtual void startStream();
    virtual void stopStream();
signals:
    
private slots:
    void received(STObjectManager::StatusRef);
};

#endif // STSIMPLESTREAMINGTIMELINE_H
