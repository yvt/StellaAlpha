#include "stresizeview.h"
#include <QCursor>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QWidget>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneHoverEvent>
#include <QPainter>

STResizeView::STResizeView(QGraphicsItem *parent) :
    QGraphicsObject(parent),m_size(0,0)
{
    //setAcceptedMouseButtons(Qt::LeftButton);
    //setFlag(ItemHasNoContents);
    setFlag(ItemIsFocusable);
    setCursor(Qt::SizeVerCursor);
    m_drag=false;
}

void STResizeView::mousePressEvent(QGraphicsSceneMouseEvent *event){

    m_drag=true;
    m_oldDragPos=event->screenPos();
    grabMouse();;

}

void STResizeView::mouseMoveEvent(QGraphicsSceneMouseEvent *event){
    if(m_drag){
        QPoint delta=event->screenPos()-m_oldDragPos;
        m_oldDragPos=event->screenPos();
        QGraphicsView *vw=this->scene()->views()[0];
        QWidget *wid=vw->topLevelWidget();
        int newHeight=wid->height()+delta.y();
        if(newHeight<wid->minimumHeight()){
            m_oldDragPos.ry()+=wid->minimumHeight()-newHeight;
            newHeight=wid->minimumHeight();

        }
        wid->resize(wid->width(), newHeight);
    }


}

void STResizeView::mouseReleaseEvent(QGraphicsSceneMouseEvent *event){
    m_drag=false;
    ungrabMouse();;
}

void STResizeView::setSize(QSize sz){
    if(sz==m_size)return;
    prepareGeometryChange();
    m_size=sz;
}

void STResizeView::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *){

    painter->fillRect(0,1,m_size.width(), m_size.height()-1, QColor(185,185,185));
    //painter->fillRect(0,0,m_size.width(), 1, QColor(60,60,60));
}

QRectF STResizeView::boundingRect() const{
    return QRectF(0,0,m_size.width(), m_size.height());
}
