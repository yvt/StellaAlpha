#include "stdynamicanimator.h"
#include "stmath.h"

void STDynamicAnimator::setPos(double pos){
    m_startTime=0;
    m_duration=0;
    m_newPos=pos;
    m_hasFuturePos=false;
}

void STDynamicAnimator::setPosAnimated(double newPos, unsigned int duration, quint64 currentTime){

    if(currentTime==1){
        m_futurePos=newPos;
        m_hasFuturePos=true;
        m_futureDuration=duration;
        return;
    }else if(m_hasFuturePos){
        m_hasFuturePos=false;
    }

    Q_ASSERT(currentTime!=1);
    Q_ASSERT(m_startTime!=1);

    if(newPos==m_newPos)
        return;
    if(fabs(newPos-m_newPos)<1.e-4){
        m_newPos=newPos;
        return;
    }

    double curPos=currentPos(currentTime);
    double curVel=currentVelocity(currentTime);

    if(curPos==newPos){
        setPos(newPos);
        return;
    }

    curVel*=(double)duration/(newPos-curPos);

    if(curVel>1. && curVel<2.){
        unsigned int newDuration;
        newDuration=(unsigned int)((double)duration/curVel);
        curVel*=(double)newDuration/(double)duration;
        duration=newDuration;
    }

    m_startVel=curVel;
    m_oldPos=curPos;
    m_newPos=newPos;
    m_duration=duration;
    m_startTime=currentTime;
}

double STDynamicAnimator::currentPos(quint64 currentTime) {
    Q_ASSERT(currentTime!=1);
    if(m_hasFuturePos){
        m_hasFuturePos=false;
        setPosAnimated(m_futurePos, m_futureDuration, currentTime);
    }
    if(currentTime>=m_startTime+m_duration)
        return m_newPos;
    double per=(double)(currentTime-m_startTime)/(double)m_duration;
    if(per>=1.)
        return m_newPos;
    if(per<=0.)
        return m_oldPos;
    per=STForcedSmoothStep(per, m_startVel);

    return m_oldPos+(m_newPos-m_oldPos)*per;
}

double STDynamicAnimator::currentVelocity(quint64 currentTime) {
    Q_ASSERT(currentTime!=1);
    if(m_hasFuturePos){
        m_hasFuturePos=false;
        setPosAnimated(m_futurePos, m_futureDuration, currentTime);
    }
    if(currentTime>=m_startTime+m_duration)
        return 0.;
    double per=(double)(currentTime-m_startTime)/(double)m_duration;
    if(per>=1.)
        return 0.;
    if(per<=0.)
        return (m_newPos-m_oldPos)*m_startVel/(double)m_duration;
    per=STForcedSmoothStepVelocity(per, m_startVel);

    return (m_newPos-m_oldPos)*per/(double)m_duration;
}

bool STDynamicAnimator::isAnimating(quint64 currentTime) const{
    if(m_hasFuturePos)
        return true;
    return currentTime<m_startTime+m_duration;
}

