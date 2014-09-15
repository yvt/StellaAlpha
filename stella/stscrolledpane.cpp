#include "stscrolledpane.h"
#include <QPainter>
#include <QGraphicsSceneWheelEvent>

STScrolledPane::STScrolledPane(QGraphicsItem *parent) :
    STPane(parent)
{
    m_contentsView=new QGraphicsRectItem(this);
    m_contentsView->setFlag(ItemHasNoContents);

    m_scrollbar=new STScrollBarView(this);

    connect(m_scrollbar, SIGNAL(valueChanged()),
            this, SLOT(scrollChanged()));

    setFlag(ItemClipsChildrenToShape);
}

void STScrolledPane::scrollChanged(){
    relayout();
}

void STScrolledPane::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *){
    painter->fillRect(boundingRect(), QColor(70,70,70));
}

void STScrolledPane::relayout(){
    m_contentsView->setRect(0.f, 0.f, contentsWidth(), m_contentsView->rect().height());
    m_contentsView->setPos(0.f, -m_scrollbar->value());
    relayoutContents();
    m_scrollbar->setPos((float)(size().width()-15)+.01f, -1);
    m_scrollbar->setSize(QSize(16, size().height()+2));

    double scrollMax=(double)m_contentsView->rect().height()-(double)size().height();
    if(scrollMax<=0.)
        scrollMax=0.;
    m_scrollbar->setRange(0., scrollMax);
    m_scrollbar->setLargeIncrement((double)size().height());
}

void STScrolledPane::relayoutContents(){

}

int STScrolledPane::contentsWidth(){
    return size().width()-16;
}

void STScrolledPane::setContentsHeight(float h){
    if(m_contentsView->rect().height()==h)
        return;
    if(size().width()==0)
        return;
    m_contentsView->setRect(0.f, 0.f, contentsWidth(), h);
    relayout();
}

void STScrolledPane::wheelEvent(QGraphicsSceneWheelEvent *event){
    m_scrollbar->setValue(m_scrollbar->value()-event->delta());
}

