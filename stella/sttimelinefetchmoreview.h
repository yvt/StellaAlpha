#ifndef STTIMELINEFETCHMOREVIEW_H
#define STTIMELINEFETCHMOREVIEW_H

#include <QGraphicsObject>

class STTimeline;
class STButtonView;
class STStripeAnimationView;

class STTimelineFetchMoreView : public QGraphicsObject
{
    Q_OBJECT

    QSize m_size;
    STTimeline *m_timeline;
    quint64 m_id;

    STButtonView *m_button;
    STStripeAnimationView *m_animView;

    bool isFetching();
public:
    explicit STTimelineFetchMoreView(QSize size, STTimeline *timeline, quint64 id, QGraphicsItem *parent = 0);
    
    static QSize defaultSize(int width);

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual QRectF boundingRect() const;



signals:
    
public slots:
    void updateState(quint64 id);
    void doFetch();
};

#endif // STTIMELINEFETCHMOREVIEW_H
