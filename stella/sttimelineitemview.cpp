#include "sttimelineitemview.h"
#include <QLinearGradient>
#include <QPainter>
#include <QImage>
#include "stfont.h"
#include "stusericonview.h"
#include "stclearbuttonview.h"
#include <QPixmap>
#include <QPixmapCache>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneHoverEvent>
#include <QCursor>
#include "stactionmanager.h"
#include <QMenu>
#include <QGraphicsScene>
#include <QGraphicsView>
#include "staccountsmanager.h"
#include "staccount.h"
#include <QDebug>
#include "ststandardview.h"
#include "sttweetview.h"
#include <QApplication>
#include "stbasetabpageview.h"
#include "ststatusmenu.h"
#include "stthumbnailmanager.h"
#include <QDesktopServices>
#include "stzoomedimagemanager.h"

static const int g_thumbnailSize=48;

STTimelineItemView::STTimelineItemView(const QSize& sz,
                                       STObjectManager::StatusRef status, quint64 accountId,
                                       QGraphicsItem *parent) :
    QGraphicsObject(parent), m_size(sz), m_status(status),m_accountId(accountId)
{

    Q_ASSERT(m_status);

    setCacheMode(DeviceCoordinateCache);
    if(status->retweetedStatus){
        m_iconView=new STUserIconView(status->retweetedStatus->user, this);
    }else{
        m_iconView=new STUserIconView(status->user, this);
    }
    m_iconView->setPos(6.f, 6.f);
    m_iconView->setSize(QSize(52, 52));
    m_iconView->setCursor(Qt::PointingHandCursor);
    m_iconView->setAcceptedMouseButtons(Qt::LeftButton|Qt::RightButton);
    connect(m_iconView, SIGNAL(clicked()),
            this, SLOT(showUser()));

    STObjectManager::EntityRange thumbEntRange=entityRangeForThumbnailForStatus(status);
    static QString mediaUrlString="media_url";
    static QString smallUrlString="small_url";
    static QString urlString="url";
    if((!thumbEntRange.entityType.isEmpty()) && (thumbEntRange.entity.toMap().contains(smallUrlString))){
        // has thumbnail
        m_imageView=new STRemoteImageView(this);
        m_imageView->setPos(sz.width()-g_thumbnailSize-6,
                            23);
        m_imageView->setSize(QSize(g_thumbnailSize, g_thumbnailSize));
        m_imageView->setCursor(Qt::PointingHandCursor);
        m_imageView->setAcceptedMouseButtons(Qt::LeftButton|Qt::RightButton);
        QString smallUrl=thumbEntRange.entity.toMap()[smallUrlString].toString();
        m_imageView->setTicket(STThumbnailManager::sharedManager()->requestUrl(QUrl(smallUrl)));

        QString largeUrl=thumbEntRange.entity.toMap()[mediaUrlString].toString();
        m_imageView->setZoomedImageUrl(largeUrl);

        connect(m_imageView, SIGNAL(clicked()),
                this, SLOT(showThumbnailedImage()));

    }

    m_timeView=new STTimelineItemTimeView(m_status->createdAt,
                                          this);
    m_timeView->setPos(sz.width(), 6);

    m_buttonsBgItem=NULL;
    m_buttonsAnimator=NULL;

    m_replyButton=NULL;
    m_retweetButton=NULL;
    m_favoriteButton=NULL;
    m_optionsButton=NULL;


    setAcceptsHoverEvents(true);



}

void STTimelineItemView::setButtonsOpacity(float v){
    m_buttonsBgItem->setOpacity(v);
}

