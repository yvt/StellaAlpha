#ifndef STCLEARBUTTONVIEW_H
#define STCLEARBUTTONVIEW_H

#include <QGraphicsObject>
#include <QGraphicsPixmapItem>

class STClearButtonView : public QGraphicsObject
{
    Q_OBJECT

    QSizeF m_size;
    QGraphicsPixmapItem *m_item;
    bool m_drag, m_hover;

public:
    explicit STClearButtonView(QGraphicsItem *parent = 0);
    
    void setSize(const QSizeF&);
    void setText(const QString&);

    void relayout();

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual QRectF boundingRect() const;

    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

signals:
    void activated();
public slots:
    
};

#endif // STCLEARBUTTONVIEW_H
