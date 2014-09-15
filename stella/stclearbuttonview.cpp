#include "stclearbuttonview.h"
#include <QGraphicsSceneHoverEvent>
#include <QPixmapCache>

STClearButtonView::STClearButtonView(QGraphicsItem *parent) :
    QGraphicsObject(parent),m_size(0,0),
    m_drag(false),m_hover(false)
{
    setFlag(ItemHasNoContents);
    setAcceptsHoverEvents(true);

    m_item=new QGraphicsPixmapItem(this);
}

void STClearButtonView::setSize(const QSizeF &sz){
    if(sz==m_size)
        return;
    prepareGeometryChange();
    m_size=sz;
    relayout();
}

void STClearButtonView::relayout(){
    QPixmap pix=m_item->pixmap();
    QSizeF pixSize(0,0);
    if(!pix.isNull()){
        pixSize=pix.size();
    }
    m_item->setPos((m_size.width()-pixSize.width())*.5f,
                   (m_size.height()-pixSize.height())*.5f);

    if(!m_hover){
        m_item->setOpacity(0.6f);
    }else if(m_drag){
        m_item->setOpacity(0.4f);
    }else{
        m_item->setOpacity(0.8f);
    }

}

void STClearButtonView::setText(const QString &text){
    if(text.startsWith(":")){
        QPixmap pix;
        if(!QPixmapCache::find(text, &pix)){
            pix.load(text);
            QPixmapCache::insert(text, pix);
        }
        m_item->setPixmap(pix);
        relayout();
    }else{
        // TODO: text
    }
}

void STClearButtonView::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){

}

QRectF STClearButtonView::boundingRect()const{
    return QRectF(QPointF(0,0), m_size);
}

void STClearButtonView::hoverEnterEvent(QGraphicsSceneHoverEvent *event){
    m_hover=true;
    relayout();
}

void STClearButtonView::hoverLeaveEvent(QGraphicsSceneHoverEvent *event){
    m_hover=false;
    relayout();
}

void STClearButtonView::mousePressEvent(QGraphicsSceneMouseEvent *event){
    if(event->button()==Qt::LeftButton){
        m_drag=true;
        m_hover=true;
        relayout();
        grabMouse();
    }
}

void STClearButtonView::mouseMoveEvent(QGraphicsSceneMouseEvent *event){
    if(m_drag){
        m_hover=boundingRect().contains(event->pos());
        relayout();
    }
}

void STClearButtonView::mouseReleaseEvent(QGraphicsSceneMouseEvent *event){
    if(m_drag){
        m_drag=false;
        relayout();
        ungrabMouse();
        if(m_hover){
            emit activated();
        }
    }
}


