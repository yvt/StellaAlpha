#include "stremoteimageview.h"
#include <QPainter>
#include <QMutexLocker>
#include "stsimpleanimator.h"
#include <QGraphicsSceneMouseEvent>
#include <QDebug>
#include "stzoomedimagedownloader.h"
#include "ststripeanimationview.h"

STRemoteImageView::STRemoteImageView( QGraphicsItem *parent) :
    QGraphicsObject(parent), m_size(0,0)
{

    m_pixmapItem=new QGraphicsPixmapItem(this);

    m_progressViewAnimator=NULL;
    m_progressView=NULL;
    m_borderless=false;

    updateIcon();
    setCacheMode(DeviceCoordinateCache);
    m_pixmapItem->setFlag(ItemStacksBehindParent);
    m_pixmapItem->setPos(2,2);

    setAcceptedMouseButtons(Qt::NoButton);
    m_pixmapItem->setAcceptedMouseButtons(Qt::NoButton);

    m_animator=NULL;




}

void STRemoteImageView::setBorderless(bool b){
    m_borderless=b;
    m_pixmapItem->setPos(b?0:2,b?0:2);
    setFlag(ItemHasNoContents, b);
    updateIcon();
}

void STRemoteImageView::setZoomedImageUrl(QUrl url){
    connect(STZoomedImageDownloader::sharedDownloader(),
            SIGNAL(imageLoadStarted(QUrl)),
            this,
            SLOT(imageLoadStarted(QUrl)),
            Qt::UniqueConnection);
    connect(STZoomedImageDownloader::sharedDownloader(),
            SIGNAL(imageLoaded(QUrl,QPixmap)),
            this,
            SLOT(imageLoadDone(QUrl)),
            Qt::UniqueConnection);
    connect(STZoomedImageDownloader::sharedDownloader(),
            SIGNAL(imageLoadFailed(QUrl,QString)),
            this,
            SLOT(imageLoadDone(QUrl)),
            Qt::UniqueConnection);
    connect(STZoomedImageDownloader::sharedDownloader(),
            SIGNAL(imageLoadProgress(QUrl,float)),
            this,
            SLOT(imageLoadProgress(QUrl,float)));
    m_zoomedImageUrl=url;
}

void STRemoteImageView::mousePressEvent(QGraphicsSceneMouseEvent *event){

}

void STRemoteImageView::mouseReleaseEvent(QGraphicsSceneMouseEvent *event){
    if(boundingRect().contains(event->pos())){
        qDebug()<<m_ticket->url;
        emit clicked();
    }
}

void STRemoteImageView::setTicket(STRemoteImageManager::TicketRef t){
    if(m_ticket){
        disconnect(this, SLOT(imageReady(quint64)));
    }
    m_ticket=t;
    if(m_ticket){
        connect(m_ticket->manager, SIGNAL(iconReady(quint64)),
                this, SLOT(imageReady(quint64)));
    }
    updateIcon();
}


void STRemoteImageView::setSize(const QSize & size){
    if(size==m_size)return;
    prepareGeometryChange();;
    m_size=size;
    update();
    updateIcon();

}

void STRemoteImageView::opacityAnimate(float op){
    m_pixmapItem->setOpacity(op);
}

void STRemoteImageView::imageReady(quint64 hash){
    if(!m_ticket)
        return;
    if(hash!=m_ticket->hash)
        return;
    if(sender()!=m_ticket->manager)
        return;
    if(!m_animator){
        m_animator=new STSimpleAnimator(this);
        connect(m_animator, SIGNAL(valueChanged(float)),
                this, SLOT(opacityAnimate(float)));
        m_animator->setValue(0.f);
        m_animator->setValueAnimated(1.f, 200);
    }
    updateIcon();
}

