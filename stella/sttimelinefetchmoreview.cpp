#include "sttimelinefetchmoreview.h"
#include "stbuttonview.h"
#include "sttimeline.h"
#include <QPainter>
#include "ststripeanimationview.h"

STTimelineFetchMoreView::STTimelineFetchMoreView(QSize size,
                                                 STTimeline *timeline, quint64 id,
                                                 QGraphicsItem *parent) :
    QGraphicsObject(parent), m_size(size),m_timeline(timeline),m_id(id)
{
    m_button=new STButtonView(this);
    //m_button->setSize(QSize(140, size.height()+1));
    //m_button->setPos((size.width()-140)/2, -1);
    m_button->setSize(QSize(size.width()+2, size.height()+1));
    m_button->setPos(-1, -1);
   // m_button->setFlat(true);
    m_button->setText(":/stella/res/FetchMoreIcon.png");

    m_button->setVisible(!isFetching());

    m_animView=new STStripeAnimationView(this);
    m_animView->setSize(QSize(size.width(), size.height()-1));
    m_animView->setVisible(isFetching());

    connect(m_button, SIGNAL(activated()),
            this, SLOT(doFetch()));
    connect(m_timeline, SIGNAL(fetchRowUpdated(quint64)),
            this, SLOT(updateState(quint64)));

}

bool STTimelineFetchMoreView::isFetching(){
    STTimeline::Row *row=m_timeline->row(m_id);
    if(row)return row->fetching;
    return false;
}

QSize STTimelineFetchMoreView::defaultSize(int width){
    return QSize(width, 20);
}

void STTimelineFetchMoreView::paint(QPainter *outPainter, const QStyleOptionGraphicsItem *, QWidget *){
    QImage img(QSize(m_size.width(), m_size.height()+1), QImage::Format_RGB32);
    img.fill(0);

    QPainter painter(&img);
    painter.fillRect(QRect(0, 1, m_size.width(), m_size.height()), QColor(70,70,70));
    painter.fillRect(QRect(0, 0, m_size.width(), 1), QColor(60,60,60));
    painter.fillRect(QRect(0, m_size.height(), m_size.width(), 1), QColor(60,60,60));

    outPainter->drawImage(0,-1,img);
}

QRectF STTimelineFetchMoreView::boundingRect() const{
    return QRectF(0.f, -1, m_size.width(), m_size.height()+1);
}

void STTimelineFetchMoreView::updateState(quint64 id){
    if(id!=m_id)return;

    // TODO: animate
    m_button->setVisible(!isFetching());
    m_animView->setVisible(isFetching());
}

void STTimelineFetchMoreView::doFetch(){
    m_timeline->fetchMore(m_id);
}

