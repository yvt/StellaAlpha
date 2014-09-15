#ifndef STNEWSLISTITEMVIEW_H
#define STNEWSLISTITEMVIEW_H

#include <QGraphicsObject>
#include "stnewslistview.h"
#include "stlinkedtextmanager.h"
#include <QGraphicsRectItem>
#include "stsimpleanimator.h"
#include "stusericonview.h"

class STTimelineItemTimeView;

class STNewsListItemView : public QGraphicsObject
{
    Q_OBJECT
    STNewsListView::NewsEntry m_entry;
    STLinkedTextManager m_text;
    QSize m_size;
    STTimelineItemTimeView *m_timeView;
    QGraphicsRectItem *m_flashView;

    STSimpleAnimator *m_flashAnimator;

    STUserIconView *m_iconView;
    STSimpleAnimator *m_iconViewAnimator;

    void updateCursor();
public:
    explicit STNewsListItemView(STNewsListView::NewsEntry *entry, QSize, QGraphicsItem *parent = 0);
    void setTriggerTime(QDateTime);
    
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual QRectF boundingRect() const;

    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    virtual void hoverMoveEvent(QGraphicsSceneHoverEvent *event);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

signals:
    void linkActivated(STObjectManager::EntityRange *,
                       STObjectManager::LinkActivationInfo);
private slots:
    void updateFlash(float);
    void updateIconViewOpacity(float);
    void activateNewsAccount();
};

#endif // STNEWSLISTITEMVIEW_H
