#include "stheaderview.h"
#include <QLinearGradient>
#include <QPainter>
#include "stfont.h"
#include <QDateTime>
#include <QDebug>
#include "stmath.h"
#include "stbuttonview.h"
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QFontMetrics>

STHeaderView::STHeaderView(QGraphicsItem *parent) :
    QGraphicsObject(parent),m_userBarWidth(0)
{
    m_pixmapItem=new QGraphicsPixmapItem(this);
    m_oldPixmapItem=new QGraphicsPixmapItem(this);
    m_contents.title="Oops! No Title!";

    m_backButton=new STButtonView(this);
    m_backButton->setOpacity(0);
    m_backButton->setText("Back");
    m_backButton->setBack(true);

    m_backButtonOld=new STButtonView(this);
    m_backButtonOld->setOpacity(0);
    m_backButtonOld->setText("Back");
    m_backButtonOld->setBack(true);

    m_backButton->setZValue(1.f);
    m_backButtonOld->setZValue(1.f);

    m_animationDuration=200;
    m_animationSlideDistance=100.f;
    m_animating=false;

    connect(&m_timer, SIGNAL(timeout()),
            this, SLOT(timerFired()));

    connect(m_backButton, SIGNAL(activated()),
            this, SLOT(backButtonActivatedSlot()));
    connect(m_backButtonOld, SIGNAL(activated()),
            this, SLOT(backButtonActivatedSlot()));

    setFlag(ItemIsFocusable);
    setCacheMode(DeviceCoordinateCache);

    m_drag=false;
}

void STHeaderView::relayout(){
    m_backButton->setSize(QSize(80, m_size.height()+1));
    m_backButton->setPos(-1.f, -1.f);
    m_backButtonOld->setSize(QSize(80, m_size.height()+1));
    m_backButtonOld->setPos(-1.f, -1.f);



    if(m_animating){
        float per=(float)(QDateTime::currentMSecsSinceEpoch()-m_animationStartTime)/(float)m_animationDuration;
        if(per>1.f){
            m_timer.stop();
            m_animating=false;
        }
        per=STSmoothStep(per);

        float oldShiftX=per*m_animationSlideDistance;
        float shiftX;
        if(m_transitionType==SlideToLeft){
            oldShiftX=-oldShiftX;
            shiftX=oldShiftX+m_animationSlideDistance;
        }else if(m_transitionType==Dissolve){
            oldShiftX=shiftX=0.f;
        }else if(m_transitionType==SlideToRight){
            shiftX=oldShiftX-m_animationSlideDistance;
        }


        m_pixmapItem->setOpacity(per);
        m_oldPixmapItem->setOpacity(1.f-per);
        m_pixmapItem->setPos(shiftX,0);
        m_oldPixmapItem->setPos(oldShiftX,0);

        m_backButton->setOpacity((1.f-per)*(m_oldContents.hasBackButton?0.f:0.f)+
                                 per*(m_contents.hasBackButton?1.f:0.f));
        m_backButtonOld->setOpacity((1.f-per)*(m_oldContents.hasBackButton?1.f:0.f)+
                                 per*(m_contents.hasBackButton?0.f:0.f));

    }else{
        m_pixmapItem->setOpacity(1.f);
        m_oldPixmapItem->setOpacity(0.f);
        m_pixmapItem->setPos(0.f,0.f);
        m_backButton->setOpacity((m_contents.hasBackButton?1.f:0.f));
        m_backButtonOld->setOpacity(0);
    }
}

QRectF STHeaderView::boundingRect() const{
    return QRectF(0.f, 0.f,
                  m_size.width(), m_size.height());
}


#define BRIGHT 1

