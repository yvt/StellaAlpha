#ifndef STFAVORITETIMELINE_H
#define STFAVORITETIMELINE_H

#include "sttimeline.h"

class STUserStreamer;

class STFavoriteTimeline : public STTimeline
{
    Q_OBJECT

    STUserStreamer *m_streamer;
    quint64 m_targetUserId;

public:
    explicit STFavoriteTimeline(QString identifier, quint64 targetUserId, QObject *parent = 0);
    
    virtual bool streamingSupported();
    virtual bool stoppingStreamingSupported(){return false;}

protected:
    virtual void startStream();;
    virtual void stopStream();
signals:
    
private slots:
    void favoritedStatus(STObjectManager::StatusRef);
    void unfavoritedStatus(STObjectManager::StatusRef);
public slots:
    
};

#endif // STFAVORITETIMELINE_H
