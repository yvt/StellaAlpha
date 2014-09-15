#include "ststdafx.h"
#include "stauthorizecancelbutton.h"
#include <QPainter>
#include <QPixmapCache>
#include <QImage>
#include "stfont.h"

STAuthorizeCancelButton::STAuthorizeCancelButton(QWidget *parent) :
    QPushButton(parent)
{

    setText(tr("Cancel"));
    setFixedSize(91, 25);

}

bool STAuthorizeCancelButton::hitButton(const QPoint &pos) const{
    return QRect(1,0,91-1,25).contains(pos);
}

QSize STAuthorizeCancelButton::sizeHint() const{
    return QSize(91,25);
}

QSize STAuthorizeCancelButton::minimumSizeHint() const{
    return QSize(91,25);
}


void STAuthorizeCancelButton::paintEvent(QPaintEvent *event){
    QPixmap pix;
    if(isDown()){
        static QPixmapCache::Key key;
        if(!QPixmapCache::find(key, &pix)){
            pix.load(":/stella/res/AuthorizeCancelButtonOn.png");
            key=QPixmapCache::insert(pix);
        }
    }else{
        static QPixmapCache::Key key;
        if(!QPixmapCache::find(key, &pix)){
            pix.load(":/stella/res/AuthorizeCancelButton.png");
            key=QPixmapCache::insert(pix);
        }
    }

    QImage img(pix.size(), QImage::Format_RGB32);
    img.fill(0);

    QPainter imgPainter(&img);
    imgPainter.drawPixmap(0,0,pix);

    STFont *font=STFont::defaultFont();
    STFont::DrawOption opt;
    STFont::Layout lay=font->layoutString(text(), img.width()-6, true);
    QSizeF sz(img.width()-2, img.height()-1);
    QRectF bnd=font->boundingRectForLayout(lay);

    opt.useSubpixelRendering=false;

    QPointF pos((sz.width()-bnd.right())*.5f, (sz.height()-bnd.bottom())*.5f+2.f);
    font->drawLayout(img, pos-QPointF(0,1), QColor(0,0,0,64),
                     lay, opt);
    font->drawLayout(img, pos, QColor(255,255,255),
                     lay, opt);

    QPainter painter(this);
    painter.drawImage(0,0,img);
}
