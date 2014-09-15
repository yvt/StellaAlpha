#include "stzoomedimageview.h"
#include <math.h>
#include <QKeyEvent>


STZoomedImageView::STZoomedImageView(QGraphicsItem *parent) :
    QGraphicsObject(parent),m_size(0,0)
{
    setFlag(ItemHasNoContents);
    setFlag(ItemIsFocusable);
    m_lqImageItem=new QGraphicsPixmapItem(this);
    m_hqImageItem=new QGraphicsPixmapItem(this);

    m_hqImageItem->setCacheMode(DeviceCoordinateCache);

    m_progress=new STStripeAnimationView(this);
    m_progress->setVisible(false);

    m_hqImageTimer=new QTimer(this);
    connect(m_hqImageTimer, SIGNAL(timeout()),
            this, SLOT(viewHighQualityImage()));

    m_rotateAnimator=new STSimpleAnimator(this);
    m_rotateAnimator->setValue(0);
    connect(m_rotateAnimator, SIGNAL(valueChanged(float)),
            this, SLOT(relayout()));

    m_fadeAnimator=new STSimpleAnimator(this);
    m_fadeAnimator->setValue(0);
    connect(m_fadeAnimator, SIGNAL(valueChanged(float)),
            this, SLOT(relayout()));



    m_currentImageAngle=0.f;
}

void STZoomedImageView::setSize(QSize sz){
    if(sz==m_size)return;
    prepareGeometryChange();
    m_size=sz;
    relayout();
}

void STZoomedImageView::relayout(){
    if(m_size.isEmpty())
        return;

    m_progress->setOpacity(1.f-m_fadeAnimator->value());
    QSize progSz(200, 14);
    m_progress->setSize(progSz);
    m_progress->setPos((m_size.width()-progSz.width())/2,
                       (m_size.height()-progSz.height())/2);

    if(m_pixmap.isNull())
        return;
    QSizeF sz=QSizeF(m_pixmap.width(),m_pixmap.height());
    QSizeF vpSize(m_size.width(),m_size.height());
    QSizeF rotSz=QSizeF(sz.height(),sz.width()); // 90-deg rotated
    if(sz.width()>vpSize.width()){
        sz*=vpSize.width()/sz.width();
    }
    if(sz.height()>vpSize.height()){
        sz*=vpSize.height()/sz.height();
    }
    if(rotSz.width()>vpSize.width()){
        rotSz*=vpSize.width()/rotSz.width();
    }
    if(rotSz.height()>vpSize.height()){
        rotSz*=vpSize.height()/rotSz.height();
    }
    float rotateValue=m_rotateAnimator->value();
    float per=((rotateValue/2.f)-floorf(rotateValue/2.f))*2.f;
    if(per>=1.f)per=2.f-per;
    rotSz=QSizeF(rotSz.height(),rotSz.width()); // convert to local coord
    sz=sz*(1.f-per)+rotSz*per;

    QRectF rt(QPointF(roundf((vpSize.width()-sz.width())*.5f),
                      roundf((vpSize.height()-sz.height())*.5f)),
              sz);
    float ang=rotateValue*90.f;
    setImageRect(rt, ang);

    // layout high quality image
    QSizeF hqImageSize=m_hqImageItem->boundingRect().size();
    m_hqImageItem->setPos((vpSize.width()-hqImageSize.width())*.5f,
                          (vpSize.height()-hqImageSize.height())*.5f);

    m_lqImageItem->setVisible(!m_hqImageItem->isVisible());

    m_lqImageItem->setOpacity(m_fadeAnimator->value());
    m_hqImageItem->setOpacity(m_fadeAnimator->value());


}

void STZoomedImageView::setImageRect(QRectF rt, float ang){

    m_lqImageItem->setPos(rt.center());
    m_lqImageItem->setScale(rt.width()/(float)m_pixmap.width());
    m_lqImageItem->setRotation(ang);
    if(m_currentImageRect.size()!=rt.size()||
            m_currentImageAngle!=ang){
        m_hqImageItem->setVisible(false);
        m_hqImageTimer->stop();
        m_hqImageTimer->setSingleShot(true);
        m_hqImageTimer->start(800);
    }
    m_currentImageAngle=ang;
    m_currentImageRect=rt;
}

void STZoomedImageView::viewHighQualityImage(){
    if(m_size.isEmpty())
        return;
    QPixmap hqPixmap(m_size);
    hqPixmap.fill(QColor(0,0,0));

    {
        QPainter hqPainter(&hqPixmap);
        hqPainter.setRenderHint(QPainter::Antialiasing);
        hqPainter.setRenderHint(QPainter::HighQualityAntialiasing);
        hqPainter.setRenderHint(QPainter::SmoothPixmapTransform);

        hqPainter.translate(m_currentImageRect.center());
        hqPainter.rotate(m_currentImageAngle);
        hqPainter.scale(m_currentImageRect.width()/(float)m_pixmap.width(),
                        m_currentImageRect.width()/(float)m_pixmap.width());/*
        hqPainter.drawPixmap(QRectF(m_lqImageItem->offset(),m_lqImageItem->boundingRect().size()), m_pixmap,
                             QRectF(0,0,m_pixmap.width(),m_pixmap.height()));*/
        hqPainter.drawPixmap(m_lqImageItem->offset(),m_pixmap);
    }

    m_hqImageItem->setPixmap(hqPixmap);
    m_hqImageItem->setVisible(true);

    relayout();
}

void STZoomedImageView::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){

}

QRectF STZoomedImageView::boundingRect() const{
    return QRectF(0,0,m_size.width(),m_size.height());
}

void STZoomedImageView::setImage(QUrl url, QPixmap pix){
    m_pixmap=pix;

    m_lqImageItem->setPixmap(pix);
    m_lqImageItem->setCacheMode(ItemCoordinateCache);
    m_lqImageItem->setOffset(-(float)pix.width()*.5f,
                             -(float)pix.height()*.5f);

    if(pix.isNull()){
        m_fadeAnimator->setValue(0);
        m_progress->setVisible(true);
    }else{
        m_fadeAnimator->setValueAnimated(1, 200);
    }

    relayout();
}
void STZoomedImageView::keyPressEvent(QKeyEvent *event){
    if(event->key()==Qt::Key_Left){
        m_rotateAnimator->setValueAnimated(roundf(m_rotateAnimator->finalValue())-1.f,
                                           200);
        event->accept();
    }else if(event->key()==Qt::Key_Right){
        m_rotateAnimator->setValueAnimated(roundf(m_rotateAnimator->finalValue())+1.f,
                                           200);
        event->accept();
    }else{
        event->ignore();
    }
}
void STZoomedImageView::setProgressValue(float v){
    m_progress->setValue(v);
}