void STTimelineItemView::hoverEnterEvent(QGraphicsSceneHoverEvent *){
    if(!m_replyButton){
        m_buttonsBgItem=new QGraphicsPixmapItem(this);
        QPixmap bgPixmap;
        static QPixmapCache::Key bgPixmapKey;
        if(!QPixmapCache::find(bgPixmapKey, &bgPixmap)){
            bgPixmap=QPixmap(104, 20);
            bgPixmap.fill(QColor(0,0,0,0));
            QPainter painter(&bgPixmap);
            QLinearGradient grad(0, 0,
                                 20, 0);

            grad.setColorAt(0, QColor(70,70,70,0));
            grad.setColorAt(1, QColor(70,70,70,255));
            painter.fillRect(QRect(QPoint(0,0),bgPixmap.size()), QBrush(grad));

            bgPixmapKey=QPixmapCache::insert(bgPixmap);
        }
        m_buttonsBgItem->setPixmap(bgPixmap);
        m_buttonsBgItem->setPos(m_size.width()-bgPixmap.width(), 2);

        m_replyButton=new STClearButtonView(m_buttonsBgItem);
        m_retweetButton=new STClearButtonView(m_buttonsBgItem);
        m_favoriteButton=new STClearButtonView(m_buttonsBgItem);
        m_optionsButton=new STClearButtonView(m_buttonsBgItem);

        m_replyButton->setToolTip(tr("Reply"));
        m_retweetButton->setToolTip(tr("Retweet"));
        m_favoriteButton->setToolTip(tr("Favorite"));
        m_optionsButton->setToolTip(tr("Options"));

        m_replyButton->setCursor(Qt::PointingHandCursor);
        m_retweetButton->setCursor(Qt::PointingHandCursor);
        m_favoriteButton->setCursor(Qt::PointingHandCursor);
        m_optionsButton->setCursor(Qt::PointingHandCursor);

        m_replyButton->setSize(QSizeF(20, 20));
        m_retweetButton->setSize(QSizeF(20, 20));
        m_favoriteButton->setSize(QSizeF(20, 20));
        m_optionsButton->setSize(QSizeF(20, 20));

        m_replyButton->setText(":/stella/res/ReplyButton.png");
        m_optionsButton->setText(":/stella/res/OptionsSmallButton.png");
        updateButton();

        m_optionsButton->setPos(80, -1);
        m_favoriteButton->setPos(61, -1);
        m_retweetButton->setPos(40, -1);
        m_replyButton->setPos(20, -1);

        connect(m_favoriteButton, SIGNAL(activated()),
                this, SLOT(toggleFavorite()));
        connect(m_retweetButton, SIGNAL(activated()),
                this, SLOT(showRetweetMenu()));
        connect(m_replyButton, SIGNAL(activated()),
                this, SLOT(reply()));
        connect(m_optionsButton, SIGNAL(activated()),
                this, SLOT(showOptionsMenu()));

        connect(STActionManager::sharedManager(),
                SIGNAL(statusActionDone(STObjectManager::StatusRef,quint64,STActionManager::StatusAction)),
                this, SLOT(statusActionDone(STObjectManager::StatusRef,quint64,STActionManager::StatusAction)));

        connect(STActionManager::sharedManager(),
                SIGNAL(statusActionFailed(quint64,quint64,STActionManager::StatusAction)),
                this, SLOT(statusActionFailed(quint64,quint64,STActionManager::StatusAction)));

        m_buttonsAnimator=new STSimpleAnimator(this);
        connect(m_buttonsAnimator, SIGNAL(valueChanged(float)),
                this, SLOT(setButtonsOpacity(float)));
        m_buttonsAnimator->setValue(0);

    }
    m_buttonsBgItem->setVisible(true);
    m_buttonsAnimator->setValueAnimated(1, 100);
}

void STTimelineItemView::updateButton(){
    if(!m_replyButton)
        return;

    STObjectManager::StatusRef ref=m_status;

    if(ref->retweetedStatus){
        ref=ref->retweetedStatus;
    }
    QMutexLocker locker(&(ref->mutex));

    if(ref->retweetStatusId(m_accountId))
        m_retweetButton->setText(":/stella/res/RetweetButtonOn.png");
    else
        m_retweetButton->setText(":/stella/res/RetweetButton.png");
    m_favorited=ref->isFavoritedBy(m_accountId);
    if(m_favorited)
        m_favoriteButton->setText(":/stella/res/FavoriteButtonOn.png");
    else
        m_favoriteButton->setText(":/stella/res/FavoriteButton.png");
}

void STTimelineItemView::statusActionDone(STObjectManager::StatusRef stat, quint64 accountId, STActionManager::StatusAction act){
    if(accountId!=m_accountId)
        return;

    STObjectManager::StatusRef ref=m_status;

    if(ref->retweetedStatus){
        ref=ref->retweetedStatus;
    }

    if(act==STActionManager::Favorite ||
            act==STActionManager::Unfavorite){
        if(stat->id!=ref->id)
            return;
        updateButton();
    }else if(act==STActionManager::Retweet){
        if(!stat->retweetedStatus)
            return;
        if(stat->retweetedStatus->id!=ref->id)
            return;
        updateButton();
    }else if(act==STActionManager::DeleteStatus){
        if(stat->retweetedStatus){
            // unretweet
            if(stat->retweetedStatus->id!=ref->id)
                return;
            updateButton();
        }
    }
}

