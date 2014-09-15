#include "stsimpleanimator.h"
#include "stmath.h"

STSimpleAnimator::STSimpleAnimator(QObject *parent) :
    QObject(parent)
{
    m_animTimer=0;
}

static qint64 currentTime() {
    return QDateTime::currentMSecsSinceEpoch();
}

void STSimpleAnimator::setValue(float val){
    if(!this)return;
    m_oldValue=m_newValue=val;
    m_startTime=0;
    m_duration=1;
    emit valueChanged(val);
}

void STSimpleAnimator::setValueAnimated(float newVal, int duration){
     if(!this)return;
    if(newVal==m_newValue)
        return;
    if(duration==0){
        setValue(newVal);
        return;
    }
    m_oldValue=value();
    m_newValue=newVal;
    m_startTime=currentTime();
    m_duration=duration;
    startAnimation();
}

void STSimpleAnimator::startAnimation(){
     if(!this)return;
    if(!isAnimating())
        return;
    if(m_animTimer)
        return;
    m_animTimer=startTimer(15);
}

float STSimpleAnimator::value() const{
     if(!this)return 0.;
    float per=(float)(currentTime()-m_startTime)/(float)m_duration;
    if(per<0.f)per=0.f;
    if(per>1.f)per=1.f;
    per=STSmoothStep(per);
    return m_oldValue*(1.f-per)+m_newValue*per;
}

bool STSimpleAnimator::isAnimating() const{
     if(!this)return false;
    return currentTime()<(m_startTime+m_duration);
}

void STSimpleAnimator::timerEvent(QTimerEvent *){
    bool ani=isAnimating();
    emit valueChanged(value());
    if(!ani){
        killTimer(m_animTimer);
        m_animTimer=0;
    }
}
