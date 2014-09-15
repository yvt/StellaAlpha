#ifndef STBUTTONGRAPHICSVIEW_H
#define STBUTTONGRAPHICSVIEW_H

#include <QGraphicsItem>
#include "stbuttonview.h"

class STButtonGraphicsView : public QGraphicsItem
{
    QSize m_size;
    STButtonView::ButtonState m_state;
public:
    STButtonGraphicsView(STButtonView *parent, STButtonView::ButtonState state);

    void setSize(const QSize& sz){
        if(m_size==sz) return;
        prepareGeometryChange();
        m_size=sz;
    }

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual QRectF boundingRect() const;
};

#endif // STBUTTONGRAPHICSVIEW_H
