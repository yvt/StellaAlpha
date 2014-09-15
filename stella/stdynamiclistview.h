#ifndef STDYNAMICLISTVIEW_H
#define STDYNAMICLISTVIEW_H

#include <QGraphicsObject>
#include "stdynamicanimator.h"
#include <QList>
#include <QLinkedList>
#include "stscrollbarview.h"
#include "stclipview.h"
#include <QMap>
#include "stsimpleanimator.h"

class STDynamicListView : public QGraphicsObject
{
    Q_OBJECT
public:
    enum FollowMode{
        AlwaysFollow,
        NeverFollow,
        AutoFollow
    };

private:
    struct Item{
        quint64 id;
        QGraphicsItem *view;
        bool beingRemoved:1;
        bool wasPurged:1;
        double height;
        STDynamicAnimator posAnimator;
        quint64 appearTime;
        double visiblePos;
        double visualPos;
        double lastVisiblePos;
    };

    QSize m_size;
    QLinkedList<Item> m_items;
    QMap<quint64, QLinkedList<Item>::iterator> m_itemsIndex;
    STDynamicAnimator m_topPosAnimator;
    STDynamicAnimator m_bottomPosAnimator;
    FollowMode m_followMode;

    STScrollBarView *m_scrollBar;
    STClipView *m_clipView;
    QGraphicsPixmapItem *m_shadowView;
    QGraphicsPixmapItem *m_dragTopShadowView; // visible while drag
    QGraphicsPixmapItem *m_dragBottomShadowView; // visible while drag
    float m_shadowSize;

    bool m_ignoreValueChange;

    bool m_pinned;
    double m_lastTopPos;
    bool m_autoScrolling;
    STDynamicAnimator m_autoScroller;

    int m_timer;
    int m_maxItemCount;
    quint64 currentTime();
    Item *itemWithId(quint64);
    double itemTopPos();
    QLinkedList<Item>::iterator posToInsertItem(quint64 id);

    enum DragMode{
        NoDrag=0,
        Dragging,
        DragDone
    };

    DragMode m_dragging;
    quint64 m_dragId;
    double m_draggedItemPos; // in view coordinate
    double m_dragPoint; // in item coordinate
    double m_dragOldFinalPos; // in case dragged item, so itemTopPos works
    STDynamicAnimator m_dragEndAnimator;

public:
    explicit STDynamicListView(QGraphicsItem *parent = 0);
    
    void setSize(const QSize&);
    QSize size() const{return m_size;}
    quint64 relayout();
    virtual void relayoutScrollBar();
    void updateShadow();

    FollowMode followMode() const{return m_followMode;}

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual QRectF boundingRect() const;

    STScrollBarView *scrollBarView() const{return m_scrollBar;}

    virtual void timerEvent(QTimerEvent *);
    bool isAnimating(quint64 tm=0);
    void startAnimation();

    quint64 idForView(QGraphicsItem *) const;

    void setItems(QList<QPair<quint64, double> >);

    void insertItem(quint64 id, double height);
    void removeItem(quint64 id);
    void setMaxItemCount(int i);
    int itemCount() const;
    void initiateDragItem(quint64, double initialShift=0.);
    void abortDrag();

    int itemPreferredWidth() const;



    virtual void wheelEvent(QGraphicsSceneWheelEvent *event);

public slots:


    void initateBulkLoad();// animation used during initial fetch

    void setShadowOpacity(float);
    void setShadowSize(float);
    void autoScrollTo(double); // followMode must be AlwaysFollow for all value, AutoFollow for 0
    void autoScrollToRow(quint64); // warning: doesn't follow item's movement
    void setFollowMode(FollowMode md);

    void abortAutoScroll();

protected:
    virtual QGraphicsItem *createView(quint64 id, double height, STClipView *parent);
    virtual void itemBeingPurged(quint64 id,double height); // just before deletig
    virtual void itemWasPurged(quint64 id, double height);  // deleted
    virtual void itemMoved(quint64 oldId, quint64 newId);   // by drag

    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
signals:
    
private slots:
    void scrollChanged();

};

#endif // STDYNAMICLISTVIEW_H
