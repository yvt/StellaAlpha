#include "stnewsview.h"
#include <QImage>
#include <QLinearGradient>
#include <QPainter>
#include <QBrush>
#include "stnewsshadowview.h"
#include <QDateTime>
#include "stmath.h"
#include <QCursor>
#include "stnewslistview.h"
#include "sttweetnoneditingview.h"

STNewsView::STNewsView(QGraphicsItem *parent) :
    QGraphicsObject(parent), m_size(0,0)
{

    m_maxHeight=130;
    m_listView=new STNewsListView(this);
    m_hoverView=new STTweetNonEditingView(this);
    m_shadowView=new STNewsShadowView(this);

    m_listView->scrollBarView()->setOpacity(0);


    m_animStartTime=0;
    m_newOpenState=0.f;
    m_animDuration=200;
    m_animTimer=0;

    setCursor(Qt::ArrowCursor);
    setFlag(ItemIsFocusable);
}

float STNewsView::openState() const{
    float per=(float)(QDateTime::currentMSecsSinceEpoch()-m_animStartTime)/(float)m_animDuration;
    if(per<0.f)per=0.f;
    if(per>1.f)per=1.f;
    per=STSmoothStep(per);
    return m_oldOpenState*(1.f-per)+m_newOpenState*per;
}

void STNewsView::startAnimation(){
    if(!isAnimating())
        return;
    if(m_animTimer)
        return;
    m_animTimer=startTimer(15);
    emit currentHeightChanged();
}

bool STNewsView::isAnimating(){
    return (QDateTime::currentMSecsSinceEpoch()-m_animStartTime)<(qint64)m_animDuration;
}

void STNewsView::setSize(const QSize&s){
    if(s==m_size)return;
    prepareGeometryChange();
    m_size=s;relayout();
}

void STNewsView::relayout(){
    m_shadowView->setPos(0.f, 0.f);
    m_shadowView->setWidth(m_size.width());
    m_listView->setSize(QSize(m_size.width(), m_maxHeight-1));
    m_listView->setPos(0,1);
    m_hoverView->setSize(QSizeF(m_size.width(), 24));
}

void STNewsView::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    painter->fillRect(QRect(0,0,m_size.width(), m_maxHeight),
                      QColor(90,90,90));
}

QRectF STNewsView::boundingRect() const{

    return QRectF(0.f, 0, m_size.width(), m_maxHeight);
}

void STNewsView::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    // this handler is needed to prevent mouse event from passing through this view,
    // causing tweet button to be pressed.

}

int STNewsView::currentHeight() const{
    float v1=(float)m_size.height();
    float v2=m_maxHeight;
    return (int)floorf(v1+(v2-v1)*openState());
}

void STNewsView::timerEvent(QTimerEvent *event){
    m_listView->scrollBarView()->setOpacity(openState());
    emit currentHeightChanged();
    if(m_animTimer && (!isAnimating())){
        killTimer(m_animTimer);
        m_animTimer=0;
    }
}

void STNewsView::focusInEvent(QFocusEvent *event){
    setOpenStateAnimated(1.f);
    m_hoverView->setVisible(false);
}

void STNewsView::focusOutEvent(QFocusEvent *event){
    setOpenStateAnimated(0.f);
    m_hoverView->setVisible(true);
}

void STNewsView::setOpenStateAnimated(float v){
    if(v==m_newOpenState)
        return;
    m_oldOpenState=openState();
    m_newOpenState=v;
    m_animStartTime=QDateTime::currentMSecsSinceEpoch();
    startAnimation();
}
