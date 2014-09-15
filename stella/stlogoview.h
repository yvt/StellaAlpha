#ifndef STLOGOVIEW_H
#define STLOGOVIEW_H

#include <QGraphicsObject>
#include <QGraphicsPixmapItem>

class STLogoView : public QGraphicsObject
{
    Q_OBJECT
    QSize m_size;
    QGraphicsPixmapItem *m_image;
public:
    explicit STLogoView(QGraphicsItem *parent = 0);
    
    void setSize(QSize);
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual QRectF boundingRect() const;
signals:
    
public slots:
    
};

#endif // STLOGOVIEW_H
