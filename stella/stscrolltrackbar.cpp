#include "stscrolltrackbar.h"
#include "stscrollbarview.h"
#include <QPainter>
#include <QGraphicsSceneMouseEvent>

STScrollTrackBar::STScrollTrackBar(STScrollBarView *parent):
    QGraphicsItem(parent),
    m_size(0,0)
{
    setCacheMode(DeviceCoordinateCache);
    setAcceptsHoverEvents(true);
    m_hover=false;
    m_drag=false;
}

void STScrollTrackBar::setSize(const QSize &newsize){
    if(newsize==m_size)return;
    prepareGeometryChange();
    m_size=newsize;
    update();
}

void STScrollTrackBar::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    STScrollBarView::Style style=dynamic_cast<STScrollBarView *>(parentItem())->m_style;
    if(style==STScrollBarView::StellaLight){
        if(m_drag){
            painter->setBrush(QBrush(QColor(70, 70, 70)));
        }else if(m_hover){
            painter->setBrush(QBrush(QColor(110,110,110)));
        }else{
            painter->setBrush(QBrush(QColor(90,90,90)));
        }
        painter->setPen(QColor(50,50,50));
        painter->drawRect(boundingRect().adjusted(0.f,0.f,-1.f,-1.f));

        QLinearGradient shine(0.f, 0.f, 0.f, m_size.height());
        if(m_drag){
            shine.setColorAt(0.f, QColor(0,0,0, 20));
            shine.setColorAt(1.f, QColor(0,0,0, 10));
        }else{
            shine.setColorAt(0.f, QColor(255, 255, 255, 13));
            shine.setColorAt(1.f, QColor(255, 255, 255, 6));
        }
        painter->fillRect(1, 1, m_size.width()-2, 1, shine);
        painter->fillRect(1, m_size.height()-2, m_size.width()-2, 1, shine);
        painter->fillRect(1, 2, 1, m_size.height()-4, shine);
        painter->fillRect(m_size.width()-2, 2, 1, m_size.height()-4, shine);

    }else if(style==STScrollBarView::StellaBright){
        if(m_drag){
            painter->setBrush(QBrush(QColor(160,160,160)));
        }else if(m_hover){
            painter->setBrush(QBrush(QColor(190,190,190)));
        }else{
            painter->setBrush(QBrush(QColor(170,170,170)));
        }
        painter->setPen(QColor(106,106,106));
        painter->drawRect(boundingRect().adjusted(0.f,0.f,-1.f,-1.f));

        QLinearGradient shine(0.f, 0.f, 0.f, m_size.height());
        if(m_drag){
            shine.setColorAt(0.f, QColor(0,0,0, 20));
            shine.setColorAt(1.f, QColor(0,0,0, 10));
        }else{
            shine.setColorAt(0.f, QColor(255, 255, 255, 13));
            shine.setColorAt(1.f, QColor(255, 255, 255, 6));
        }
        painter->fillRect(1, 1, m_size.width()-2, 1, shine);
        painter->fillRect(1, m_size.height()-2, m_size.width()-2, 1, shine);
        painter->fillRect(1, 2, 1, m_size.height()-4, shine);
        painter->fillRect(m_size.width()-2, 2, 1, m_size.height()-4, shine);

    }else if(style==STScrollBarView::ClearBlack){
        QBrush br;
        if(m_drag){
            br=(QBrush(QColor(0,0,0,160)));
        }else if(m_hover){
            br=(QBrush(QColor(0,0,0,96)));
        }else{
            br=(QBrush(QColor(0,0,0,64)));
        }
        painter->fillRect(boundingRect(), br);
    }else if(style==STScrollBarView::ClearWhite){
        QBrush br;
        if(m_drag){
            br=(QBrush(QColor(255,255,255,64)));
        }else if(m_hover){
            br=(QBrush(QColor(255,255,255,96)));
        }else{
            br=(QBrush(QColor(255,255,255,160)));
        }
        painter->fillRect(boundingRect(), br);
    }
}

QRectF STScrollTrackBar::boundingRect() const{
    return QRectF(0,0,m_size.width(),m_size.height());
}

void STScrollTrackBar::mousePressEvent(QGraphicsSceneMouseEvent *event){
    if(event->button()!=Qt::LeftButton)
        return;
    m_drag=true;
    m_lastScrollPos=event->scenePos();

    STScrollBarView *scr=dynamic_cast<STScrollBarView *>(parentItem());
    Q_ASSERT(scr);
    scr->m_scrolling=true;
    scr->m_scrollOverflow=0.;

    update();
    grabMouse();
}

void STScrollTrackBar::mouseMoveEvent(QGraphicsSceneMouseEvent *event){
    if(m_drag){
        STScrollBarView *scr=dynamic_cast<STScrollBarView *>(parentItem());
        Q_ASSERT(scr);

        scr->trackBarDraggedBy(event->scenePos()-m_lastScrollPos);
        m_lastScrollPos=event->scenePos();
    }
}

void STScrollTrackBar::mouseReleaseEvent(QGraphicsSceneMouseEvent *event){
    STScrollBarView *scr=dynamic_cast<STScrollBarView *>(parentItem());
    Q_ASSERT(scr);
    scr->m_scrolling=false;

    m_drag=false;
    update();
    ungrabMouse();
}

void STScrollTrackBar::hoverEnterEvent(QGraphicsSceneHoverEvent *event){
    m_hover=true;
    update();
}

void STScrollTrackBar::hoverLeaveEvent(QGraphicsSceneHoverEvent *event){
    m_hover=false;
    update();
}
