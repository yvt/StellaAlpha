#include "ststatuscontentsview.h"
#include "stlinkedlabelview.h"
#include "stactionmanager.h"
#include "stuserprofilenumberview.h"
#include "stbuttonview.h"
#include "ststatuspane.h"
#include "staccount.h"
#include "sttweetview.h"
#include "ststandardview.h"
#include "ststatusmenu.h"
#include <QDebug>
#include <QGraphicsView>
#include<QGraphicsScene>
#include "stbasetabpageview.h"
#include "stdateformatter.h"
#include "stremoteimageview.h"
#include <QDebug>
#include "stthumbnailmanager.h"
#include "stzoomedimagemanager.h"
#include "stplaceview.h"

static const int g_thumbnailSize=48;

STStatusContentsView::STStatusContentsView(STStatusPane *parent) :
    STScrolledPane(parent)
{
    m_textView=new STLinkedLabelView(contentsView());
    m_textView->setSelectable(true);
    m_textView->setFlag(ItemIsFocusable);
    m_textView->setBgColor(QColor(70,70,70));
    m_textView->setTextColor(QColor(240,240,240));

    m_dateView=new STLinkedLabelView(contentsView());
    m_dateView->setSelectable(true);
    m_dateView->setFlag(ItemIsFocusable);
    m_dateView->setBgColor(QColor(70,70,70));
    m_dateView->setTextColor(QColor(240,240,240, 128));
    m_lastWidth=0;

    m_retweetsView=new STUserProfileNumberView(contentsView());
    m_replyButton=new STButtonView(contentsView());
    m_retweetButton=new STButtonView(contentsView());
    m_favoriteButton=new STButtonView(contentsView());
    m_optionsButton=new STButtonView(contentsView());

    m_replyButton->setButtonType(STButtonView::Flat);
    m_retweetButton->setButtonType(STButtonView::Flat);
    m_favoriteButton->setButtonType(STButtonView::Flat);
    m_optionsButton->setButtonType(STButtonView::Flat);

    m_retweetsView->setLabelText(":/stella/res/RtsText.png");
    m_replyButton->setText(":/stella/res/BigReplyButton.png");

    m_optionsButton->setText(":/stella/res/OptionsButton.png");

    m_retweetsView->setToolTip(tr("View Retweets"));
    m_replyButton->setToolTip(tr("Reply"));
    m_retweetButton->setToolTip(tr("Retweet"));
    m_favoriteButton->setToolTip(tr("Favorite"));
    m_optionsButton->setToolTip(tr("Options"));

    m_placeView=new STPlaceView(contentsView());

    connect(m_textView, SIGNAL(linkActivated(STObjectManager::EntityRange*,
                                             STObjectManager::LinkActivationInfo)),
            this, SLOT(linkActivate(STObjectManager::EntityRange*,
                                    STObjectManager::LinkActivationInfo)));
    connect(m_placeView, SIGNAL(linkActivated(STObjectManager::EntityRange*,
                                              STObjectManager::LinkActivationInfo)),
            this, SLOT(linkActivate(STObjectManager::EntityRange*,
                                    STObjectManager::LinkActivationInfo)));

    connect(STActionManager::sharedManager(),
            SIGNAL(statusActionDone(STObjectManager::StatusRef,quint64,STActionManager::StatusAction)),
            this,
            SLOT(statusActionDone(STObjectManager::StatusRef,quint64,STActionManager::StatusAction)));
    connect(STActionManager::sharedManager(),
            SIGNAL(statusActionFailed(quint64,quint64,STActionManager::StatusAction)),
            this,
            SLOT(statusActionFailed(quint64,quint64,STActionManager::StatusAction)));

    connect(m_retweetsView, SIGNAL(activated()),
            this, SLOT(viewRetweets()));
    connect(m_replyButton, SIGNAL(activated()),
            this, SLOT(reply()));
    connect(m_retweetButton, SIGNAL(activated()),
            this, SLOT(showRetweetMenu()));
    connect(m_favoriteButton, SIGNAL(activated()),
            this, SLOT(toggleFavorite()));
    connect(m_optionsButton, SIGNAL(activated()),
            this, SLOT(showOptionsMenu()));
}

