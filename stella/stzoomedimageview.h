#ifndef STZOOMEDIMAGEVIEW_H
#define STZOOMEDIMAGEVIEW_H

#include "ststdafx.h"
#include <QGraphicsObject>
#include <QGraphicsPixmapItem>
#include <QTimer>
#include "stsimpleanimator.h"
#include "ststripeanimationview.h"

class STZoomedImageView : public QGraphicsObject
{
    Q_OBJECT
    QSize m_size;
    QPixmap m_pixmap;
    QGraphicsPixmapItem *m_lqImageItem;
    QGraphicsPixmapItem *m_hqImageItem;
    QTimer *m_hqImageTimer;
    QRectF m_currentImageRect;
    float m_currentImageAngle;
    STSimpleAnimator *m_rotateAnimator;
    STSimpleAnimator *m_fadeAnimator;
    STStripeAnimationView *m_progress;

    void setImageRect(QRectF, float angle);
public:
    explicit STZoomedImageView(QGraphicsItem *parent = 0);
    
    void setSize(QSize);


    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual QRectF boundingRect() const;

    void setImage(QUrl, QPixmap);

    virtual void keyPressEvent(QKeyEvent *event);

    void setProgressValue(float);
signals:
    
public slots:
    void relayout();
private slots:
    void viewHighQualityImage();


};

#endif // STZOOMEDIMAGEVIEW_H
