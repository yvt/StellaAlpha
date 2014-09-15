#include "ststdafx.h"
#include "ststartupview.h"
#include <QPixmapCache>
#include "stlogoview.h"

STStartupView::STStartupView(QGraphicsItem *parent) :
    QGraphicsObject(parent),m_size(0,0)
{
    setCacheMode(DeviceCoordinateCache);
    m_resizeView=new STResizeView(this);
    m_logoView=new STLogoView(this);
}


static const int g_userBarWidth=40;
static const int g_headerHeight=32;
static const int g_sizeBarHeight=10;
static const int g_newsDefaultHeight=20;

void STStartupView::setSize(QSize sz){
    if(sz==m_size)return;
    prepareGeometryChange();
    m_size=sz;


    m_resizeView->setPos(0,m_size.height()-g_sizeBarHeight);
    m_resizeView->setSize(QSize(m_size.width(), g_sizeBarHeight));

    QRect contentsRect(g_userBarWidth, 1, m_size.width()-g_userBarWidth-1, m_size.height()-g_sizeBarHeight-1);
    m_logoView->setPos(contentsRect.left(), contentsRect.top()+g_headerHeight);
    m_logoView->setSize(QSize(contentsRect.width(), contentsRect.height()-g_headerHeight));
}

static void drawBrightPseudoScrollBar(QPainter *painter, QRect rt){



        painter->setBrush(QBrush(QColor(90,90,90)));

    painter->setPen(QColor(50,50,50));
    painter->drawRect(rt.adjusted(0,0,-1,-1));

    painter->save();
    painter->translate(rt.topLeft());

    QLinearGradient shine(0.f, 0.f, 0.f, rt.height());

    shine.setColorAt(0.f, QColor(255, 255, 255, 13));
    shine.setColorAt(1.f, QColor(255, 255, 255, 6));

    painter->fillRect(1, 1, rt.width()-2, 1, shine);
    painter->fillRect(1, rt.height()-2, rt.width()-2, 1, shine);
    painter->fillRect(1, 2, 1, rt.height()-4, shine);
    painter->fillRect(rt.width()-2, 2, 1, rt.height()-4, shine);

    painter->restore();
}

void STStartupView::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    if(m_size.isEmpty())return;

    painter->fillRect(0,0,m_size.width(),m_size.height(),
                      QColor(70,70,70));


    QRect contentsRect(g_userBarWidth, 1, m_size.width()-g_userBarWidth-1, m_size.height()-g_sizeBarHeight-1);



    // side bar
    {
        QRect sideRect(0,g_headerHeight+1, g_userBarWidth, m_size.height()-g_headerHeight-1-g_sizeBarHeight);
        QLinearGradient grad(0,0,g_userBarWidth,0);
        grad.setColorAt(0, QColor(70,70,70));
        grad.setColorAt(0.1f, QColor(80,80,80));
        grad.setColorAt(0.2f, QColor(85,85,85));
        grad.setColorAt(0.5f, QColor(86,86,86));
        grad.setColorAt(0.7f, QColor(85,85,85));
        grad.setColorAt(0.8f, QColor(80,80,80));
        grad.setColorAt(1, QColor(70,70,70)); // dark sidebar, more darkness


        painter->fillRect(sideRect, grad);

        QColor borderColor(60,60,60);
        QColor brightBorderColor(106,106,106);
        brightBorderColor=borderColor; // dark sidebar
        painter->fillRect(0,m_size.height()-g_sizeBarHeight,sideRect.width(), 1, brightBorderColor);
        painter->fillRect(sideRect.width(),m_size.height()-g_sizeBarHeight,m_size.width()- sideRect.width(), 1, borderColor);

        painter->fillRect(0,0,1, m_size.height(), borderColor);

        painter->fillRect(g_userBarWidth-1,0,1, m_size.height(), borderColor);

        // sidebar scroll bar
        {
            drawBrightPseudoScrollBar(painter, QRect(sideRect.right()-6, sideRect.top()-1,
                                                     7, sideRect.height()+2));
        }

    }



    // header view
    {
        QRectF boundRect(0,0,m_size.width(),33);

        QLinearGradient grad(0.f, 0.f, 0.f, 33);
        grad.setColorAt(0.f, QColor(241,241,241));
        grad.setColorAt(0.02f, QColor(229,229,229));
        grad.setColorAt(0.5f, QColor(200,200,200));
        grad.setColorAt(0.95f, QColor(180,180,180));
        grad.setColorAt(1.f, QColor(190,190,190));

        painter->fillRect(QRect(0, 0, m_size.width(), 32), QBrush(grad));
        painter->fillRect(QRect(0, 32, m_size.width(), 1),
                          QColor(60,60,60));

    }



}

QRectF STStartupView::boundingRect() const{
    return QRectF(0,0,m_size.width(),m_size.height());
}
