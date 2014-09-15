#ifndef STNEWSVIEW_H
#define STNEWSVIEW_H

#include <QGraphicsObject>
#include <QGraphicsPixmapItem>

class STNewsShadowView;
class STNewsListView;
class STTweetNonEditingView;
class STNewsView : public QGraphicsObject
{
    Q_OBJECT

    QSize m_size;
    int m_maxHeight;

    STNewsShadowView *m_shadowView;
    STNewsListView *m_listView;
    STTweetNonEditingView *m_hoverView;

    float m_oldOpenState;
    float m_newOpenState;
    qint64 m_animStartTime;
    int m_animDuration;
    int m_animTimer;

    float openState() const;
    bool isAnimating();
    void startAnimation();

public:
    explicit STNewsView(QGraphicsItem *parent = 0);

    void relayout();
    void setSize(const QSize&s);

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual QRectF boundingRect() const;
    int currentHeight() const;

    virtual void focusInEvent(QFocusEvent *event);
    virtual void focusOutEvent(QFocusEvent *event);

    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);

    virtual void timerEvent(QTimerEvent *);

    void setOpenStateAnimated(float);

signals:
    void currentHeightChanged();

public slots:
    
};

#endif // STNEWSVIEW_H