void STStatusContentsView::setStatus(STObjectManager::StatusRef st){
    m_status=st;
    if(m_status->retweetedStatus)
        m_status=m_status->retweetedStatus;
    m_retweetsView->setNumberText(st->data["retweet_count"].toString());
    m_favorited=st->isFavoritedBy(account()->userId());

    m_retweetButton->setText(st->retweetStatusId(account()->userId())?
                                 ":/stella/res/BigRetweetButtonOn.png":
                                 ":/stella/res/BigRetweetButton.png");
    m_favoriteButton->setText(m_favorited?
                                  ":/stella/res/BigFavoriteButtonOn.png":
                                  ":/stella/res/BigFavoriteButton.png");

    foreach(STRemoteImageView *vw, m_imageViews){
        delete vw;
    }

    foreach(const STObjectManager::EntityRange& range, m_status->entityRanges){
        if(range.entityType=="st_image" || range.entityType=="st_video"){
            STRemoteImageView *imgView=new STRemoteImageView(contentsView());
            static QString mediaUrlString="media_url";
            static QString smallUrlString="small_url";
            static QString urlString="url";

            imgView->setSize(QSize(g_thumbnailSize, g_thumbnailSize));

            imgView->setCursor(Qt::PointingHandCursor);
            imgView->setAcceptedMouseButtons(Qt::LeftButton|Qt::RightButton);
            QString smallUrl=range.entity.toMap()[smallUrlString].toString();
            imgView->setTicket(STThumbnailManager::sharedManager()->requestUrl(QUrl(smallUrl)));

            QString largeUrl=range.entity.toMap()[mediaUrlString].toString();
            imgView->setZoomedImageUrl(largeUrl);

            connect(imgView, SIGNAL(clicked()),
                    this, SLOT(thumbnailClicked()));

            imgView->setProperty("entityRangeUrl", range.entity.toMap()[urlString]);

            m_imageViews.append(imgView);
        }
    }

    relayout();
}

void STStatusContentsView::thumbnailClicked(){
    STRemoteImageView *imgView=dynamic_cast<STRemoteImageView *>(sender());
    Q_ASSERT(imgView);

    QVariant entityRangeUrl=imgView->property("entityRangeUrl");

    foreach(const STObjectManager::EntityRange& range, m_status->entityRanges){
        if(range.entityType=="st_image" || range.entityType=="st_video"){
            static QString mediaUrlString="media_url";
            static QString smallUrlString="small_url";
            static QString urlString="url";
            if(range.entity.toMap()[urlString]==entityRangeUrl){
                if(range.entityType=="st_image"){
                    QString mediaUrl=range.entity.toMap()["media_url"].toString();
                    if(!mediaUrl.isNull()){
                        STZoomedImageManager::sharedManager()->zoomImageWithUrl(QUrl(mediaUrl), false);
                        return;
                    }
                }

                STObjectManager::LinkActivationInfo info;
                linkActivate(const_cast<STObjectManager::EntityRange *>(&range),info);
                return;
            }
        }
    }

    qWarning()<<"STStatusContentsView::thumbnailClicked: thumbnailed entity not found with url: "<<entityRangeUrl;
}

STAccount *STStatusContentsView::account() {
    STStatusPane *pane=static_cast<STStatusPane *>(parentItem());
    return pane->account();
}

void STStatusContentsView::relayoutContents(){
    if(contentsWidth()<=0)
        return;
    bool loaded=(bool)(m_status) && (contentsWidth()!=m_lastWidth);
    int cy=7;

    if(loaded){

        m_textView->setContents(STFont::defaultFont(), m_status->displayText,
                                m_status->entityRanges, contentsWidth()-20,false, 5.f);
    }
    m_textView->setPos(10, cy-(int)m_textView->boundingRect().top());
    cy+=(int)m_textView->boundingRect().height();
    cy+=1;

    // thumbnails
    int thumbX=10;
    int thumbWrapRight=contentsWidth()-10;
    foreach(STRemoteImageView *imageView, m_imageViews){
        if(thumbX+g_thumbnailSize>thumbWrapRight){
            thumbX=10;
            cy+=g_thumbnailSize+5;
        }
        imageView->setPos(thumbX, cy);
        thumbX+=g_thumbnailSize+5;

    }
    if(!m_imageViews.isEmpty()){
        cy+=g_thumbnailSize+5;
    }

    if(loaded){
        m_dateView->setContents(STFont::defaultFont(),
                                STDateFormatter::toLongDateTimeString(m_status->createdAt)
                                , contentsWidth()-20);
    }
    m_dateView->setPos(10, cy-(int)m_dateView->boundingRect().top());
    cy+=(int)m_dateView->boundingRect().height();
    cy+=10;

    QVariantMap place=m_status->data.contains("place")?m_status->data["place"].toMap():QVariantMap();
    QVariantMap coord=m_status->data.contains("coordinates")?m_status->data["coordinates"].toMap():QVariantMap();
    if(!place.isEmpty() && !coord.isEmpty()){
        m_placeView->setObjects(coord, place);

        int placeViewHeight=200;
        m_placeView->setSize(QSize(contentsWidth()-20, placeViewHeight));
        m_placeView->setPos(10, cy);
        cy+=placeViewHeight;
        cy+=10;
    }

    int cx=contentsWidth()-10;
    const int btnWidth=43;
    const int btnHeight=33;
    cx-=btnWidth;
    m_optionsButton->setSize(QSize(btnWidth,btnHeight));
    m_optionsButton->setPos(cx, cy);
    cx-=btnWidth-1;
    m_favoriteButton->setSize(QSize(btnWidth,btnHeight));
    m_favoriteButton->setPos(cx, cy);
    cx-=btnWidth-1;
    m_retweetButton->setSize(QSize(btnWidth,btnHeight));
    m_retweetButton->setPos(cx, cy);
    cx-=btnWidth-1;
    m_replyButton->setSize(QSize(btnWidth,btnHeight));
    m_replyButton->setPos(cx, cy);

    cx++;

    m_retweetsView->setSize(QSize(cx-10, btnHeight));
    m_retweetsView->setPos(10, cy);
    cy+=btnHeight;

    cy+=10;

    setContentsHeight(cy);
}

