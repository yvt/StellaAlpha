#ifndef STTWEETSHADOWVIEW_H
#define STTWEETSHADOWVIEW_H

#include <QGraphicsItem>

class STTweetShadowView : public QGraphicsItem
{
    int m_width;
public:
    STTweetShadowView(QGraphicsItem *parent);
    void setWidth(int);

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual QRectF boundingRect() const;
};

#endif // STTWEETSHADOWVIEW_H
