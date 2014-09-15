#include "ststatususernameview.h"
#include <QImage>
#include "stfont.h"
#include <QGraphicsSceneMouseEvent>
#include <QCursor>
#include <QGraphicsSceneHoverEvent>
#include <QPainterPath>
#include <QLinearGradient>

static const int g_arrowTop=47;
static const int g_arrowBottom=g_arrowTop+6;
static const int g_arrowPos=4+18;
static const int g_arrowWidthHalf=(g_arrowBottom-g_arrowTop);

STStatusUserNameView::STStatusUserNameView(QGraphicsItem *parent) :
    QGraphicsObject(parent)
{
    setCacheMode(DeviceCoordinateCache);
    setAcceptsHoverEvents(true);

}

void STStatusUserNameView::setWidth(int sz){

    if(sz==m_size.width())return;
    prepareGeometryChange();
    m_size=QSize(sz, g_arrowBottom+10);
}

int STStatusUserNameView::boxHeight() const{
    return g_arrowBottom-1;
}

void STStatusUserNameView::setUser(STObjectManager::UserRef us){
    m_user=us;
    m_nameLinkManager.clear();;
    m_screenNameLinkManager.clear();;
    update();
}

void STStatusUserNameView::paint(QPainter *outPainter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    QImage img(m_size, QImage::Format_ARGB32_Premultiplied);
    img.fill(QColor(70,70,70,0));

    QPainter painter(&img);

    // draw gradient fade
    {
        QLinearGradient grad(0,g_arrowBottom,0,g_arrowBottom+10);
        grad.setColorAt(0, QColor(70,70,70));
        grad.setColorAt(1, QColor(70,70,70,0));
        painter.fillRect(0,0,m_size.width()-16,m_size.height(),grad);
    }

    // draw arrow and something
    {
        painter.save();
        painter.setRenderHint(QPainter::HighQualityAntialiasing);
        painter.setRenderHint(QPainter::Antialiasing);

        QPainterPath path;
        path.moveTo(0,0);
        path.lineTo(0,g_arrowBottom);
        path.lineTo(g_arrowPos-g_arrowWidthHalf, g_arrowBottom);
        path.lineTo(g_arrowPos, g_arrowTop);
        path.lineTo(g_arrowPos+g_arrowWidthHalf, g_arrowBottom);
        path.lineTo(m_size.width(),g_arrowBottom);
        path.lineTo(m_size.width(),0);
        path.closeSubpath();

        path.translate(0, 0.f);
        painter.save();
        painter.setClipRect(0,0,m_size.width()-16,m_size.height());
        painter.fillPath(path, QColor(75,75,75));
        painter.restore();
        path.translate(0, -1);
        painter.fillPath(path, QColor(50,50,50));
        path.translate(0, -1);
        painter.fillPath(path, QColor(63,63,63));
        path.translate(0, -1);
        painter.fillPath(path, QColor(60,60,60));

        painter.restore();
    }

    if(m_user){

        if(m_nameLinkManager.isEmpty()){
            QString name=m_user->data["name"].toString();
            QList<STObjectManager::EntityRange> ranges;
            STObjectManager::EntityRange range;
            range.charIndexStart=0;
            range.charIndexEnd=name.length();
            range.entityType="user_stobject";
            range.stObject=m_user;
            ranges.push_back(range);
            m_nameLinkManager.init(name, ranges, STFont::defaultBoldFont());
            m_nameLinkManager.setPos(QPointF(42, 8));
        }
        m_nameLinkManager.draw(img, painter, QColor(240,240,240), QColor(240,240,240));

        if(m_screenNameLinkManager.isEmpty()){
            QString screenName="@"+m_user->data["screen_name"].toString();
            QList<STObjectManager::EntityRange> ranges;
            STObjectManager::EntityRange range;
            range.charIndexStart=0;
            range.charIndexEnd=screenName.length();
            range.entityType="user_stobject";
            range.stObject=m_user;
            ranges.push_back(range);
            m_screenNameLinkManager.init(screenName, ranges, STFont::defaultFont());
            m_screenNameLinkManager.setPos(QPointF(42, 24));
        }
        m_screenNameLinkManager.draw(img, painter, QColor(200,200,200), QColor(200,200,200));

    }

    outPainter->drawImage(0,0,img);
}

QRectF STStatusUserNameView::boundingRect() const{
    return QRectF(0,0,m_size.width(),m_size.height());
}

void STStatusUserNameView::hoverEnterEvent(QGraphicsSceneHoverEvent *event){

}

void STStatusUserNameView::updateCursor(){
    if(m_nameLinkManager.hotEntityRange() ||
            m_screenNameLinkManager.hotEntityRange())
        setCursor(Qt::PointingHandCursor);
    else
        setCursor(Qt::ArrowCursor);
}

void STStatusUserNameView::hoverMoveEvent(QGraphicsSceneHoverEvent *event){
    if(m_nameLinkManager.mouseMove(event->pos()))
        update();
    if(m_screenNameLinkManager.mouseMove(event->pos()))
        update();
    updateCursor();
}

void STStatusUserNameView::hoverLeaveEvent(QGraphicsSceneHoverEvent *event){
    if(m_nameLinkManager.mouseLeave())
        update();
    if(m_screenNameLinkManager.mouseLeave())
        update();
    updateCursor();
}

void STStatusUserNameView::mousePressEvent(QGraphicsSceneMouseEvent *event){
    if(m_nameLinkManager.mouseDown(event->pos()))
        update();
    if(m_screenNameLinkManager.mouseDown(event->pos()))
        update();
    updateCursor();
}

void STStatusUserNameView::mouseReleaseEvent(QGraphicsSceneMouseEvent *event){
    if(m_nameLinkManager.mouseMove(event->pos()))
        update();
    if(m_screenNameLinkManager.mouseMove(event->pos()))
        update();
    STObjectManager::LinkActivationInfo info;
    if(m_nameLinkManager.mouseUp()){
        emit linkActivated(m_nameLinkManager.hotEntityRange(),info);
    }else if(m_screenNameLinkManager.mouseUp()){
        emit linkActivated(m_screenNameLinkManager.hotEntityRange(),info);
    }
    updateCursor();
}
