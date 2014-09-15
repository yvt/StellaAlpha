#include "stscrollbarview.h"
#include "stscrolltrackbar.h"
#include <QPainter>
#include <QBrush>
#include <QCursor>
#include <math.h>

STScrollBarView::STScrollBarView(QGraphicsItem *parent) :
    QGraphicsObject(parent)
{
    m_size=QSize(0,0);
    m_trackBar=new STScrollTrackBar(this);
    m_style=StellaLight;
    m_minValue=0.;
    m_maxValue=100.;
    m_value=0.;
    m_largeIncrement=20.;
    m_scrolling=false;
    setCursor(Qt::ArrowCursor);
}

void STScrollBarView::setSize(const QSize &size){
    if(size==m_size)
        return;
    prepareGeometryChange();
    m_size=size;
    relayout();
    update();
}

void STScrollBarView::setStyle(const Style &style){
    if(style==m_style)
        return;
    setFlag(ItemHasNoContents, style!=StellaLight &&  style!=StellaBright);
    m_style=style;
    m_trackBar->update();
    relayout();
    update();
}

void STScrollBarView::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    if(m_style==StellaLight){
        painter->setBrush(QBrush(QColor(60,60,60)));
        painter->setPen(QColor(50,50,50));
        painter->drawRect(boundingRect().adjusted(0.f,0.f,-1.f,-1.f));
    }else if(m_style==StellaBright){
        painter->setBrush(QBrush(QColor(150,150,150)));
        painter->setPen(QColor(106,106,106));
        painter->drawRect(boundingRect().adjusted(0.f,0.f,-1.f,-1.f));
    }
}

QRectF STScrollBarView::boundingRect() const{
    return QRectF(QPointF(0,0), m_size);
}

void STScrollBarView::relayout(){
    if(isScrollable()){
        double trackBarHeight=(double)m_size.height();
        trackBarHeight*=actualLargeIncrement()/(fabs(m_maxValue-m_minValue)+actualLargeIncrement());

        double minTrackBarHeight=qMin(20., (double)m_size.height()*.2);
        if(trackBarHeight<minTrackBarHeight)trackBarHeight=minTrackBarHeight;

        trackBarHeight=round(trackBarHeight);

        double range=(double)m_size.height()-trackBarHeight;

        double pos=range*(m_value-m_minValue)/(m_maxValue-m_minValue);
        m_coordRange=range;

        m_trackBar->setSize(QSize(m_size.width(), (int)trackBarHeight));
        m_trackBar->setPos(0, (float)(int)pos);

    }else{
        m_trackBar->setSize(QSize(m_size.width(), m_size.height()));
        m_trackBar->setPos(0,0);
    }
}

void STScrollBarView::trackBarDraggedBy(const QPointF &delta){
    if(!isScrollable())
        return;

    double ov=m_value;
    double sft=(double)delta.y()/m_coordRange*(m_maxValue-m_minValue);
    double tgtValue=m_value+m_scrollOverflow+sft;
    m_value=tgtValue;
    if(m_value<m_minValue)
        m_value=m_minValue;
    if(m_value>m_maxValue)
        m_value=m_maxValue;
    m_scrollOverflow=tgtValue-m_value;

    relayout();

    if(ov!=m_value){
        emit valueChanged();
        emit scrolled();
    }

}

void STScrollBarView::setRange(double minValue, double maxValue){
    setRange(minValue, maxValue, m_value);
}

void STScrollBarView::setRange(double minValue, double maxValue, double value){
    if(minValue==m_minValue && maxValue==m_maxValue && value==m_value)
        return;

    double ov=m_value;

    m_minValue=minValue;
    m_maxValue=maxValue;
    m_value=value;
    if(m_value<m_minValue)
        m_value=m_minValue;
    if(m_value>m_maxValue)
        m_value=m_maxValue;

    if(m_scrolling && m_scrollOverflow!=0.){
        double tgtValue=m_value+m_scrollOverflow;
        m_value=tgtValue;
        if(m_value<m_minValue)
            m_value=m_minValue;
        if(m_value>m_maxValue)
            m_value=m_maxValue;
        m_scrollOverflow=tgtValue-m_value;

    }

    if(!isScrollable()){
        m_scrollOverflow=0.;
    }

    relayout();
    if(m_value!=ov)
        emit valueChanged();
}

double STScrollBarView::actualLargeIncrement() const{
    return m_largeIncrement;
}

bool STScrollBarView::isScrollable() const{
    return m_minValue!=m_maxValue;
}

void STScrollBarView::setValue(double val){
    if(val==m_value)
        return;
    m_value=val;
    if(m_value<m_minValue)
        m_value=m_minValue;
    if(m_value>m_maxValue)
        m_value=m_maxValue;

    if(m_scrolling && m_scrollOverflow!=0.){
        double tgtValue=m_value+m_scrollOverflow;
        m_value=tgtValue;
        if(m_value<m_minValue)
            m_value=m_minValue;
        if(m_value>m_maxValue)
            m_value=m_maxValue;
        m_scrollOverflow=tgtValue-m_value;

    }

    relayout();
    emit valueChanged();
}

void STScrollBarView::setLargeIncrement(double v){
    if(v==m_largeIncrement) return;
    m_largeIncrement=v;
    relayout();
}
