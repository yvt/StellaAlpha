#include "sttweetshadowview.h"
#include <QLinearGradient>
#include <QBrush>
#include <QPainter>

static const int m_shadowHeight=20;

STTweetShadowView::STTweetShadowView(QGraphicsItem *parent):
    QGraphicsItem(parent),m_width(0)
{
    this->setAcceptedMouseButtons(Qt::NoButton);
    setCacheMode(DeviceCoordinateCache);
}

void STTweetShadowView::setWidth(int w){
    if(w==m_width)return;
    prepareGeometryChange();
    m_width=w;
    update();
}


void STTweetShadowView::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){

    QLinearGradient grad(0.f, 0.f, 0.f, m_shadowHeight);
    grad.setColorAt(0.f, QColor(0,0,0,70));
    grad.setColorAt(0.2f, QColor(0,0,0,23));
    grad.setColorAt(1.f, QColor(0,0,0,0));
    //grad.setColorAt(1.f, QColor(0,0,0));
    painter->fillRect(QRect(0, 1, m_width, m_shadowHeight-1), QBrush(grad));
    painter->fillRect(QRect(0, 0, m_width, 1),
                      QColor(50,50,50));

}

QRectF STTweetShadowView::boundingRect() const{
    return QRectF(0.f,0.f,m_width,m_shadowHeight);
}


