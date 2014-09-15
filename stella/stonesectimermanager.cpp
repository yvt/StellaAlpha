#include "stonesectimermanager.h"
#include <QTimer>
#include <QDateTime>

static int g_activeCount=0;
static QTimer *g_timer=NULL;

STOneSecTimerManager::STOneSecTimerManager(QObject *parent) :
    QObject(parent)
{
}

STOneSecTimerManager *STOneSecTimerManager::sharedManager(){
    static STOneSecTimerManager *manager=NULL;
    if(!manager){
        manager=new STOneSecTimerManager();
    }
    return manager;
}

static int timeToNextTick(){
    quint64 tm=(quint64)QDateTime::currentMSecsSinceEpoch();
    int msec=(int)(tm%1000ULL);
    msec=1000-msec;
    while(msec<50){
        msec+=1000;
    }
    return msec;
}

void STOneSecTimerManager::startOneSecTimer(){
    if(!g_timer){
        g_timer=new QTimer(this);
        connect(g_timer, SIGNAL(timeout()),
                this, SLOT(oneSecTimeout()));
    }

    Q_ASSERT(g_timer);
    g_timer->setInterval(timeToNextTick());
    g_timer->setSingleShot(true);
    g_timer->start();
}

void STOneSecTimerManager::stopOneSecTimer(){
    Q_ASSERT(g_timer);
    g_timer->stop();
}

void STOneSecTimerManager::timerActivated(){
    if(g_activeCount==0){
        startOneSecTimer();
    }
    g_activeCount++;
}

void STOneSecTimerManager::timerDeactivated(){
    Q_ASSERT(g_activeCount>0);
    g_activeCount--;
    if(g_activeCount==0){
        stopOneSecTimer();
    }
}

void STOneSecTimerManager::oneSecTimeout(){
    emit timeout();

    Q_ASSERT(g_timer);
    g_timer->setInterval(timeToNextTick());
    g_timer->setSingleShot(true);
    g_timer->start();
}
