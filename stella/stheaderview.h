#ifndef STHEADERVIEW_H
#define STHEADERVIEW_H

#include <QGraphicsPixmapItem>
#include <QPixmap>
#include <QTimer>

class STButtonView;
class STHeaderView : public QGraphicsObject
{
    Q_OBJECT

public:

    struct Contents{
        QString title;
        bool hasBackButton;
        QString backButtonText;
        Contents():
            hasBackButton(false),
            backButtonText("Back"){

        }
    };

    enum TransitionType{
        SlideToLeft,
        SlideToRight,
        Dissolve
    };

    explicit STHeaderView(QGraphicsItem *parent = 0);

    void setContents(const Contents& c);
    void setContentsAnimated(const Contents& c, TransitionType);
    void setSize(const QSize& size){
        if(size==m_size)return;
        this->prepareGeometryChange();
        m_size=size;renderContents(); relayout();
                                   }
    void setUserBarWidth(int v){
        if(m_userBarWidth==v)return;
        m_userBarWidth=v;
        relayout();
    }

    void relayout();
    void renderContents();
    virtual QRectF boundingRect() const;
    virtual void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *);

    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

private:
    QGraphicsPixmapItem *m_pixmapItem;
    QGraphicsPixmapItem *m_oldPixmapItem;
    QSize m_size;
    int m_userBarWidth;
    Contents m_contents;
    Contents m_oldContents;
    QTimer m_timer;
    bool m_animating;
    TransitionType m_transitionType;
    quint64 m_animationStartTime;
    unsigned int m_animationDuration;
    float m_animationSlideDistance;

    STButtonView *m_backButton;
    STButtonView *m_backButtonOld;

    QPoint m_oldDragPos;
    bool m_drag;

    void drawContents(QPainter& painter, QPixmap&, float shiftX, float opacity, Contents&);
signals:
    void backButtonActivated();
private slots:
    void timerFired();
    void backButtonActivatedSlot();
};

#endif // STHEADERVIEW_H
