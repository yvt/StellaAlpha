#ifndef STSTARTUPVIEW_H
#define STSTARTUPVIEW_H

#include <QGraphicsObject>
#include "stresizeview.h"
class STLogoView;

class STStartupView : public QGraphicsObject
{
    Q_OBJECT

    QSize m_size;
    STResizeView *m_resizeView;
    STLogoView *m_logoView;
public:
    explicit STStartupView(QGraphicsItem *parent = 0);
    
    void setSize(QSize);

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual QRectF boundingRect() const;
signals:
    
public slots:
    
};

#endif // STSTARTUPVIEW_H
