#include "stviewanimator.h"
#include <QDateTime>
#include "stmath.h"

STViewAnimator::STViewAnimator(QObject *parent) :
    QObject(parent)
{
    m_newView=NULL;
    m_duration=200;
    m_startTime=0;
    m_width=0;
    m_timer=0;
    m_visibleOnlyWhenParentIsVisible=false;
    m_deleteWhenDone=false;
}

bool STViewAnimator::isAnimating(){
    return QDateTime::currentMSecsSinceEpoch()<m_startTime+(quint64)m_duration;
}

float STViewAnimator::progress(){
    float per=(float)(QDateTime::currentMSecsSinceEpoch()-m_startTime)/(float)m_duration;
    if(per<0.f)per=0.f;
    if(per>1.f)per=1.f;
    per=STSmoothStep(per);
    return per;
}

bool STViewAnimator::isViewVisible(QGraphicsItem *item){
    if(m_visibleOnlyWhenParentIsVisible){
        if(!item->parentItem()->isVisible())
            return false;
    }
    if(!isAnimating()){
        return item==m_newView;
    }else{
        return item==m_newView || item==m_oldView;
    }
}

float STViewAnimator::viewXPos(QGraphicsItem *item){
    if(!isAnimating())return 0.f;
    float per=progress();
    if(item==m_newView){
        if(m_opening==1){
            per=1.f-per;
            return roundf(per*(float)m_width);
        }else if(m_opening==-1){
            per=per-1.f;
            return roundf(per*(float)m_width);
        }
    }else if(item==m_oldView){
        if(m_opening==1){
            per=-per;
            return roundf(per*(float)m_width);
        }else if(m_opening==-1){
            return roundf(per*(float)m_width);
        }
    }else{
        return 0.f;
    }
    Q_ASSERT(false);
}

void STViewAnimator::setWidth(int width){
    if(width==m_width)return;
    m_width=width;
    relayout();
}

void STViewAnimator::relayout(bool fromTimer){
    if(m_newView){
        m_newView->setPos(viewXPos(m_newView), m_newView->pos().y());
        m_newView->setVisible(isViewVisible(m_newView));
    }
    if(m_oldView && (fromTimer || isAnimating())){
        m_oldView->setPos(viewXPos(m_oldView), m_oldView->pos().y());
        m_oldView->setVisible(isViewVisible(m_oldView));
    }
}

void STViewAnimator::startAnimation(){
    if(m_timer)return;
    if(!isAnimating())return;
    m_timer=this->startTimer(15);
}

void STViewAnimator::timerEvent(QTimerEvent *){
    relayout(true);
    if(!isAnimating()){
        if(m_deleteWhenDone){
            delete m_oldView;
            m_oldView=NULL;
        }
        killTimer(m_timer);
        m_timer=0;
    }
}

void STViewAnimator::activateView(QGraphicsItem *item){
    abortAnimation();

    if(item==m_newView) return;

    m_newView=item;
    relayout();
}

void STViewAnimator::activateViewAnimated(QGraphicsItem *itm, int direction, int duration,
                                          bool deleteWhenDone){
    if(itm==m_newView) return;
    abortAnimation();

    m_oldView=m_newView;
    m_newView=itm;
    m_opening=direction;
    m_duration=duration;
    m_startTime=QDateTime::currentMSecsSinceEpoch();
    m_deleteWhenDone=deleteWhenDone;
    startAnimation();

    // this makes starting of animation smoother
    timerEvent(NULL);
    m_startTime=QDateTime::currentMSecsSinceEpoch();
}

void STViewAnimator::abortAnimation(){
    if(!isAnimating())
        return;
    m_startTime=0;
    if(m_deleteWhenDone){
        delete m_oldView;
        m_oldView=NULL;
    }
    relayout(true);
    if(m_timer){
        killTimer(m_timer);
        m_timer=0;
    }
    m_deleteWhenDone=false;
    Q_ASSERT(!isAnimating());
}

void STViewAnimator::setVisibleOnlyWhenParentIsVisible(bool b){
    m_visibleOnlyWhenParentIsVisible=b;
    relayout();
}