void STHeaderView::paint(QPainter *outPainter, const QStyleOptionGraphicsItem *, QWidget *){
    QRectF boundRect(0,0,m_size.width(),m_size.height());
    if(boundRect.isEmpty())
        return;

    /*
    QImage image(m_size.width(),m_size.height(),QImage::Format_RGB32);

    image.fill(0);*/

    //QPainter painter(&image);
    QPainter& painter=*outPainter;
    QLinearGradient grad(0.f, 0.f, 0.f, boundRect.height());
#if BRIGHT
    grad.setColorAt(0.f, QColor(241,241,241));
    grad.setColorAt(0.02f, QColor(229,229,229));
    grad.setColorAt(0.5f, QColor(200,200,200));
    grad.setColorAt(0.95f, QColor(180,180,180));
    grad.setColorAt(1.f, QColor(190,190,190));
#else
    grad.setColorAt(0.f, QColor(140, 140, 140));
    grad.setColorAt(1.f, QColor(110, 110, 110));
#endif
    //

    /*
    ;*/

    painter.fillRect(QRect(0, 0, m_size.width(), m_size.height()-1), QBrush(grad));

    /*
    painter.fillRect(QRect(0, m_size.height()-1,m_userBarWidth, 1),
                      QColor(106,106,106));
    painter.fillRect(QRect(m_userBarWidth, m_size.height()-1, m_size.width()-m_userBarWidth, 1),
                      QColor(60,60,60));*/
    painter.fillRect(QRect(0, m_size.height()-1, m_size.width(), 1),
                      QColor(60,60,60)); // dark sidebar

    /*
    QLinearGradient shine(0.f, 0.f, 0.f, m_size.height());
    shine.setColorAt(0.f, QColor(255, 255, 255, 13));
    shine.setColorAt(1.f, QColor(255, 255, 255, 6));
    painter.fillRect(QRect(0, m_size.height()-2, m_size.width(), 1),
                      shine);
    painter.fillRect(QRect(0, 0, m_size.width(), 1),
                      shine);
    painter.fillRect(QRect(0, 1,1, m_size.height()-3),
                      shine);
    painter.fillRect(QRect(m_size.width()-1, 1,1, m_size.height()-3),
                      shine);

*/


   // outPainter->drawImage(0,0,image);
}

void STHeaderView::renderContents(){
    QRectF boundRect(0,0,m_size.width(),m_size.height());
    if(boundRect.isEmpty())
        return;
    QPixmap image(m_size.width(),m_size.height());

    image.fill(QColor(0,0,0,0));

    QPainter painter(&image);

    drawContents(painter, image, 0.f, 1.f, m_contents);

    m_pixmapItem->setPixmap(image);
}

void STHeaderView::drawContents(QPainter& painter, QPixmap& image, float shiftX, float opacity, Contents& contents){
    float maxWidth=(float)image.width();
    if(contents.hasBackButton)
        maxWidth-=80.f;
    maxWidth-=4.f;

    float minX=0.f;
    if(contents.hasBackButton)
        minX=80.f;
    minX+=5.f;
    /*
    if(contents.hasBackButton)
        shiftX+=50.f;*/

    painter.save();
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::HighQualityAntialiasing);
    {
        QPainterPath path;
        QFont font("Stella Sans");
        font.setPixelSize(16);
        font.setWeight(QFont::Bold);
        font.setStyleHint(QFont::Helvetica, QFont::ForceOutline);
        QString txt=QFontMetrics(font).elidedText(contents.title, Qt::ElideRight, (int)maxWidth-8);
        path.addText(0,0,font,txt);
        shiftX+=((float)image.width()-path.boundingRect().right())*.5f;
        if(shiftX<minX)shiftX=minX;

        painter.translate(shiftX, (image.height()-14)/2+14);

        painter.fillPath(path, QBrush(QColor(255,255,255,(int)(opacity*64.f))));
        painter.translate(0,-1);
        painter.fillPath(path, QBrush(QColor(70,70,70, (int)(opacity*255.f))));
    }
    painter.restore();

}

void STHeaderView::setContents(const Contents &c){
    m_contents=c;
    m_animating=false;
    m_backButton->setText(c.backButtonText);
    m_timer.stop();
    renderContents();
    relayout();;
}

void STHeaderView::setContentsAnimated(const Contents &c, TransitionType type){
    m_oldContents=m_contents;
    m_contents=c;
    qSwap(m_oldPixmapItem,m_pixmapItem);
    qSwap(m_backButton, m_backButtonOld);
    m_backButtonOld->setText(m_oldContents.backButtonText);
    m_backButton->setText(c.backButtonText);
    m_transitionType=type;
    m_animationStartTime=QDateTime::currentMSecsSinceEpoch();
    m_animating=true;
    relayout();
    renderContents();
    m_timer.start();
}

void STHeaderView::timerFired(){
    relayout();
}

void STHeaderView::backButtonActivatedSlot(){
    emit backButtonActivated();
}

void STHeaderView::mousePressEvent(QGraphicsSceneMouseEvent *event){
#if QT_MAC_USE_COCOA
    // in mac, dragging window becomes too laggy!
#else
    if(event->button()==Qt::LeftButton){
        m_oldDragPos=event->screenPos();
        m_drag=true;
        grabMouse();
    }
#endif
}

void STHeaderView::mouseMoveEvent(QGraphicsSceneMouseEvent *event){
    if(m_drag){
        QPoint shift=event->screenPos()-m_oldDragPos;
        m_oldDragPos=event->screenPos();

        QGraphicsView *vw=this->scene()->views()[0];
        QWidget *wid=vw->topLevelWidget();
        wid->move(wid->pos()+shift);
    }


}

void STHeaderView::mouseReleaseEvent(QGraphicsSceneMouseEvent *){
    ungrabMouse();
    m_drag=false;
}
