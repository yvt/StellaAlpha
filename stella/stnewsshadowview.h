#ifndef STNEWSSHADOWVIEW_H
#define STNEWSSHADOWVIEW_H

#include <QGraphicsItem>

class STNewsShadowView : public QGraphicsItem
{
    int m_width;
public:
    STNewsShadowView(QGraphicsItem *);
    void setWidth(int);

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual QRectF boundingRect() const;

};

#endif // STNEWSSHADOWVIEW_H
