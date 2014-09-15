#include "sttimelineitemtimeview.h"
#include "stonesectimer.h"
#include "stfont.h"
#include <QPainter>
#include <QLinearGradient>
#include <QBrush>
#include "stbuttonview.h"
#include "stmath.h"
#include "stdateformatter.h"

STTimelineItemTimeView::STTimelineItemTimeView(QDateTime dt, QGraphicsItem *parent) :
    QGraphicsObject(parent), m_dateTime(dt), m_absView(false)
{
    setAcceptedMouseButtons(Qt::NoButton);
    setCacheMode(DeviceCoordinateCache);

    m_timer=new STOneSecTimer(this);
    connect(m_timer, SIGNAL(timeout()),
            this, SLOT(timerFired()));

    m_backgroundColor=QColor(70,70,70);

    m_width=100;
    updateText();
    m_timer->setActive(!m_absView);


}

QVariant STTimelineItemTimeView::itemChange(GraphicsItemChange change, const QVariant &value){
    if(change==ItemVisibleHasChanged){
        m_timer->setActive(isVisible()&&(!m_absView));
    }
    return value;
}

void STTimelineItemTimeView::timerFired(){
    updateText();
}

void STTimelineItemTimeView::setBackgroundColor(const QColor &col){
    m_backgroundColor=col;
    update();
}

void STTimelineItemTimeView::paint(QPainter *outPainter, const QStyleOptionGraphicsItem *, QWidget *){
    QImage img(QSize(m_width-20, 13), QImage::Format_RGB32);
    img.fill(m_backgroundColor);

    STFont *font=STFont::defaultFont();
    STFont::Layout layout=font->layoutString(m_text);
    QRectF bnd=font->boundingRectForLayout(layout);

    font->drawLayout(img, QPointF((float)(m_width-26)-bnd.right(), 0.f),
                     QColor(200,200,200, 160),
                     layout,
                     STFont::DrawOption());

    QLinearGradient grad(-m_width, 0,
                         -(m_width-20), 0);
    QColor trans=m_backgroundColor;
    trans.setAlpha(0);
    grad.setColorAt(0, trans);
    grad.setColorAt(1, m_backgroundColor);
    outPainter->fillRect(-m_width, 0, 20, 13, QBrush(grad));

    outPainter->drawImage(-(m_width-20), 0, img);
}

QRectF STTimelineItemTimeView::boundingRect() const{
    return QRectF(-m_width, 0.f, m_width, 13.f);
}

void STTimelineItemTimeView::updateText(){
    QString newText;
    QDateTime now=QDateTime::currentDateTime();
    qint64 interval=(m_dateTime.msecsTo(now));
    if(interval<0)interval=0;
    quint64 secs=((quint64)interval+500ULL)/1000ULL; // rounding

    if(secs<86400ULL){
        newText=STDateFormatter::toRelativePastDateString(m_dateTime);
    }else{
        // tweet older than 24 hours should show the created date in
        // the absolute format.
        // once it was shown in this format, it will never be shown in
        // relative format, so the update timer can be deactivated.
        m_absView=true;
        newText=STDateFormatter::toMiniDateString(m_dateTime);
    }

    if(newText!=m_text){
        m_text=newText;

        int wid=(int)STFont::defaultFont()->boundingRectForString(m_text).right();
        wid+=30;
        if(wid!=m_width){
            prepareGeometryChange();
            m_width=wid;
        }

        if(m_absView)
            m_timer->setActive(false);
        update();
    }
}
