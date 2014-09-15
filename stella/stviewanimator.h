#ifndef STVIEWANIMATOR_H
#define STVIEWANIMATOR_H

#include <QObject>
#include <QGraphicsItem>

class STViewAnimator : public QObject
{
    Q_OBJECT

    int m_timer;
    unsigned int m_duration;
    quint64 m_startTime;

    int m_opening;
    QGraphicsItem *m_oldView;
    QGraphicsItem *m_newView;
    bool m_deleteWhenDone;

    int m_width;

    bool m_visibleOnlyWhenParentIsVisible;

public:
    explicit STViewAnimator(QObject *parent = 0);
    
    bool isViewVisible(QGraphicsItem *);
    float viewXPos(QGraphicsItem *);

    float progress();
    bool isAnimating();

    void setWidth(int);

    void relayout(bool fromTimer=false);

    void startAnimation();
    virtual void timerEvent(QTimerEvent *);

    void activateView(QGraphicsItem *);
    void activateViewAnimated(QGraphicsItem *, int direction, int duration,
                              bool deleteWhenDone=false);
    void abortAnimation();

    // warning: STViewAnimator don't know when the parent becomes invisible/visible
    void setVisibleOnlyWhenParentIsVisible(bool);

signals:

    
private slots:
    

};

#endif // STVIEWANIMATOR_H
