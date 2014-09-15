#include "stnewsshadowview.h"
#include <QLinearGradient>
#include <QBrush>
#include <QPainter>

static const int m_shadowHeight=20;

STNewsShadowView::STNewsShadowView(QGraphicsItem *parent):QGraphicsItem(parent)
{
    setCacheMode(DeviceCoordinateCache);
    setAcceptedMouseButtons(Qt::NoButton);
}

void STNewsShadowView::setWidth(int width){
    prepareGeometryChange();
    m_width=width;
}

void STNewsShadowView::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){

    QLinearGradient grad(0.f, 0.f, 0.f, m_shadowHeight);
    grad.setColorAt(0.f, QColor(0,0,0,70));
    grad.setColorAt(0.2f, QColor(0,0,0,23));
    grad.setColorAt(1.f, QColor(0,0,0,0));
    //grad.setColorAt(1.f, QColor(0,0,0));
    painter->fillRect(QRect(0, 1, m_width, m_shadowHeight-1), QBrush(grad));
    painter->fillRect(QRect(0, 0, m_width, 1),
                      QColor(60,60,60));

}

QRectF STNewsShadowView::boundingRect() const{
    return QRectF(0.f,0.f,m_width,m_shadowHeight);
}

