#include "stcaretview.h"
#include <QPainter>
#include <math.h>
#include <QDateTime>

STCaretView::STCaretView(QGraphicsItem *parent) :
    QGraphicsObject(parent), m_size(0.f,0.f)
{
    setOpacity(0.f);
    this->setAcceptedMouseButtons(Qt::NoButton);
    this->setCacheMode(DeviceCoordinateCache);

    m_fadeDuration=200;
    m_isCaretVisible=false;
    m_fadeStartTime=0;
    m_newFadeOpacity=0.f;
    m_oldFadeOpacity=0.f;
    m_timer=0;
}

bool STCaretView::shouldAnimate(){
    if(m_isCaretVisible)
        return true;
    return (QDateTime::currentMSecsSinceEpoch()-m_fadeStartTime)<m_fadeDuration;
}

float STCaretView::fadeOpacity(){
    float per=(float)(QDateTime::currentMSecsSinceEpoch()-m_fadeStartTime)/(float)m_fadeDuration;
    if(per<0.f)per=0.f;
    if(per>1.f)per=1.f;
    return m_oldFadeOpacity+(m_newFadeOpacity-m_oldFadeOpacity)*per;
}

void STCaretView::setSize(const QSizeF &size){
    if(size==m_size)return;
    prepareGeometryChange();
    m_size=size;
}

void STCaretView::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    QColor caretColor(220, 128, 40);
    painter->fillRect(boundingRect(), caretColor);
}

QRectF STCaretView::boundingRect() const{
    return QRectF(QPointF(0.f, 0.f), m_size);
}

void STCaretView::showCaret(){
    blankCaret();
    if(m_isCaretVisible) return;
    m_oldFadeOpacity=fadeOpacity();
    m_newFadeOpacity=1.f;
    m_fadeStartTime=QDateTime::currentMSecsSinceEpoch();
    m_isCaretVisible=true;
    startAnimate();;
}

void STCaretView::hideCaret(){
    if(!m_isCaretVisible) return;
    m_oldFadeOpacity=fadeOpacity();
    m_newFadeOpacity=0.f;
    m_fadeStartTime=QDateTime::currentMSecsSinceEpoch();
    m_isCaretVisible=false;
    startAnimate();;
}

void STCaretView::blankCaret(){
    m_blankStartTime=QDateTime::currentMSecsSinceEpoch();
}

float STCaretView::blankOpacity(){
    unsigned int dur=(unsigned int)((QDateTime::currentMSecsSinceEpoch()-m_blankStartTime)&1023);
    return 0.5f+0.5f*cosf((float)dur*M_PI*2.f/1024.f);
}

void STCaretView::timerEvent(QTimerEvent *){
    float fadeOp=fadeOpacity();
    float blankOp=blankOpacity();

    setOpacity(fadeOp*blankOp);

    if(!shouldAnimate()){
        this->killTimer(m_timer);
        m_timer=0;
    }
}

void STCaretView::startAnimate(){
    if(m_timer){
        return;
    }
    m_timer=this->startTimer(50);
}

