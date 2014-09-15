#ifndef STPLACEVIEW_H
#define STPLACEVIEW_H

#include "ststdafx.h"
#include <QGraphicsObject>
#include "stobjectmanager.h"

class STLinkedLabelView;
class STRemoteImageView;

class STPlaceView : public QGraphicsObject
{
    Q_OBJECT

    QSize m_size;

    QVariantMap m_coords;
    QVariantMap m_place;

    STLinkedLabelView *m_nameView;
    STLinkedLabelView *m_coordView;

    STRemoteImageView *m_mapView;

    int m_lastWidth;

public:
    explicit STPlaceView(QGraphicsItem *parent = 0);
    
    void setObjects(QVariantMap coords, QVariantMap place);

    void setSize(QSize);
    void relayout();

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual QRectF boundingRect() const;

signals:
    void linkActivated(STObjectManager::EntityRange *,
                       STObjectManager::LinkActivationInfo);
public slots:
    void childLinkActivated(STObjectManager::EntityRange *,
                            STObjectManager::LinkActivationInfo);
};

#endif // STPLACEVIEW_H
