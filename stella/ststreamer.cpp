#include "ststreamer.h"
#include "staccount.h"
#include <QtKOAuth>
#include <QThread>
#include <QDateTime>
#include <QTimerEvent>
#include <QTimer>
#include <time.h>

QThread *STStreamer::streamerThread() {
    static QThread *thread=NULL;
    if(!thread){
        thread=new QThread();
        thread->start(QThread::LowPriority);
    }
    return thread;
}

STStreamer::STStreamer(STAccount *acc, QUrl ep) :
    QObject(), m_account(acc), m_endpoint(ep)
{
    this->moveToThread(streamerThread());
    m_streamer=NULL;

    qsrand(time(0)+(int)QDateTime::currentMSecsSinceEpoch());
    m_reconnectTimer=0;
    m_reconnectDelay=1000;
    m_connectionTimeout=5000;
    m_dataLineTimeout=60000;
    m_streamingRequested=false;
    m_changingParameter=false;

    m_timeoutTimer=NULL;
}



STStreamer::State STStreamer::state() const{
    if(m_reconnectTimer)
        return Reconnecting;
    if(!m_streamer)
        return NotConnected;

    switch(m_streamer->state()){
    case KQOAuthStreamer::NotConnected:
        return NotConnected;
    case KQOAuthStreamer::Connecting:
        return Connecting;
    case KQOAuthStreamer::Streaming:
        return Streaming;
    }
    Q_ASSERT(false);
    return NotConnected;
}

void STStreamer::setEndpoint(QUrl url){

    if(QThread::currentThread()!=this->thread()){
        this->metaObject()->invokeMethod(this, "setEndpoint",
                                         Qt::QueuedConnection,
                                         Q_ARG(QUrl, url));
        return;
    }
/*
    if(url==m_endpoint)
        return;*/
    m_endpoint=url;

    if(!m_streamer)
        return;

    m_changingParameter=true;

    KQOAuthParameters params;
    typedef QPair<QString, QString> StringPair;
    foreach(StringPair pair, url.queryItems()){
        params.insert(pair.first, pair.second);
    }
    url.setQueryItems(QList<QPair<QString, QString> >());
    m_streamer->setUrl(url);
    m_streamer->setAdditionalParameters(params);

    if(state()==Connecting || state()==Streaming){
        stopStreaming();
        startStreaming();
    }else if(state()==Reconnecting){
        if(m_reconnectTimer){
            killTimer(m_reconnectTimer);
        }
        startStreaming();
    }

    m_changingParameter=false;
}

void STStreamer::startStreaming(){
    if(QThread::currentThread()!=this->thread()){
        this->metaObject()->invokeMethod(this, "startStreaming",
                                         Qt::QueuedConnection);
        return;
    }
    if(state()!=NotConnected)
        return;

    if(!m_streamer){
        m_streamer=new KQOAuthStreamer(this);
        QUrl url=m_endpoint;
        KQOAuthParameters params;
        typedef QPair<QString, QString> StringPair;
        foreach(StringPair pair, url.queryItems()){
            params.insert(pair.first, pair.second);
        }
        url.setQueryItems(QList<QPair<QString, QString> >());
        m_streamer->setUrl(url);
        m_streamer->setAdditionalParameters(params);
        m_streamer->setHttpMethod("GET");

        connect(m_streamer, SIGNAL(streamingStarted()),
                this, SLOT(connected()));
        connect(m_streamer, SIGNAL(streamingStopped()),
                this, SLOT(disconnected()));
        connect(m_streamer, SIGNAL(streamingErrorStarting()),
                this, SLOT(connectionError()));
        connect(m_streamer, SIGNAL(streamingLineReceived(QByteArray)),
                this, SLOT(resetTimeoutTimer()));
        connect(m_streamer, SIGNAL(streamingLineReceived(QByteArray)),
                this, SLOT(lineReceived(QByteArray)));

    }

    m_streamer->setConsumerKey(m_account->consumerKey());
    m_streamer->setConsumerSecretKey(m_account->consumerSecret());
    m_streamer->setToken(m_account->accessToken());
    m_streamer->setTokenSecret(m_account->accessTokenSecret());



    if(!m_timeoutTimer){
        m_timeoutTimer=new QTimer(this);
        m_timeoutTimer->setSingleShot(true);
        connect(m_timeoutTimer,SIGNAL(timeout()),this, SLOT(timeoutTimerTimeout()));
    }

    m_timeoutTimer->setInterval(m_connectionTimeout);
    m_timeoutTimer->start();

    m_streamingRequested=true;
    qDebug()<<"starting streaming to "<<m_endpoint<<"...";
    m_streamer->startStreaming();
}

void STStreamer::stopStreaming(){
    if(m_reconnectTimer){
        killTimer(m_reconnectTimer);
    }
    if(m_timeoutTimer)
        m_timeoutTimer->stop();
    if(QThread::currentThread()!=this->thread()){
        this->metaObject()->invokeMethod(this, "stopStreaming",
                                         Qt::QueuedConnection);
        return;
    }
    m_streamingRequested=false;

    m_streamer->stopStreaming();
}


void STStreamer::resetTimeoutTimer(){
    if(m_timeoutTimer){
        m_timeoutTimer->stop();
        m_timeoutTimer->setInterval(m_dataLineTimeout);
        m_timeoutTimer->start();
    }
}
/*
void STStreamer::lineReceived(const QByteArray &data){

}*/

void STStreamer::connected(){
    m_streaming=true;
    m_reconnectDelay=1000;
    Q_ASSERT(m_timeoutTimer);
    m_timeoutTimer->setInterval(m_dataLineTimeout);
    m_timeoutTimer->start();
    emit streamingStarted();
}

void STStreamer::connectionError(){
    if(m_streaming){
        emit streamingStopped(tr("Connection error occured."));
    }else{
        emit streamingCouldNotStarted(tr("Connection error occured."));
    }
    Q_ASSERT(m_timeoutTimer);
    m_timeoutTimer->stop();

    if(m_streamingRequested){
        m_reconnectTimer=startTimer(m_reconnectDelay);

        m_reconnectDelay*=2;
        if(m_reconnectDelay>60000)
            m_reconnectDelay=60000;
    }
}

void STStreamer::disconnected(){
    if(m_reconnectTimer) // may be called through timeoutTimerTimeout
        return;
    m_streaming=false;
    if(m_changingParameter)
        emit streamingStopped(tr("Parameter updated."));
    else if(!m_streamingRequested)
        emit streamingStopped(tr("Disconnected."));
    else
        emit streamingStopped(tr("Connection closed by host."));
    Q_ASSERT(m_timeoutTimer);
    m_timeoutTimer->stop();

    if(m_streamingRequested && !m_reconnectTimer){
        m_reconnectTimer=startTimer(m_reconnectDelay);

        m_reconnectDelay*=2;
        if(m_reconnectDelay>60000)
            m_reconnectDelay=60000;
    }
}

void STStreamer::timerEvent(QTimerEvent *ev){
    this->killTimer(ev->timerId());
    m_reconnectTimer=0;

    // try reconnect
    startStreaming();
}

void STStreamer::timeoutTimerTimeout(){
    this->blockSignals(true);
    m_streamer->stopStreaming();
    this->blockSignals(false);
    m_streaming=false;
    emit streamingStopped("Timeout.");

    if(m_streamingRequested && !m_reconnectTimer){
        m_reconnectTimer=startTimer(m_reconnectDelay);

        m_reconnectDelay*=2;
        if(m_reconnectDelay>60000)
            m_reconnectDelay=60000;
    }
}