void STTimelineItemView::statusActionFailed(quint64 statusId, quint64 accountId, STActionManager::StatusAction){
    if(accountId!=m_accountId)
        return;

    STObjectManager::StatusRef ref=m_status;

    if(ref->retweetedStatus){
        ref=ref->retweetedStatus;
    }

    if(ref->id==statusId)
        updateButton();
}

void STTimelineItemView::hoverLeaveEvent(QGraphicsSceneHoverEvent *){
    if(m_buttonsAnimator)
        m_buttonsAnimator->setValueAnimated(0, 200);
    //m_buttonsBgItem->setVisible(false);
    if(m_tweetTextLinkManager.mouseLeave())
        update();
    if(m_retweetUserLinkManager.mouseLeave())
        update();
    if(m_userNameLinkManager.mouseLeave())
        update();
    if(m_userScreenNameLinkManager.mouseLeave())
        update();
    updateCursor();
}

void STTimelineItemView::hoverMoveEvent(QGraphicsSceneHoverEvent *event){
    if(m_tweetTextLinkManager.mouseMove(event->pos()))
        update();
    if(m_retweetUserLinkManager.mouseMove(event->pos()))
        update();
    if(m_userNameLinkManager.mouseMove(event->pos()))
        update();
    if(m_userScreenNameLinkManager.mouseMove(event->pos()))
        update();
    updateCursor();
}

void STTimelineItemView::mousePressEvent(QGraphicsSceneMouseEvent *event){
    if(m_tweetTextLinkManager.mouseDown(event->pos()))
        update();
    if(m_retweetUserLinkManager.mouseDown(event->pos()))
        update();
    if(m_userNameLinkManager.mouseDown(event->pos()))
        update();
    if(m_userScreenNameLinkManager.mouseDown(event->pos()))
        update();
    updateCursor();
}

void STTimelineItemView::mouseReleaseEvent(QGraphicsSceneMouseEvent *event){
    if(m_tweetTextLinkManager.mouseMove(event->pos()))
        update();
    if(m_retweetUserLinkManager.mouseMove(event->pos()))
        update();
    if(m_userNameLinkManager.mouseMove(event->pos()))
        update();
    if(m_userScreenNameLinkManager.mouseMove(event->pos()))
        update();
    STObjectManager::LinkActivationInfo info;
    if(m_tweetTextLinkManager.mouseUp()){
        emit linkActivated(m_tweetTextLinkManager.hotEntityRange(), info);
    }else if(m_retweetUserLinkManager.mouseUp()){
        emit linkActivated(m_retweetUserLinkManager.hotEntityRange(), info);
    }else if(m_userNameLinkManager.mouseUp()){
        emit linkActivated(m_userNameLinkManager.hotEntityRange(), info);
    }else if(m_userScreenNameLinkManager.mouseUp()){
        emit linkActivated(m_userScreenNameLinkManager.hotEntityRange(), info);
    }
    updateCursor();
}

void STTimelineItemView::updateCursor(){
    if(m_tweetTextLinkManager.hotEntityRange()||
            m_retweetUserLinkManager.hotEntityRange()||
            m_userNameLinkManager.hotEntityRange()||
            m_userScreenNameLinkManager.hotEntityRange()){
        setCursor(Qt::PointingHandCursor);
    }else{
        setCursor(Qt::ArrowCursor);
    }
}

