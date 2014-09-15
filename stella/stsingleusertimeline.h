#ifndef STSINGLEUSERTIMELINE_H
#define STSINGLEUSERTIMELINE_H

#include "stsimplestreamingtimeline.h"
#include "stfilteredstreamer.h"

class STUserStreamer;
class STFilteredStreamer;

class STSingleUserTimeline : public STTimeline
{
    Q_OBJECT
    STUserStreamer *m_userStreamer;
    STFilteredStreamer *m_filteredStreamer;
    STFilteredStreamer::FilterHandle m_filterHandle;
    quint64 m_targetUserId;
public:
    explicit STSingleUserTimeline(QString identifier, quint64 targetUserId, QObject *parent = 0);
    
    virtual bool streamingSupported() {return m_userStreamer!=NULL;}
    virtual bool stoppingStreamingSupported(){return false;}

protected:
    virtual void startStream();
    virtual void stopStream();
signals:
    
private slots:
    void userStreamReceived(STObjectManager::StatusRef);
    void filteredStreamReceived(STObjectManager::StatusRef);
    void filteredStreamStarted();
    void filteredStreamStopped(QString);
    void filteredStreamFailedToStart(QString);
    void filterAdded(STFilteredStreamer::Filter);
};

#endif // STSINGLEUSERTIMELINE_H
