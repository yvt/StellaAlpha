#include "stonesectimer.h"
#include "stonesectimermanager.h"

STOneSecTimer::STOneSecTimer(QObject *parent) :
    QObject(parent),m_active(false)
{
    connect(STOneSecTimerManager::sharedManager(),
            SIGNAL(timeout()),
            this,
            SLOT(timeoutReceived()));
}

STOneSecTimer::~STOneSecTimer(){
    if(m_active){
        STOneSecTimerManager::sharedManager()->timerDeactivated();
    }
}

void STOneSecTimer::setActive(bool act){
    if(m_active==act)
        return;
    if(act){
        STOneSecTimerManager::sharedManager()->timerActivated();
    }else{
        STOneSecTimerManager::sharedManager()->timerDeactivated();
    }
    m_active=act;
}

void STOneSecTimer::timeoutReceived(){
    if(!m_active)
        return;
    emit timeout();
}
