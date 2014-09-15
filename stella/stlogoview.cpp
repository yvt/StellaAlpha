#include "stlogoview.h"
#include <QPixmapCache>

STLogoView::STLogoView(QGraphicsItem *parent) :
    QGraphicsObject(parent),m_size(0,0)
{
    setFlag(ItemHasNoContents);
    m_image=new QGraphicsPixmapItem(this);

    // logo
    static QPixmapCache::Key key;
    QPixmap logo;
    if(!QPixmapCache::find(key, &logo)){
        logo.load(":/stella/res/StellaEmbossBlack.png");
        key=QPixmapCache::insert(logo);
    }

    m_image->setPixmap(logo);
}

void STLogoView::setSize(QSize sz){
    if(sz==m_size)
        return;
    m_size=sz;

    QPixmap pix=m_image->pixmap();
    m_image->setPos((m_size.width()-pix.width())/2,
                    (m_size.height()-pix.height())/2);
}

void STLogoView::paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *){

}

QRectF STLogoView::boundingRect() const{
    return QRectF(0,0,m_size.width(),m_size.height());
}

