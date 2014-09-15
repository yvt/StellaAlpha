#ifndef STRESIZEVIEW_H
#define STRESIZEVIEW_H

#include <QGraphicsObject>

class STResizeView : public QGraphicsObject
{
    Q_OBJECT

    bool m_drag;
    QPoint m_oldDragPos;

    QSize m_size;
public:
    explicit STResizeView(QGraphicsItem *parent = 0);

    void setSize(QSize);
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual QRectF boundingRect() const;
    
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
signals:
    
public slots:
    
};

#endif // STRESIZEVIEW_H
