#include "stbuttongraphicsview.h"
#include <QBrush>
#include <QLinearGradient>
#include <QPainter>
#include "stfont.h"
#include <QPixmapCache>
#include <QPainterPath>

STButtonGraphicsView::STButtonGraphicsView(STButtonView *parent,
                                           STButtonView::ButtonState state):
    QGraphicsItem(parent), m_size(0,0), m_state(state)
{
    setCacheMode(DeviceCoordinateCache);
    setAcceptedMouseButtons(Qt::NoButton);
}

void STButtonGraphicsView::paint(QPainter *outPainter, const QStyleOptionGraphicsItem *, QWidget *){
    STButtonView *button=dynamic_cast<STButtonView *>(this->parentItem());
    Q_ASSERT(button!=NULL);

    if(button->buttonType()==STButtonView::SidebarButton){

        QImage image(m_size.width(), m_size.height(), QImage::Format_RGB32);
        QPainter painter(&image);

        QLinearGradient bgGrad(0.f, 0.f, m_size.width(),0);

        painter.setPen(QColor(60, 60, 60));
        if(m_state==STButtonView::Normal){
            bgGrad.setColorAt(0, QColor(106,106,106));
            bgGrad.setColorAt(0.9, QColor(96,96,96));
            bgGrad.setColorAt(1, QColor(86,86,86));
        }else if(m_state==STButtonView::Hover){
            bgGrad.setColorAt(0.f, QColor(115, 115, 115));
            bgGrad.setColorAt(1.f, QColor(105, 105, 105));
        }else if(m_state==STButtonView::Pressed){
            bgGrad.setColorAt(0.f, QColor(80, 80, 80));
            bgGrad.setColorAt(1.f, QColor(65, 65, 65));
        }else if(m_state==STButtonView::Checked){
            bgGrad.setColorAt(0.f, QColor(90, 90, 90));
            bgGrad.setColorAt(1.f, QColor(75, 75, 75));
        }


        painter.setBrush(bgGrad);
        painter.drawRect(0, 0, m_size.width()-1, m_size.height()-1);

        if(m_state==STButtonView::Normal){
            QLinearGradient shine(0.f, 0.f, 0.f, m_size.height());
            shine.setColorAt(0.f, QColor(255, 255, 255, 30));
            shine.setColorAt(1.f, QColor(255, 255, 255, 15));
            painter.fillRect(1, 1, m_size.width()-2, 1, shine);
            painter.fillRect(1, m_size.height()-2, m_size.width()-2, 1, shine);
            painter.fillRect(1, 2, 1, m_size.height()-4, shine);
            painter.fillRect(m_size.width()-2, 2, 1, m_size.height()-4, shine);

        }else if(m_state==STButtonView::Hover){
            QLinearGradient shine(0.f, 0.f, 0.f, m_size.height());
            shine.setColorAt(0.f, QColor(255, 255, 255, 45));
            shine.setColorAt(1.f, QColor(255, 255, 255, 30));
            painter.fillRect(1, 1, m_size.width()-2, 1, shine);
            painter.fillRect(1, m_size.height()-2, m_size.width()-2, 1, shine);
            painter.fillRect(1, 2, 1, m_size.height()-4, shine);
            painter.fillRect(m_size.width()-2, 2, 1, m_size.height()-4, shine);

            QLinearGradient shine2(0.f, 0.f, 0.f, m_size.height());
            shine2.setColorAt(0.f, QColor(255, 255, 255, 10));
            shine2.setColorAt(1.f, QColor(255, 255, 255, 4));
            painter.fillRect(2, 2, m_size.width()-4, 1, shine2);
            painter.fillRect(2, m_size.height()-3, m_size.width()-4, 1, shine2);
            painter.fillRect(2, 3, 1, m_size.height()-6, shine2);
            painter.fillRect(m_size.width()-3, 3, 1, m_size.height()-6, shine2);
        }else if(m_state==STButtonView::Pressed){
            QLinearGradient shine(0.f, 0.f, 0.f, m_size.height());
            shine.setColorAt(0.f, QColor(0, 0, 0, 70));
            shine.setColorAt(1.f, QColor(0, 0, 0, 40));
            painter.fillRect(1, 1, m_size.width()-2, 1, shine);
            painter.fillRect(1, m_size.height()-2, m_size.width()-2, 1, shine);
            painter.fillRect(1, 2, 1, m_size.height()-4, shine);
            painter.fillRect(m_size.width()-2, 2, 1, m_size.height()-4, shine);

            QLinearGradient shine2(0.f, 0.f, 0.f, m_size.height());
            shine2.setColorAt(0.f, QColor(0, 0, 0, 30));
            shine2.setColorAt(1.f, QColor(0, 0, 0,20));
            painter.fillRect(2, 2, m_size.width()-4, 1, shine2);
            painter.fillRect(2, m_size.height()-3, m_size.width()-4, 1, shine2);
            painter.fillRect(2, 3, 1, m_size.height()-6, shine2);
            painter.fillRect(m_size.width()-3, 3, 1, m_size.height()-6, shine2);
        }else if(m_state==STButtonView::Checked){
            QLinearGradient shine(0.f, 0.f, 0.f, m_size.height());
            shine.setColorAt(0.f, QColor(0, 0, 0, 50));
            shine.setColorAt(1.f, QColor(0, 0, 0, 30));
            painter.fillRect(1, 1, m_size.width()-2, 1, shine);
            painter.fillRect(1, m_size.height()-2, m_size.width()-2, 1, shine);
            painter.fillRect(1, 2, 1, m_size.height()-4, shine);
            painter.fillRect(m_size.width()-2, 2, 1, m_size.height()-4, shine);

            QLinearGradient shine2(0.f, 0.f, 0.f, m_size.height());
            shine2.setColorAt(0.f, QColor(0, 0, 0, 20));
            shine2.setColorAt(1.f, QColor(0, 0, 0,10));
            painter.fillRect(2, 2, m_size.width()-4, 1, shine2);
            painter.fillRect(2, m_size.height()-3, m_size.width()-4, 1, shine2);
            painter.fillRect(2, 3, 1, m_size.height()-6, shine2);
            painter.fillRect(m_size.width()-3, 3, 1, m_size.height()-6, shine2);
        }

        QString text=button->text();

        if(text.startsWith(":")){
            // image button
            QPixmap pix;
            if(!QPixmapCache::find(text, &pix)){
                pix.load(text);
                QPixmapCache::insert(text, pix);
            }

            painter.drawPixmap((m_size.width()-pix.width())/2,
                              (m_size.height()-pix.height())/2,
                              pix);

        }else if(!text.isEmpty()){

            STFont *font=STFont::defaultFont();
            STFont::Layout layout=font->layoutString(text, m_size.width()-4);
            QRectF fontRect=font->boundingRectForLayout(layout);
            QPointF pos(((float)m_size.width()-fontRect.right())*0.5f,
                        ((float)m_size.height()-fontRect.bottom())*0.5f+1.f);

            font->drawLayout(image, QPointF(pos.x(), pos.y()-1.f), QColor(60, 60, 60, 200),
                             layout, STFont::DrawOption());

            font->drawLayout(image, pos, QColor(240,240,240),
                             layout, STFont::DrawOption());

        }

        outPainter->drawImage(0,0,image);


    }else if(button->isBack()){

        QImage image(m_size.width(), m_size.height(), QImage::Format_ARGB32_Premultiplied);
        image.fill(0);
        QPainter painter(&image);
        QPixmap backImage;
        static QPixmapCache::Key backImageKey;
        if(!QPixmapCache::find(backImageKey, &backImage)){
            backImage.load(":/stella/res/BackButton.png");
            backImageKey=QPixmapCache::insert(backImage);
        }


        QString text=button->text();
        STFont *font=STFont::defaultFont();
        STFont::Layout lay=font->layoutString(text, m_size.width()-20, true);
        QRectF rt=font->boundingRectForLayout(lay);
        QPointF pt((m_size.width()-5-rt.right())*.5f, (m_size.height()-rt.bottom())*.5f+1.f);
        font->drawLayout(image, pt+QPointF(0,1), QColor(255,255,255,128), lay, STFont::DrawOption());
        font->drawLayout(image, pt, QColor(0,0,0,200), lay, STFont::DrawOption());

        QPoint pos(m_size.width()-backImage.width(), (m_size.height()-backImage.height())/2);

        if(m_state==STButtonView::Pressed){
            QLinearGradient bgGrad(0.f, 0.f, 0.f, m_size.height());
            bgGrad.setColorAt(0, QColor(0,0,0,8));
            bgGrad.setColorAt(0.5, QColor(0,0,0,16));
            bgGrad.setColorAt(1, QColor(0,0,0,8));

            QPainterPath path;
            path.moveTo(0,0);
            path.lineTo(pos.x()+2, 0);
            path.lineTo(pos.x()+backImage.width(), pos.y()+backImage.height()/2);
            path.lineTo(pos.x()+2, m_size.height());
            path.lineTo(0, m_size.height());
            path.closeSubpath();

            painter.save();
            painter.setRenderHint(QPainter::Antialiasing);
            painter.setRenderHint(QPainter::HighQualityAntialiasing);
            painter.fillPath(path, bgGrad);
            painter.restore();
        }

        painter.drawPixmap(pos, backImage);

        outPainter->drawImage(0,0,image);

    }else{

        QImage image(m_size.width(), m_size.height(), QImage::Format_RGB32);
        QPainter painter(&image);

        QLinearGradient bgGrad(0.f, 0.f, 0.f, m_size.height());


        if(button->isFlat()){
            if(m_state==STButtonView::Normal){
                bgGrad.setColorAt(0.f, QColor(70,70,70));
                painter.setPen(QColor(60, 60, 60));
            }else if(m_state==STButtonView::Hover){
                bgGrad.setColorAt(0.f, QColor(95,95,95));
                bgGrad.setColorAt(1.f, QColor(85,85,85));
                painter.setPen(QColor(60, 60, 60));
            }else if(m_state==STButtonView::Pressed){
                bgGrad.setColorAt(0.f, QColor(75,75,75));
                bgGrad.setColorAt(1.f, QColor(65, 65, 65));
                painter.setPen(QColor(60, 60, 60));
            }else if(m_state==STButtonView::Checked){
                bgGrad.setColorAt(0.f, QColor(70,70,70));
                bgGrad.setColorAt(1.f, QColor(65,65,65));
                painter.setPen(QColor(60, 60, 60));
            }
        }else{
            if(m_state==STButtonView::Normal){
                bgGrad.setColorAt(0.f, QColor(110, 110, 110));
                bgGrad.setColorAt(1.f, QColor(100, 100, 100));
                painter.setPen(QColor(60, 60, 60));
            }else if(m_state==STButtonView::Hover){
                bgGrad.setColorAt(0.f, QColor(115, 115, 115));
                bgGrad.setColorAt(1.f, QColor(105, 105, 105));
                painter.setPen(QColor(60, 60, 60));
            }else if(m_state==STButtonView::Pressed){
                bgGrad.setColorAt(0.f, QColor(80, 80, 80));
                bgGrad.setColorAt(1.f, QColor(65, 65, 65));
                painter.setPen(QColor(60, 60, 60));
            }else if(m_state==STButtonView::Checked){
                bgGrad.setColorAt(0.f, QColor(90, 90, 90));
                bgGrad.setColorAt(1.f, QColor(75, 75, 75));
                painter.setPen(QColor(60, 60, 60));
            }
        }

        painter.setBrush(bgGrad);
        painter.drawRect(0, 0, m_size.width()-1, m_size.height()-1);

        if(m_state==STButtonView::Normal){
            if(!button->isFlat()){
                QLinearGradient shine(0.f, 0.f, 0.f, m_size.height());
                shine.setColorAt(0.f, QColor(255, 255, 255, 30));
                shine.setColorAt(1.f, QColor(255, 255, 255, 15));
                painter.fillRect(1, 1, m_size.width()-2, 1, shine);
                painter.fillRect(1, m_size.height()-2, m_size.width()-2, 1, shine);
                painter.fillRect(1, 2, 1, m_size.height()-4, shine);
                painter.fillRect(m_size.width()-2, 2, 1, m_size.height()-4, shine);
            }
        }else if(m_state==STButtonView::Hover){
            QLinearGradient shine(0.f, 0.f, 0.f, m_size.height());
            shine.setColorAt(0.f, QColor(255, 255, 255, 45));
            shine.setColorAt(1.f, QColor(255, 255, 255, 30));
            painter.fillRect(1, 1, m_size.width()-2, 1, shine);
            painter.fillRect(1, m_size.height()-2, m_size.width()-2, 1, shine);
            painter.fillRect(1, 2, 1, m_size.height()-4, shine);
            painter.fillRect(m_size.width()-2, 2, 1, m_size.height()-4, shine);

            QLinearGradient shine2(0.f, 0.f, 0.f, m_size.height());
            shine2.setColorAt(0.f, QColor(255, 255, 255, 10));
            shine2.setColorAt(1.f, QColor(255, 255, 255, 4));
            painter.fillRect(2, 2, m_size.width()-4, 1, shine2);
            painter.fillRect(2, m_size.height()-3, m_size.width()-4, 1, shine2);
            painter.fillRect(2, 3, 1, m_size.height()-6, shine2);
            painter.fillRect(m_size.width()-3, 3, 1, m_size.height()-6, shine2);
        }else if(m_state==STButtonView::Pressed){
            QLinearGradient shine(0.f, 0.f, 0.f, m_size.height());
            shine.setColorAt(0.f, QColor(0, 0, 0, 70));
            shine.setColorAt(1.f, QColor(0, 0, 0, 40));
            painter.fillRect(1, 1, m_size.width()-2, 1, shine);
            painter.fillRect(1, m_size.height()-2, m_size.width()-2, 1, shine);
            painter.fillRect(1, 2, 1, m_size.height()-4, shine);
            painter.fillRect(m_size.width()-2, 2, 1, m_size.height()-4, shine);

            QLinearGradient shine2(0.f, 0.f, 0.f, m_size.height());
            shine2.setColorAt(0.f, QColor(0, 0, 0, 30));
            shine2.setColorAt(1.f, QColor(0, 0, 0,20));
            painter.fillRect(2, 2, m_size.width()-4, 1, shine2);
            painter.fillRect(2, m_size.height()-3, m_size.width()-4, 1, shine2);
            painter.fillRect(2, 3, 1, m_size.height()-6, shine2);
            painter.fillRect(m_size.width()-3, 3, 1, m_size.height()-6, shine2);
        }else if(m_state==STButtonView::Checked){
            QLinearGradient shine(0.f, 0.f, 0.f, m_size.height());
            shine.setColorAt(0.f, QColor(0, 0, 0, 50));
            shine.setColorAt(1.f, QColor(0, 0, 0, 30));
            painter.fillRect(1, 1, m_size.width()-2, 1, shine);
            painter.fillRect(1, m_size.height()-2, m_size.width()-2, 1, shine);
            painter.fillRect(1, 2, 1, m_size.height()-4, shine);
            painter.fillRect(m_size.width()-2, 2, 1, m_size.height()-4, shine);

            QLinearGradient shine2(0.f, 0.f, 0.f, m_size.height());
            shine2.setColorAt(0.f, QColor(0, 0, 0, 20));
            shine2.setColorAt(1.f, QColor(0, 0, 0,10));
            painter.fillRect(2, 2, m_size.width()-4, 1, shine2);
            painter.fillRect(2, m_size.height()-3, m_size.width()-4, 1, shine2);
            painter.fillRect(2, 3, 1, m_size.height()-6, shine2);
            painter.fillRect(m_size.width()-3, 3, 1, m_size.height()-6, shine2);
        }

        QString text=button->text();

        if(text.startsWith(":")){
            // image button
            QPixmap pix;
            if(!QPixmapCache::find(text, &pix)){
                pix.load(text);
                QPixmapCache::insert(text, pix);
            }

            painter.drawPixmap((m_size.width()-pix.width())/2,
                              (m_size.height()-pix.height())/2,
                              pix);

        }else if(!text.isEmpty()){

            STFont *font=STFont::defaultFont();
            STFont::Layout layout=font->layoutString(text, m_size.width()-4);
            QRectF fontRect=font->boundingRectForLayout(layout);
            QPointF pos(((float)m_size.width()-fontRect.right())*0.5f,
                        ((float)m_size.height()-fontRect.bottom())*0.5f+1.f);

            font->drawLayout(image, QPointF(pos.x(), pos.y()-1.f), QColor(60, 60, 60, 200),
                             layout, STFont::DrawOption());

            font->drawLayout(image, pos, QColor(240,240,240),
                             layout, STFont::DrawOption());

        }

        outPainter->drawImage(0,0,image);

    }

}

QRectF STButtonGraphicsView::boundingRect() const{
    return QRectF(0.f, 0.f, m_size.width(), m_size.height());
}

