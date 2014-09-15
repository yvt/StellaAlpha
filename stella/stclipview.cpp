#include "stclipview.h"

STClipView::STClipView(QGraphicsItem *parent) :
    QGraphicsObject(parent),m_rect(0,0,0,0)
{
    setFlag(ItemHasNoContents);
    setFlag(ItemClipsChildrenToShape);
}

void STClipView::setRect(const QRectF &rt){
    if(rt==m_rect)return;
    prepareGeometryChange();
    m_rect=rt;
}

void STClipView::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){

}

QRectF STClipView::boundingRect() const{
    return m_rect;
}
