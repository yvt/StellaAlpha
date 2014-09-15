#ifndef STUSERLISTITEMVIEW_H
#define STUSERLISTITEMVIEW_H

#include "ststdafx.h"
#include <QGraphicsObject>
#include "stobjectmanager.h"
#include "stusericonview.h"
#include "stlinkedtextmanager.h"
#include <QGraphicsPixmapItem>

class STUserListModel;


class STUserListItemView : public QGraphicsObject
{
    Q_OBJECT

    QSize m_size;
    qint64 m_rowId;
    quint64 m_userId;
    STObjectManager::UserRef m_user;

    STUserListModel *m_model;

    STUserIconView*m_iconView;
    QGraphicsPixmapItem *m_placeholderView;

    STLinkedTextManager m_userNameLinkManager;
    STLinkedTextManager m_userScreenNameLinkManager;
    STLinkedTextManager m_bioLinkManager;

    void updateCursor();

    int m_fetchTimer;
    int m_animTimer;

    qint64 m_animStartTime;
    int m_animDuration;

    bool shouldAnimate();
    float placeholderOpacity();

public:
    explicit STUserListItemView(qint64 rowId, QSize, STUserListModel *model, QGraphicsItem *parent = 0);

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual QRectF boundingRect() const;

    virtual void timerEvent(QTimerEvent *);

    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    virtual void hoverMoveEvent(QGraphicsSceneHoverEvent *event);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

signals:

    void linkActivated(STObjectManager::EntityRange *,
                       STObjectManager::LinkActivationInfo);

    
public slots:
    void itemStateUpdated();
    void showUser();
};

#endif // STUSERLISTITEMVIEW_H
