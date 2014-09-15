#ifndef STSTRIPEANIMATIONVIEW_H
#define STSTRIPEANIMATIONVIEW_H

#include <QGraphicsObject>
#include <QGraphicsPixmapItem>

class STStripeAnimationView : public QGraphicsObject
{
    Q_OBJECT

    QSize m_size;
    QGraphicsPixmapItem *m_item;
    QGraphicsPixmapItem *m_continuousItem;

    int m_timer;
    float m_value;

    void visibilityChanged();

public:
    explicit STStripeAnimationView(QGraphicsItem *parent = 0);
    
    void setSize(const QSize&);

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual QRectF boundingRect() const;

    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);

    virtual void timerEvent(QTimerEvent *);

    void setValue(float);
signals:
    
public slots:
    
};

#endif // STSTRIPEANIMATIONVIEW_H
