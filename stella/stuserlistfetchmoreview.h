#ifndef STUSERLISTFETCHMOREVIEW_H
#define STUSERLISTFETCHMOREVIEW_H

#include "ststdafx.h"
#include <QGraphicsObject>
#include "stobjectmanager.h"

class STUserListModel;
class STButtonView;
class STStripeAnimationView;

class STUserListFetchMoreView : public QGraphicsObject
{
    Q_OBJECT

    QSize m_size;

    STButtonView *m_buttonView;
    STStripeAnimationView *m_animationView;
public:
    explicit STUserListFetchMoreView(QSize, QGraphicsItem *parent = 0);
    
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual QRectF boundingRect() const;

public slots:
    void setFetching(bool);

signals:
    void fetchMoreRequested();
private slots:
    void doFetch();

};

#endif // STUSERLISTFETCHMOREVIEW_H
