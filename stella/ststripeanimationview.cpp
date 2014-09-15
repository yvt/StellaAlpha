#include "ststripeanimationview.h"
#include <QDateTime>
#include <QPainter>
#include <QPainterPath>
#include <QBrush>

STStripeAnimationView::STStripeAnimationView(QGraphicsItem *parent) :
    QGraphicsObject(parent), m_size(0,0)
{
    m_item=new QGraphicsPixmapItem(this);
    m_continuousItem=new QGraphicsPixmapItem(this);
    m_timer=startTimer(32);

    setFlag(ItemHasNoContents);

    setFlag(ItemClipsChildrenToShape);
    m_value=2.f;
    m_continuousItem->setVisible(false);

}

void STStripeAnimationView::setSize(const QSize &sz){
    if(sz==m_size)
        return;
    prepareGeometryChange();
    m_size=sz;

    {
        QPixmap pix(m_size.width()+32, m_size.height());
        pix.fill(QColor(60, 60, 60));
        {
            QPainter painter(&pix);
            int interval=32;
            int stripeWidth=16;
            if(m_size.height()<4){
                interval=7;
                stripeWidth=3;
            }
            for(int x=0;x<m_size.width()+m_size.height()+32;x+=interval){
                QPainterPath path;
                path.moveTo(x, 0);
                path.lineTo(x+stripeWidth, 0);
                path.lineTo(x-m_size.height()+stripeWidth, m_size.height());
                path.lineTo(x-m_size.height(), m_size.height());
                painter.fillPath(path, QColor(220,128, 40));
            }

        }
        m_item->setPixmap(pix);
    }
    {
        QPixmap pix(m_size.width()*2, m_size.height());
        pix.fill(QColor(60, 60, 60));
        {
            QPainter painter(&pix);
            painter.fillRect(0,0,m_size.width(),m_size.height(), QColor(220,128,40));
        }
        m_continuousItem->setPixmap(pix);
    }
    timerEvent(NULL);
}

void STStripeAnimationView::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    painter->fillRect(boundingRect(), QColor(60,60,60));
}

QRectF STStripeAnimationView::boundingRect() const{
    return QRectF(0,0,m_size.width(),m_size.height());
}

void STStripeAnimationView::visibilityChanged(){
    if(isVisible() && opacity()>=.01f && !(m_value>=0.f && m_value<=1.f)){
        if(!m_timer){
            m_timer=startTimer(32);
            timerEvent(NULL);
        }
    }else{
        if(m_timer){
            killTimer(m_timer);
            m_timer=0;
        }
    }
}

QVariant STStripeAnimationView::itemChange(GraphicsItemChange change, const QVariant &value){
    if(change==ItemVisibleHasChanged){
        visibilityChanged();
    }else if(change==ItemOpacityHasChanged){
        visibilityChanged();;
    }
    return value;
}


void STStripeAnimationView::timerEvent(QTimerEvent *){
    if(m_value>=0.f && m_value<=1.f){
        m_item->setVisible(false);
        m_continuousItem->setVisible(true);
        m_continuousItem->setPos((float)m_size.width()*(m_value-1.f),0.f);
    }else{
        int frm=(int)(QDateTime::currentMSecsSinceEpoch()&1023);
        int interval=32;
        if(m_size.height()<4){
            interval=7;
            frm=(frm<<2)&1023;
        }
        m_item->setPos(((float)frm/1024.f-1.f)*(float)interval, 0);
        m_item->setVisible(true);
        m_continuousItem->setVisible(false);
    }
}

void STStripeAnimationView::setValue(float vl){
    if(m_value==vl)
        return;
    m_value=vl;
    visibilityChanged();
    timerEvent(NULL);
}
