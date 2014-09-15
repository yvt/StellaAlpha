#ifndef STSTATICLISTVIEW_H
#define STSTATICLISTVIEW_H

#include "ststdafx.h"
#include <QGraphicsObject>
#include "stdynamicanimator.h"
#include <QLinkedList>

class STScrollBarView;
class STClipView;

class STStaticListView : public QGraphicsObject
{
    Q_OBJECT

    QSize m_size;
    STScrollBarView *m_scrollBar;
    int m_itemHeight;
    STClipView *m_clipView;

    qint64 m_itemsCount;

    qint64 m_loadedFirstIndex;
    QList<QGraphicsItem *> m_loadedItems;

    qint64 m_topItemPos;
    STDynamicAnimator m_topClipPos;
    STDynamicAnimator m_bottomClipPos;

    qint64 m_animatedFirstIndex;
    STDynamicAnimator m_animatedItemsPos;

    struct RemovedItem{
        qint64 originalPos;
        QGraphicsItem *view;
        bool lowestItem;
    };

    QLinkedList<RemovedItem> m_itemsBeingRemoved;

    int m_timerId;

    bool shouldAnimate();
    void startAnimation();

    void completeInnerAnimation();

    void markItemsToDelete(qint64 startIndex, qint64 endIndex, bool lowest);

public:
    explicit STStaticListView(QGraphicsItem *parent = 0);
    
    void setSize(const QSize&);


    void setItemHeight(int);
    int itemHeight() const{return m_itemHeight;}

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual QRectF boundingRect() const;

    qint64 insertItem(qint64 index, qint64 count=1);
    qint64 prependItem(qint64 count=1);
    qint64 appendItem(qint64 count=1);
    void removeItem(qint64 startIndex, qint64 count=1);
    qint64 itemCount() const{return m_itemsCount;}
    int itemPreferredWidth() const;

    virtual void wheelEvent(QGraphicsSceneWheelEvent *event);

protected:

    virtual void timerEvent(QTimerEvent *);

    virtual QGraphicsItem *createView(qint64 index, STClipView *parent);

signals:
    
public slots:
    void relayout();
};

#endif // STSTATICLISTVIEW_H
