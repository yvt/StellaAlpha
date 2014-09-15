#ifndef STCLIPVIEW_H
#define STCLIPVIEW_H

#include <QGraphicsObject>

class STClipView : public QGraphicsObject
{
    Q_OBJECT

    QRectF m_rect;
public:
    explicit STClipView(QGraphicsItem *parent = 0);
    
    void setRect(const QRectF&);
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual QRectF boundingRect() const;

signals:
    
public slots:
    
};

#endif // STCLIPVIEW_H
