#include "stuserlistfetchmoreview.h"
#include "stbuttonview.h"
#include "ststripeanimationview.h"

STUserListFetchMoreView::STUserListFetchMoreView(QSize size, QGraphicsItem *parent) :
    QGraphicsObject(parent), m_size(size)
{
    m_buttonView=new STButtonView(this);
    m_animationView=new STStripeAnimationView(this);

    //m_buttonView->setSize(QSize(140, 20+1));
    //m_buttonView->setPos((size.width()-140)/2, -1);
    m_buttonView->setSize(QSize(size.width()+2, 20+1));
    m_buttonView->setPos(-1, -1);
    //m_buttonView->setFlat(true);
    m_buttonView->setText(":/stella/res/FetchMoreIcon.png");

    m_animationView->setSize(QSize(size.width(), 20-1));

    connect(m_buttonView, SIGNAL(activated()),
            this, SLOT(doFetch()));

    setCacheMode(DeviceCoordinateCache);
}

void STUserListFetchMoreView::doFetch(){
    emit fetchMoreRequested();
}

void STUserListFetchMoreView::paint(QPainter *outPainter, const QStyleOptionGraphicsItem *, QWidget *){
    QImage img(QSize(m_size.width(), m_size.height()+1), QImage::Format_RGB32);
    img.fill(0);

    QPainter painter(&img);
    painter.fillRect(QRect(0, 1, m_size.width(), m_size.height()+1), QColor(70,70,70));
    painter.fillRect(QRect(0, 0, m_size.width(), 1), QColor(60,60,60));
    painter.fillRect(QRect(0,20, m_size.width(), 1), QColor(60,60,60));

    outPainter->drawImage(0,-1,img);
}

QRectF STUserListFetchMoreView::boundingRect() const{
    return QRectF(0,-1,m_size.width(),m_size.height()+1);
}

void STUserListFetchMoreView::setFetching(bool b){
    m_animationView->setVisible(b);
}
