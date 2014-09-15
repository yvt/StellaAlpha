#include "sttweetnoneditingview.h"
#include <QPainter>

STTweetNonEditingView::STTweetNonEditingView(QGraphicsItem *parent):
    QGraphicsItem(parent),m_size(0.f,0.f)
{
    setCacheMode(DeviceCoordinateCache);
    this->setAcceptHoverEvents(true);
    this->setOpacity(0.01f);
}

void STTweetNonEditingView::setSize(const QSizeF &size){
    if(size==m_size)return;
    prepareGeometryChange();
    m_size=size;
    update();
}

void STTweetNonEditingView::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    painter->fillRect(boundingRect(), QColor(255,255,255,10));
}

QRectF STTweetNonEditingView::boundingRect() const{
    return QRectF(QPointF(0.f, 0.f), m_size);
}

void STTweetNonEditingView::hoverEnterEvent(QGraphicsSceneHoverEvent *event){
    setOpacity(1.f);
}

void STTweetNonEditingView::hoverLeaveEvent(QGraphicsSceneHoverEvent *event){
    setOpacity(0.01f);
}

void STTweetNonEditingView::mousePressEvent(QGraphicsSceneMouseEvent *event){
    ungrabMouse();
}


