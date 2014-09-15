#ifndef STHOMETIMELINE_H
#define STHOMETIMELINE_H

#include "sttimeline.h"

class STUserStreamer;
class STHomeTimeline : public STTimeline
{
    Q_OBJECT
    STUserStreamer *m_streamer;
public:
    explicit STHomeTimeline(QString idt, QObject *parent = 0);
    
    virtual bool streamingSupported() {return true;}
    virtual bool stoppingStreamingSupported(){return false;}

protected:
    virtual void startStream();
    virtual void stopStream();
signals:
    
public slots:
    
};

#endif // STHOMETIMELINE_H
