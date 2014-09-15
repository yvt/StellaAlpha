#include "stuserprofilenumberview.h"
#include <QPainterPath>
#include <QPainter>
#include <QPixmapCache>
#include <QPixmap>


STUserProfileNumberView::STUserProfileNumberView(QGraphicsItem *parent) :
    STButtonView(parent)
{
    setFlag(ItemHasNoContents, false);
    setCacheMode(DeviceCoordinateCache);

    setFlat(true);
    foreach(QGraphicsItem *item, childItems()){
        item->setFlag(ItemStacksBehindParent);
    }
}

void STUserProfileNumberView::setNumberText(QString text){
    m_numberText=text;
    update();
}

void STUserProfileNumberView::setLabelText(QString text){
    m_labelText=text;
    update();
}

void STUserProfileNumberView::paint(QPainter *outPainter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    STButtonView::paint(outPainter, option, widget);

    QSize sz=size();
    if(sz.isEmpty())
        return;
    QPixmap outPixmap(sz);
    outPixmap.fill(QColor(0,0,0,0));

    QPainter painter(&outPixmap);
    int numberRightPos;

    painter.save();
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::HighQualityAntialiasing);
    {
        QPainterPath path;
        QFont font("Stella Sans");
        font.setPixelSize(18);
        font.setWeight(QFont::Normal);
        font.setStyleHint(QFont::Helvetica, QFont::ForceOutline);
        path.addText(0,0,font, m_numberText);

        float wid=path.boundingRect().right();
        float maxWid=(float)(size().width()-12);
        if(wid>maxWid)
            painter.scale(maxWid/wid, maxWid/wid);

        painter.translate(6, 22);

        painter.fillPath(path, QBrush(QColor(200,200,200)));
        numberRightPos=(int)ceilf(path.boundingRect().right())+6;
    }
    painter.restore();

    if(!m_labelText.isEmpty()){

        QPixmap pix;
        if(!QPixmapCache::find(m_labelText, &pix)){
            pix.load(m_labelText);
            QPixmapCache::insert(m_labelText, pix);
        }

        Q_ASSERT(!pix.isNull());

        if(size().height()>=40){
            painter.drawPixmap(6, 28, pix);
        }else{
            painter.drawPixmap(numberRightPos+5, 22-pix.height(), pix);
        }
    }

    outPainter->drawPixmap(0,0,outPixmap);

}
