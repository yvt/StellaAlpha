#ifndef STSTATUSUSERNAMEVIEW_H
#define STSTATUSUSERNAMEVIEW_H

#include <QGraphicsObject>
#include "ststdafx.h"
#include "stlinkedtextmanager.h"
#include "stobjectmanager.h"

class STStatusUserNameView : public QGraphicsObject
{
    Q_OBJECT
    QSize m_size;

    STObjectManager::UserRef m_user;
    STLinkedTextManager m_nameLinkManager;
    STLinkedTextManager m_screenNameLinkManager;

    void updateCursor();

public:
    explicit STStatusUserNameView(QGraphicsItem *parent = 0);
    void setWidth(int);
    int height() const{return m_size.height();}
    int boxHeight() const;

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual QRectF boundingRect() const;

    void setUser(STObjectManager::UserRef);

protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    virtual void hoverMoveEvent(QGraphicsSceneHoverEvent *event);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

signals:
    void linkActivated(STObjectManager::EntityRange *,
                       STObjectManager::LinkActivationInfo);
public slots:
    
};

#endif // STSTATUSUSERNAMEVIEW_H