void STRemoteImageView::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *){
    painter->setBrush(QColor(0,0,0,0));

    painter->setPen(QColor(255,255,255,12));
    painter->drawRect(0,0,m_size.width()-1,m_size.height()-1);

    painter->setPen(QColor(50,50,50));
    painter->drawRect(1,1,m_size.width()-3,m_size.height()-3);

    painter->fillRect(2,2,m_size.width()-4, 1, QColor(0,0,0,40));
    painter->fillRect(2,2,1,m_size.height()-4, QColor(0,0,0,40));
}

QRectF STRemoteImageView::boundingRect() const{
    return QRectF(0,0,m_size.width(),m_size.height());
}

void STRemoteImageView::updateIcon(){

    QSize imgAreaSize;
    if(m_borderless)
        imgAreaSize=m_size;
    else
        imgAreaSize=m_size-QSize(4,4);

    if(m_progressView){
        m_progressView->setSize(QSize(imgAreaSize.width(), 2));
        m_progressView->setPos(2,imgAreaSize.height());
    }

    if(!m_ticket)
        return;
    QPixmap pix;
    {
        QMutexLocker locker(&(m_ticket->mutex));
        if(m_ticket->pixmap.isNull() && !m_ticket->image.isNull()){
            m_ticket->pixmap=QPixmap::fromImage(m_ticket->image);
        }
        pix=m_ticket->pixmap;
    }
/*
    if(m_ticket->url.toString()=="http://twitpic.com/show/mini/b1riuy"){
        qDebug()<<m_ticket->url;
    }*/

    if(pix.isNull())
        return;

    pix=pix.scaled(qMax(imgAreaSize.width(),1), qMax(imgAreaSize.height(),1),
                   Qt::KeepAspectRatioByExpanding,
                   Qt::SmoothTransformation);
    if(pix.width()>imgAreaSize.width()){
        QRect rt((pix.width()-(imgAreaSize.width()))/2, 0,
                 imgAreaSize.width(), pix.height());
        pix=pix.copy(rt);
    }else if(pix.height()>imgAreaSize.height()){
        QRect rt(0, (pix.height()-(imgAreaSize.height()))/2,
                 pix.width(), imgAreaSize.height());
        pix=pix.copy(rt);
    }

    if(m_borderless){
        // round edge
        QPixmap pix2(pix.size());
        pix2.fill(QColor(0,0,0,0));

        QPainter painter(&pix2);
        QPainterPath path;
        path.addRoundedRect(QRectF(0,0,m_size.width(),m_size.height()),20.f,20.f,Qt::RelativeSize);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setRenderHint(QPainter::HighQualityAntialiasing);
        painter.setClipPath(path);
        painter.drawPixmap(0,0,pix);/*
        painter.setPen(QColor(0,0,0,128));
        painter.setBrush(QColor(0,0,0,0));
        painter.drawPath(path);*/
        m_pixmapItem->setPixmap(pix2);
    }else{

        m_pixmapItem->setPixmap(pix);
    }
}

void STRemoteImageView::progressOpacityAnimate(float op){
    if(!m_progressView)return;
    m_progressView->setOpacity(op);
}

void STRemoteImageView::setProgress(float vl){
    if(!m_progressView){
        m_progressView=new STStripeAnimationView(this);
        updateIcon(); // force relayout

        m_progressViewAnimator=new STSimpleAnimator(this);
        connect(m_progressViewAnimator,SIGNAL(valueChanged(float)),
                this, SLOT(progressOpacityAnimate(float)));
        m_progressViewAnimator->setValue(0);

    }

    m_progressViewAnimator->setValueAnimated(1.f, 200);

    m_progressView->setValue(vl);

}

void STRemoteImageView::imageLoadStarted(QUrl ur){
    if(ur==m_zoomedImageUrl){
        setProgress(-2.f);
    }
}
void STRemoteImageView::imageLoadProgress(QUrl ur, float val){
    if(ur==m_zoomedImageUrl)
        setProgress(val);
}

void STRemoteImageView::imageLoadDone(QUrl ur){
    if(ur==m_zoomedImageUrl){
        if(!m_progressViewAnimator)
            return;

        m_progressViewAnimator->setValueAnimated(0.f, 200);
    }
}
