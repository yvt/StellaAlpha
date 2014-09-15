#ifndef STMENTIONSTIMELINE_H
#define STMENTIONSTIMELINE_H


#include "sttimeline.h"

class STUserStreamer;
class STMentionsTimeline : public STTimeline
{
    Q_OBJECT
    STUserStreamer *m_streamer;
public:
    explicit STMentionsTimeline(QString idt, QObject *parent = 0);

    virtual bool streamingSupported() {return true;}
    virtual bool stoppingStreamingSupported(){return false;}

protected:
    virtual void startStream();
    virtual void stopStream();
signals:

public slots:

};

#endif // STMENTIONSTIMELINE_H
