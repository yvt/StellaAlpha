#ifndef STREMOTEIMAGEVIEW_H
#define STREMOTEIMAGEVIEW_H

#include <QGraphicsObject>
#include "stremoteimagemanager.h"
#include "stsimpleanimator.h"

class STStripeAnimationView;

class STRemoteImageView : public QGraphicsObject
{
    Q_OBJECT

    QSize m_size;
    QGraphicsPixmapItem *m_pixmapItem;
    STRemoteImageManager::TicketRef m_ticket;

    STStripeAnimationView *m_progressView;

    STSimpleAnimator *m_animator;
    STSimpleAnimator *m_progressViewAnimator;

    QUrl m_zoomedImageUrl;
    bool m_borderless;

    void updateIcon();

public:
    explicit STRemoteImageView(QGraphicsItem *parent = 0);
    void setSize(const QSize&);

    void setTicket(STRemoteImageManager::TicketRef);
    STRemoteImageManager::TicketRef& ticket() {return m_ticket;}

    void setZoomedImageUrl(QUrl);
    QUrl zoomedImageUrl() const{return m_zoomedImageUrl;}

    void setBorderless(bool);

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual QRectF boundingRect() const;

    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
signals:
    void clicked();
public slots:
    void imageReady(quint64 hash);

private slots:
    void opacityAnimate(float);
    void progressOpacityAnimate(float);

    void imageLoadDone(QUrl);
    void imageLoadProgress(QUrl, float);
    void imageLoadStarted(QUrl);

    void setProgress(float);
    
};

#endif // STREMOTEIMAGEVIEW_H
