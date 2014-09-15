#ifndef STSCROLLTRACKBAR_H
#define STSCROLLTRACKBAR_H

#include <QGraphicsItem>
#include "stscrollbarview.h"

class STScrollTrackBar : public QGraphicsItem
{
    QSize m_size;
    QPointF m_lastScrollPos;
    bool m_hover, m_drag;
public:
    STScrollTrackBar(STScrollBarView *parent);
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual QRectF boundingRect() const;
    void setSize(const QSize&);

    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
};

#endif // STSCROLLTRACKBAR_H
