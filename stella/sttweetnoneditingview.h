#ifndef STTWEETNONEDITINGVIEW_H
#define STTWEETNONEDITINGVIEW_H

#include <QGraphicsItem>

class STTweetNonEditingView : public QGraphicsItem
{
    QSizeF m_size;
public:
    STTweetNonEditingView(QGraphicsItem *parent);

    void setSize(const QSizeF&);

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual QRectF boundingRect() const;

    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
};

#endif // STTWEETNONEDITINGVIEW_H