void STTimelineItemView::paint(QPainter *outPainter, const QStyleOptionGraphicsItem *, QWidget *){
    STFont::Layout lay;
    if(!m_tweetTextLinkManager.isEmpty()){
        lay=m_tweetTextLinkManager.layout();
    }
    lay=layoutStatusText(m_size.width(), m_status);

    Q_ASSERT(m_status);

    QImage img(QSize(m_size.width(), m_size.height()+1), QImage::Format_RGB32);
    img.fill(0);

    QPainter painter(&img);
    painter.fillRect(QRect(0, 1, m_size.width(), m_size.height()), QColor(70,70,70));
    painter.fillRect(QRect(0, 0, m_size.width(), 1), QColor(60,60,60));

    STObjectManager::UserRef user=m_status->user;
    if(m_status->retweetedStatus){
        user=m_status->retweetedStatus->user;
    }
    Q_ASSERT(user);

    if(m_userNameLinkManager.isEmpty()){
        QString name=user->data["name"].toString();
        QList<STObjectManager::EntityRange> ranges;
        STObjectManager::EntityRange range;
        range.charIndexStart=0;
        range.charIndexEnd=name.length();
        range.entityType="user_stobject";
        range.stObject=user;
        ranges.push_back(range);
        m_userNameLinkManager.init(name, ranges, STFont::defaultBoldFont());
        m_userNameLinkManager.setPos(QPointF(64.f, 7.f));
    }
    m_userNameLinkManager.draw(img, painter, QColor(240,240,240), QColor(240,240,240));

    if(m_userScreenNameLinkManager.isEmpty()){
        QString screenName="@"+user->data["screen_name"].toString();
        QList<STObjectManager::EntityRange> ranges;
        STObjectManager::EntityRange range;
        range.charIndexStart=0;
        range.charIndexEnd=screenName.length();
        range.entityType="user_stobject";
        range.stObject=user;
        ranges.push_back(range);
        m_userScreenNameLinkManager.init(screenName, ranges, STFont::defaultFont());
        m_userScreenNameLinkManager.setPos(QPointF(64.f+STFont::defaultBoldFont()->boundingRectForLayout(m_userNameLinkManager.layout()).right()+2.f, 7.f));
    }
    m_userScreenNameLinkManager.draw(img, painter, QColor(200,200,200), QColor(200,200,200));


    if(m_tweetTextLinkManager.isEmpty()){
        m_tweetTextLinkManager.init(lay, m_status->entityRanges, STFont::defaultFont());
        m_tweetTextLinkManager.setPos(QPointF(64.f, 23.f));
    }

    m_tweetTextLinkManager.draw(img, painter,QColor(240,240,240), QColor(220,128, 40));

    if(m_status->retweetedStatus){
        //QRectF bnd=STFont::defaultFont()->boundingRectForLayout(lay);
        float rtY=(float)m_size.height()-17.f;//bnd.bottom()+(5.f+16.f)+2.f;

        QPixmap pix;
        static QPixmapCache::Key iconKey;
        if(!QPixmapCache::find(iconKey, &pix)){
            pix.load(":/stella/res/RetweetedIcon.png");
            iconKey=QPixmapCache::insert(pix);
        }

        QString text=tr("Retweeted by %1");
        QString retweeter=m_status->user->data["name"].toString();
        QString insertedText=text.arg(retweeter);

        painter.drawPixmap(QPointF(64.f, rtY), pix);
        if(m_retweetUserLinkManager.isEmpty()){
            QList<STObjectManager::EntityRange> ranges;
            STObjectManager::EntityRange range;
            range.charIndexStart=text.indexOf("%1");
            range.charIndexEnd=range.charIndexStart+retweeter.length();
            range.entityType="user_object";
            range.entity=m_status->user->data;
            ranges.push_back(range);
            m_retweetUserLinkManager.init(insertedText, ranges, STFont::defaultFont());
            m_retweetUserLinkManager.setPos( QPointF(64.f+(float)pix.width()+2.f, rtY+1.f));
        }

        m_retweetUserLinkManager.draw(img,painter, QColor(200,200,200), QColor(200,200,200));

    }

    outPainter->drawImage(0,-1,img);

    //outPainter->fillRect(boundingRect(), QColor(255,255,255,96));
}

STObjectManager::EntityRange STTimelineItemView::entityRangeForThumbnailForStatus(STObjectManager::StatusRef status){
    if(!status)
        return STObjectManager::EntityRange();
    foreach(const STObjectManager::EntityRange& range, status->entityRanges){
        if(range.entityType=="st_image"||
                range.entityType=="st_video"){
            return range;
        }
    }
    return STObjectManager::EntityRange();
}

bool STTimelineItemView::statusNeedsThumbnail(STObjectManager::StatusRef status){
    if(!status)return false;
    foreach(const STObjectManager::EntityRange& range, status->entityRanges){
        if(range.entityType=="st_image"||
                range.entityType=="st_video"){
            return true;
        }
    }
    return false;
}

QSize STTimelineItemView::estimateSize(int width, STObjectManager::StatusRef status){
    STFont::Layout lay=layoutStatusText(width,status);
    float h=STFont::defaultFont()->boundingRectForLayout(lay).bottom();
    if(statusNeedsThumbnail(status)){
        // make a room for thumbnail
        static const float thumbHeight=(float)g_thumbnailSize-1.f;
        if(h<thumbHeight)
            h=thumbHeight;
    }
    h+=16.f; // user name
    if(status->retweetedStatus){
        h+=16.f;
    }
    h+=13.f; // top/bottom margin
    if(h<64.f){
        h=64.f;
    }
    return QSize(width, (int)h);
}