STTweetView *STStatusContentsView::tweetView() {
    QGraphicsItem *item=this;
    while(item){
        STStandardView *vw=dynamic_cast<STStandardView *>(item);
        if(vw)
            return vw->tweetView();
        item=item->parentItem();
    }
    return NULL;
}

STBaseTabPageView *STStatusContentsView::baseTabPageView() {
    QGraphicsItem *item=this;
    while(item){
        STBaseTabPageView *vw=dynamic_cast<STBaseTabPageView *>(item);
        if(vw)
            return vw;
        item=item->parentItem();
    }
    return NULL;
}


void STStatusContentsView::reply(){
    STTweetView *twv=tweetView();
    if(!twv){
        qWarning()<<"STStatusContentsView::reply: tweetView()==NULL";
        return;
    }

    STObjectManager::StatusRef ref=m_status;

    if(ref->retweetedStatus){
        ref=ref->retweetedStatus;
    }

    bool additive=QApplication::keyboardModifiers()&Qt::ControlModifier;
    twv->reply(ref, additive);
}


void STStatusContentsView::showRetweetMenu(){
    STStatusMenu menu(baseTabPageView(), m_status,
                      STStatusMenu::Retweet);


    QPointF pt=m_retweetButton->mapToScene(m_retweetButton->boundingRect().bottomLeft());
    QGraphicsView *view=this->scene()->views().at(0);
    pt=view->mapToGlobal(view->mapFromScene(pt));
    menu.exec(QPoint((int)pt.x(), (int)pt.y()));
}

void STStatusContentsView::toggleFavorite(){
    STObjectManager::StatusRef ref=m_status;

    if(ref->retweetedStatus){
        ref=ref->retweetedStatus;
    }
    QMutexLocker locker(&(ref->mutex));

    if(m_favorited){
        STActionManager::sharedManager()->statusAction(ref->id, account()->userId(),
                                                       STActionManager::Unfavorite);
        m_favoriteButton->setText(":/stella/res/BigFavoriteButton.png");
        m_favorited=false;
    }else{
        STActionManager::sharedManager()->statusAction(ref->id, account()->userId(),
                                                       STActionManager::Favorite);
        m_favoriteButton->setText(":/stella/res/BigFavoriteButtonOn.png");
        m_favorited=true;
    }
}


void STStatusContentsView::showOptionsMenu(){
    STStatusMenu menu(baseTabPageView(), m_status,
                      STStatusMenu::OptionsWithoutViewDetail);


    QPointF pt=m_optionsButton->mapToScene(m_optionsButton->boundingRect().bottomLeft());
    QGraphicsView *view=this->scene()->views().at(0);
    pt=view->mapToGlobal(view->mapFromScene(pt));
    menu.exec(QPoint((int)pt.x(), (int)pt.y()));
}

void STStatusContentsView::viewRetweets(){

}

void STStatusContentsView::statusActionDone(STObjectManager::StatusRef stat, quint64 accountId, STActionManager::StatusAction){
    if(accountId!=account()->userId())
        return;
    STObjectManager::StatusRef instat=stat;
    if(instat->retweetedStatus)
        instat=instat->retweetedStatus;
    if(instat->id!=m_status->id)
        return;
    setStatus(instat);
}

void STStatusContentsView::statusActionFailed(quint64 statusId, quint64 accountId, STActionManager::StatusAction act){
    if(accountId!=account()->userId())
        return;

    STObjectManager::StatusRef ref=m_status;

    if(ref->retweetedStatus){
        ref=ref->retweetedStatus;
    }

    if(ref->id==statusId)
        setStatus(m_status);
}

