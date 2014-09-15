#include "ststdafx.h"
#include "stnewslistitemview.h"
#include "sttimelineitemtimeview.h"
#include "stfont.h"
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>
#include <QCursor>
#include <QDateTime>
#include <QPixmapCache>
#include "stusericonview.h"
#include "staccountsmanager.h"
#include "staccount.h"

STNewsListItemView::STNewsListItemView(STNewsListView::NewsEntry *entry,QSize size, QGraphicsItem *parent) :
    QGraphicsObject(parent), m_entry(*entry),m_size(size)
{
    m_timeView=new STTimelineItemTimeView(entry->timestamp, this);
    m_timeView->setPos(size.width(), 4);
    m_timeView->setBackgroundColor(QColor(90,90,90));

    m_flashView=new QGraphicsRectItem(this);
    m_flashView->setRect(0,0,size.width(),size.height());

    m_flashView->setPen(QPen(QColor(0,0,0,0)));
    switch(entry->type){
    case STNewsListView::Error:
        m_flashView->setBrush(QColor(255,0,0,160));
        break;
    case STNewsListView::Favorite:
        m_flashView->setBrush(QColor(255,208,30,160));
        break;
    case STNewsListView::Retweet:
        m_flashView->setBrush(QColor(131,221,65,160));
        break;
    case STNewsListView::Follow:
        m_flashView->setBrush(QColor(102,204,255,160));
        break;
    case STNewsListView::ListSubscribed:
        m_flashView->setBrush(QColor(204,102,255,160));
        break;
    case STNewsListView::AddedToList:
        m_flashView->setBrush(QColor(255,128,0,160));
        break;
    case STNewsListView::Tweet:
        m_flashView->setBrush(QColor(255,255,255,160));
        break;
    }

    m_iconView=NULL;
    STAccount *acc=STAccountsManager::sharedManager()->account(entry->account);
    if(acc){
        m_iconView=new STUserIconView(acc->userObject(), this);
        m_iconView->setBorderless(true);
        m_iconView->setPos(3, 3);
        m_iconView->setSize(QSize(13,13));
        m_iconView->setAcceptedMouseButtons(Qt::LeftButton);
        m_iconView->setCursor(Qt::PointingHandCursor);
        connect(m_iconView, SIGNAL(clicked()),
                this, SLOT(activateNewsAccount()));
        m_iconViewAnimator=new STSimpleAnimator(this);
        connect(m_iconViewAnimator, SIGNAL(valueChanged(float)),
                this, SLOT(updateIconViewOpacity(float)));
        m_iconViewAnimator->setValue(0);
    }else{
        m_iconViewAnimator=NULL;
    }

    m_flashAnimator=new STSimpleAnimator(this);
    connect(m_flashAnimator, SIGNAL(valueChanged(float)),
            this, SLOT(updateFlash(float)));

    setCacheMode(DeviceCoordinateCache);
    setAcceptsHoverEvents(true);

}

void STNewsListItemView::setTriggerTime(QDateTime dt){
    qint64 tm=dt.msecsTo(QDateTime::currentDateTime());
    static const float fadeTime=500.f;
    float per=(float)tm/fadeTime;
    if(per>=1.f){
        m_flashAnimator->setValue(0.f);
        return;
    }else if(per<0.f){
        per=0.f;
    }

    per=1.f-per;
    m_flashAnimator->setValue(per);
    m_flashAnimator->setValueAnimated(0.f, (int)(fadeTime*per));

}

void STNewsListItemView::updateFlash(float v){
    v=qMin(1.f, v*2.f);
    m_flashView->setOpacity(v);
}

void STNewsListItemView::updateIconViewOpacity(float v){
    m_iconView->setOpacity(v);
}

void STNewsListItemView::activateNewsAccount(){
    STObjectManager::LinkActivationInfo info;
    STObjectManager::EntityRange range;
    info.accountToOpen=m_entry.account;
    range.entityType="noop";
    emit linkActivated(&range,info);
}

void STNewsListItemView::paint(QPainter *outPainter, const QStyleOptionGraphicsItem *, QWidget *){
    QImage image(m_size, QImage::Format_RGB32);
    image.fill(QColor(90,90,90));

    QPainter painter(&image);

    QPixmap pix;
    QString imgPath;
    switch(m_entry.type){
    case STNewsListView::Error:
        imgPath=":/stella/res/NewsErrorIcon.png";
        break;
    case STNewsListView::Favorite:
        imgPath=":/stella/res/NewsFavoriteIcon.png";
        break;
    case STNewsListView::Retweet:
        imgPath=":/stella/res/NewsRetweetIcon.png";
        break;
    case STNewsListView::Follow:
        imgPath=":/stella/res/NewsFollowIcon.png";
        break;
    case STNewsListView::ListSubscribed:
        imgPath=":/stella/res/NewsListSubscribedIcon.png";
        break;
    case STNewsListView::AddedToList:
        imgPath=":/stella/res/NewsAddedToListIcon.png";
        break;
    case STNewsListView::Tweet:
        imgPath=":/stella/res/NewsTweetIcon.png";
        break;
    default:
        Q_ASSERT(false);
    }
    if(!QPixmapCache::find(imgPath, &pix)){
        pix.load(imgPath);
        QPixmapCache::insert(imgPath, pix);
    }

    painter.drawPixmap(3,3,pix);

    if(m_text.isEmpty()){
        QString text=m_entry.message;
        m_text.init(text, m_entry.links, STFont::defaultFont());
        m_text.setPos(QPointF(19,4));
    }

    m_text.draw(image, painter, QColor(240,240,240), QColor(240,240,240));

    outPainter->drawImage(0,0,image);
}

void STNewsListItemView::updateCursor(){
    if(m_text.hotEntityRange())
        setCursor(Qt::PointingHandCursor);
    else
        setCursor(Qt::ArrowCursor);
}

void STNewsListItemView::hoverEnterEvent(QGraphicsSceneHoverEvent *){
    m_iconViewAnimator->setValueAnimated(1, 200);
}

void STNewsListItemView::hoverLeaveEvent(QGraphicsSceneHoverEvent *){
    m_iconViewAnimator->setValueAnimated(0, 200);
    if(m_text.mouseLeave())
        update();
    updateCursor();
}

void STNewsListItemView::hoverMoveEvent(QGraphicsSceneHoverEvent *event){
    if(m_text.mouseMove(event->pos()))
        update();
    updateCursor();
}

void STNewsListItemView::mousePressEvent(QGraphicsSceneMouseEvent *event){
    if(m_text.mouseDown(event->pos()))
        update();
    updateCursor();
}

void STNewsListItemView::mouseReleaseEvent(QGraphicsSceneMouseEvent *event){
    if(m_text.mouseMove(event->pos()))
        update();
    updateCursor();
    if(m_text.mouseUp()){
        STObjectManager::LinkActivationInfo info;
        info.accountToOpen=m_entry.account;
        emit linkActivated(m_text.hotEntityRange(),info);
    }
}

QRectF STNewsListItemView::boundingRect() const{
    return QRectF(0,0,m_size.width(),m_size.height());
}