STFont::Layout STTimelineItemView::layoutStatusText(int width,STObjectManager::StatusRef status){
    /*QVariant txt=status->status->data["text"];

    if(status->status->retweetedStatus){
        txt=status->status->retweetedStatus->data["text"];
    }
    QString text=txt.toString();*/
    QString text=status->displayText;
    bool hasThumbnail=statusNeedsThumbnail(status);
    return STFont::defaultFont()->layoutString(text, width-70-(hasThumbnail?(g_thumbnailSize+8):0));
}

QRectF STTimelineItemView::boundingRect() const{
    return QRectF(0,-1,m_size.width(),m_size.height()+1);
}

STTweetView *STTimelineItemView::tweetView() {
    QGraphicsItem *item=this;
    while(item){
        STStandardView *vw=dynamic_cast<STStandardView *>(item);
        if(vw)
            return vw->tweetView();
        item=item->parentItem();
    }
    return NULL;
}

STBaseTabPageView *STTimelineItemView::baseTabPageView() {
    QGraphicsItem *item=this;
    while(item){
        STBaseTabPageView *vw=dynamic_cast<STBaseTabPageView *>(item);
        if(vw)
            return vw;
        item=item->parentItem();
    }
    return NULL;
}

void STTimelineItemView::reply(){
    STTweetView *twv=tweetView();
    if(!twv){
        qWarning()<<"STTimelineItemView::reply: tweetView()==NULL";
        return;
    }

    STObjectManager::StatusRef ref=m_status;

    if(ref->retweetedStatus){
        ref=ref->retweetedStatus;
    }

    bool additive=QApplication::keyboardModifiers()&Qt::ControlModifier;
    twv->reply(ref, additive);
}



void STTimelineItemView::showRetweetMenu(){


    STStatusMenu menu(baseTabPageView(), m_status,
                      STStatusMenu::Retweet);


    QPointF pt=m_retweetButton->mapToScene(m_retweetButton->boundingRect().bottomLeft());
    QGraphicsView *view=this->scene()->views().at(0);
    pt=view->mapToGlobal(view->mapFromScene(pt));
    menu.exec(QPoint((int)pt.x(), (int)pt.y()));
}

void STTimelineItemView::toggleFavorite(){
    STObjectManager::StatusRef ref=m_status;

    if(ref->retweetedStatus){
        ref=ref->retweetedStatus;
    }
    QMutexLocker locker(&(ref->mutex));

    if(m_favorited){
        STActionManager::sharedManager()->statusAction(ref->id, m_accountId,
                                                       STActionManager::Unfavorite);
        m_favoriteButton->setText(":/stella/res/FavoriteButton.png");
        m_favorited=false;
    }else{
        STActionManager::sharedManager()->statusAction(ref->id, m_accountId,
                                                       STActionManager::Favorite);
        m_favoriteButton->setText(":/stella/res/FavoriteButtonOn.png");
        m_favorited=true;
    }
}

void STTimelineItemView::showUser(){
    STObjectManager::StatusRef ref=m_status;
    if(ref->retweetedStatus){
        ref=ref->retweetedStatus;
    }
    STObjectManager::EntityRange range;
    range.charIndexStart=0;
    range.charIndexEnd=0;
    range.entityType="user_stobject";
    range.stObject=ref->user;

    STObjectManager::LinkActivationInfo info;
    emit linkActivated(&range,info);
}

void STTimelineItemView::showOptionsMenu(){

    STStatusMenu menu(baseTabPageView(), m_status,
                      STStatusMenu::Options);

    QPointF pt=m_optionsButton->mapToScene(m_optionsButton->boundingRect().bottomLeft());
    QGraphicsView *view=this->scene()->views().at(0);
    pt=view->mapToGlobal(view->mapFromScene(pt));
    menu.exec(QPoint((int)pt.x(), (int)pt.y()));
}

void STTimelineItemView::showThumbnailedImage(){
    STObjectManager::EntityRange thumbEntRange=entityRangeForThumbnailForStatus(m_status);
    if(thumbEntRange.entityType=="st_image"){
        QString mediaUrl=thumbEntRange.entity.toMap()["media_url"].toString();
        if(!mediaUrl.isNull()){
            STZoomedImageManager::sharedManager()->zoomImageWithUrl(QUrl(mediaUrl), false);
            return;
        }
    }

    STObjectManager::LinkActivationInfo info;
    emit linkActivated(&thumbEntRange, info);
}
