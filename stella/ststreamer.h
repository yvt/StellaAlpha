#ifndef STSTREAMER_H
#define STSTREAMER_H

#include <QObject>
#include <QUrl>

class STAccount;
class KQOAuthStreamer;
class QTimer;
class STStreamer : public QObject
{
    Q_OBJECT

    STAccount *m_account;
    QUrl m_endpoint;
    KQOAuthStreamer *m_streamer;
    bool m_streaming;
    int m_reconnectTimer;
    int m_reconnectDelay;
    int m_connectionTimeout;
    int m_dataLineTimeout;
    bool m_streamingRequested;
    QTimer *m_timeoutTimer;
    volatile bool m_changingParameter;

public:
    // cannot have parent because
    // streamer runs in the separated thread.
    explicit STStreamer(STAccount *, QUrl);
    static QThread *streamerThread();

    enum State{
        NotConnected,
        Connecting,
        Streaming,
        Reconnecting
    };

    State state() const;

    STAccount *account() const{return m_account;}

    QUrl endpoint() const{return m_endpoint;}


    virtual void timerEvent(QTimerEvent *);

signals:

    void streamingCouldNotStarted(QString);
    void streamingStarted();
    void streamingStopped(QString);
public slots:
    void startStreaming();
    void stopStreaming();
    void setEndpoint(QUrl); // always cause streaming to restart.
    void timeoutTimerTimeout();

protected slots:
    virtual void lineReceived(const QByteArray&)=0;
    void connected();
    void disconnected();
    void connectionError();
    void resetTimeoutTimer();
};

#endif // STSIMPLESTREAMER_H
