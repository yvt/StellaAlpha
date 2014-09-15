#include "stuserpane.h"
#include "stusericonview.h"
#include <QtKOAuth>
#include "stjsonparser.h"
#include "ststripeanimationview.h"
#include <QVariantList>
#include "stfont.h"
#include <QImage>
#include <QPainter>
#include "staccount.h"
#include <QDebug>
#include <QLinearGradient>
#include <QBrush>
#include <QPainterPath>
#include <QFont>
#include "sttabview.h"
#include "stuserprofileview.h"
#include "sttimelinepane.h"
#include <QMenu>
#include <QGraphicsSceneContextMenuEvent>
#include <QClipboard>
#include <QApplication>
#include "stsimpleanimator.h"
#include "stzoomedimagemanager.h"
#include <QPixmapCache>
#include "stclearbuttonview.h"

static QPixmap loadResourcePixmap(QString key){
    QPixmap pix;
    if(!QPixmapCache::find(key, &pix)){
        pix.load(key);
        QPixmapCache::insert(key, pix);
    }
    return pix;
}

STUserPane::STUserPane(quint64 userId, STAccount *account, QGraphicsItem *parent) :
    STPane(parent), m_userId(userId), m_account(account)
{
    setFlag(ItemHasNoContents, false);

    this->setFlag(ItemClipsChildrenToShape);

    m_iconView=new STUserIconView(STObjectManager::UserRef(), contentsView());
    m_iconView->setSize(QSize(52,52));
    connect(m_iconView, SIGNAL(clicked()),
            this, SLOT(showLargeAvatar()));
    m_iconView->setAcceptedMouseButtons(Qt::LeftButton);
    m_iconView->setCursor(Qt::PointingHandCursor);

    m_statusView=new QGraphicsPixmapItem(contentsView());
    m_nameView=new QGraphicsPixmapItem(contentsView());

    m_protectedIcon=new QGraphicsPixmapItem(contentsView());
    m_protectedIcon->setPixmap(loadResourcePixmap(":/stella/res/UserProtectedIcon.png"));
    m_verifiedIcon=new QGraphicsPixmapItem(contentsView());
    m_verifiedIcon->setPixmap(loadResourcePixmap(":/stella/res/UserVerifiedIcon.png"));
    m_translatorIcon=new QGraphicsPixmapItem(contentsView());
    m_translatorIcon->setPixmap(loadResourcePixmap(":/stella/res/UserTranslatorIcon.png"));
    m_optionsButton=new STClearButtonView(contentsView());
    m_optionsButton->setText(":/stella/res/UserOptionsButton.png");
    connect(m_optionsButton, SIGNAL(activated()),
            this, SLOT(showOptions()));
    m_optionsButton->setSize(QSizeF(16,16));
    m_protectedIcon->setZValue(1);
    m_verifiedIcon->setZValue(1);
    m_translatorIcon->setZValue(1);
    m_optionsButton->setZValue(1);

    m_animView=new STStripeAnimationView(this);
    m_animView->setZValue(2);

    m_nameView->setAcceptedMouseButtons(Qt::NoButton);
    m_nameView->setOpacity(0.f);

    m_tabView=new STTabView(4, contentsView());
    m_tabView->setTabText(0, ":/stella/res/SmallProfileIcon.png");
    m_tabView->setTabText(1, ":/stella/res/SmallTweetsIcon.png");
    m_tabView->setTabText(2, ":/stella/res/SmallMentionIcon.png");
    m_tabView->setTabText(3, ":/stella/res/SmallFavoriteIcon.png");
    //m_tabView->setTabText(4, ":/stella/res/SmallListIcon.png");

    m_tabView->setTabTooltip(0, tr("Profile"));
    m_tabView->setTabTooltip(1, tr("Tweets"));
    m_tabView->setTabTooltip(2, tr("Mentions"));
    m_tabView->setTabTooltip(3, tr("Favorites"));
   // m_tabView->setTabTooltip(4, tr("Lists"));

    m_profileTab=new STUserProfileView(this);
    m_tweetsTab=NULL;
    m_mentionsTab=NULL;
    m_favoritesTab=NULL;
    m_lastTab=0;

    m_profileTab->setOpacity(0.f);

    m_animator.activateView(m_profileTab);

    m_manager=new KQOAuthManager(this);
    m_request=NULL;

    connect(m_manager, SIGNAL(requestReadyEx(QByteArray,int)),
            this, SLOT(requestReady(QByteArray,int)));
    connect(m_tabView, SIGNAL(currentTabChanged()),
            this, SLOT(tabChanged()));

    m_copyNameAction=new QAction(tr("Copy Name"), this);
    m_copyScreenNameAction=new QAction(tr("Copy Screen Name"), this);

    connect(m_copyNameAction, SIGNAL(triggered()),
            this, SLOT(copyName()));
    connect(m_copyScreenNameAction, SIGNAL(triggered()),
            this, SLOT(copyScreenName()));

    m_displayAnimator=new STSimpleAnimator(this);
    m_displayAnimator->setValue(0.f);
    connect(m_displayAnimator, SIGNAL(valueChanged(float)),
            this, SLOT(animated(float)));

    m_animViewAnimator=new STSimpleAnimator(this);
    m_animViewAnimator->setValue(1.f);
    connect(m_animViewAnimator, SIGNAL(valueChanged(float)),
            this, SLOT(setAnimViewOpacity(float)));

    updateIcons();

    setVisible(false);
}

void STUserPane::animated(float op){
    m_nameView->setOpacity(op);
    m_profileTab->setOpacity(op);
}

void STUserPane::setAnimViewOpacity(float op){
    m_animView->setOpacity(op);
}


bool STUserPane::isRequesting(){
    return m_request!=NULL;
}

void STUserPane::startRequest(){
    if(isRequesting())
        return;
    if((!m_user) || (QDateTime::currentMSecsSinceEpoch()>m_user->loadedTime+60000)){

        m_request=new KQOAuthRequest(this);
        m_request->initRequest(KQOAuthRequest::AuthorizedRequest,
                               QUrl("https://api.twitter.com/1.1/users/show.json"));

        m_request->setHttpMethod(KQOAuthRequest::GET);
        m_request->setConsumerKey(m_account->consumerKey());
        m_request->setConsumerSecretKey(m_account->consumerSecret());
        m_request->setToken(m_account->accessToken());
        m_request->setTokenSecret(m_account->accessTokenSecret());

        KQOAuthParameters params;
        params.insert("user_id", QString::number(m_userId));
        params.insert("include_entities", "1");

        m_request->setAdditionalParameters(params);

        m_animViewAnimator->setValueAnimated(1.f, 200);
        m_manager->executeRequest(m_request);

    }else if(m_relationship.isEmpty() && m_userId!=m_account->userId()){

        m_request=new KQOAuthRequest(this);
        m_request->initRequest(KQOAuthRequest::AuthorizedRequest,
                               QUrl("https://api.twitter.com/1.1/friendships/show.json"));

        m_request->setHttpMethod(KQOAuthRequest::GET);
        m_request->setConsumerKey(m_account->consumerKey());
        m_request->setConsumerSecretKey(m_account->consumerSecret());
        m_request->setToken(m_account->accessToken());
        m_request->setTokenSecret(m_account->accessTokenSecret());

        KQOAuthParameters params;
        params.insert("target_id", QString::number(m_userId));

        m_request->setAdditionalParameters(params);

        m_animViewAnimator->setValueAnimated(1.f, 200);
        m_manager->executeRequest(m_request);

    }

}

void STUserPane::requestReady(const QByteArray & data, int code){
    if(data.isEmpty()){
        showError(tr("Network error (%1).").arg(code));
        return;
    }

    m_request->deleteLater();
    m_request=NULL;

    QVariant var=STJsonParser().parse(data);
    QVariantMap mp=var.toMap();
    if(mp.isEmpty()){
        showError(tr("Failed to parse the returned data (4)."));
        return;
    }
    if(mp.contains("errors")){
        QVariantList lst=mp["errors"].toList();
        if(lst.isEmpty()){
            showError(tr("Failed to parse the returned data (1)."));
            return;
        }
        mp=lst[0].toMap();
        var=mp["message"];
        if(var.canConvert(QVariant::String)){
            showError(var.toString());
            return;
        }else{
            showError(tr("Failed to parse the returned data (2)."));
            return;
        }
    }


 //   qDebug()<<data;

    if(mp.contains("relationship")){
        setRelationship(mp["relationship"].toMap());
    }else if(mp.contains("id")){
        STObjectManager::UserRef usr=STObjectManager::sharedManager()->user(mp);
        if(!usr){
            showError(tr("Failed to parse the returned data (3)."));
            return;
        }
        setUser(usr, true);
    }

    startRequest();
}



void STUserPane::relayoutContents(){
    m_iconView->setPos(-2, -2);
    m_animView->setPos(0, 49);
    m_statusView->setPos(0, 48);
    m_optionsButton->setPos(contentsWidth()-18, 50);
    m_animView->setSize(QSize(contentsWidth(), 19));
    m_nameView->setPos(49, 0);

    m_tabView->setSize(QSize(contentsWidth(), 25));
    m_tabView->setPos(0, 67);
    m_tabView->setZValue(1.f);

    int paneTop=67+25+1;
    int paneHeight=size().height()-paneTop;
    m_animator.setWidth(contentsWidth());
    if(m_profileTab){
        m_profileTab->setPos(m_animator.viewXPos(m_profileTab), paneTop);
        m_profileTab->setSize(QSize(contentsWidth(), paneHeight));
    }
    if(m_tweetsTab){
        m_tweetsTab->setPos(m_animator.viewXPos(m_tweetsTab), paneTop);
        m_tweetsTab->setSize(QSize(contentsWidth(), paneHeight));
    }
    if(m_mentionsTab){
        m_mentionsTab->setPos(m_animator.viewXPos(m_mentionsTab), paneTop);
        m_mentionsTab->setSize(QSize(contentsWidth(), paneHeight));
    }
    if(m_favoritesTab){
        m_favoritesTab->setPos(m_animator.viewXPos(m_favoritesTab), paneTop);
        m_favoritesTab->setSize(QSize(contentsWidth(), paneHeight));
    }

    if(m_statusView->pixmap().isNull() || contentsWidth()!=m_statusView->pixmap().width()){
        updateStatus();
        updateNameView();
    }

    this->setContentsHeight(200.f);
    updateIcons();
}

void STUserPane::contextMenuEvent(QGraphicsSceneContextMenuEvent *event){
    QMenu menu;

    if(event->reason()==QGraphicsSceneContextMenuEvent::Mouse){
        QPointF pt=event->pos();
        pt-=m_nameView->pos();
        if(m_nameView->boundingRect().contains(pt)){
            if(pt.y()<25){
                menu.addAction(m_copyNameAction);
                menu.exec(event->screenPos());
            }else{
                menu.addAction(m_copyScreenNameAction);
                menu.exec(event->screenPos());
            }
        }
    }else{
        menu.addAction(m_copyNameAction);
        menu.addAction(m_copyScreenNameAction);
        menu.exec(event->screenPos());
    }
}

void STUserPane::copyName(){
    if(!m_user)
        return;
    QApplication::clipboard()->setText(m_user->data["name"].toString());
}

void STUserPane::copyScreenName(){
    if(!m_user)
        return;
    QApplication::clipboard()->setText("@"+m_user->data["screen_name"].toString());
}

void STUserPane::tabChanged(){
    int tab=m_tabView->currentTab();
    if(tab==m_lastTab)
        return;
    int dir=(tab>m_lastTab)?1:-1;

    if(tab==0){
        Q_ASSERT(m_profileTab);
        m_animator.activateViewAnimated(m_profileTab, dir, 200);
    }else if(tab==1){
        if(!m_tweetsTab){
            m_tweetsTab=new STTimelinePane(contentsView());
            m_tweetsTab->setTimelineIdentifier(QString("%1.user.%2").arg(m_account->userId()).arg(m_userId));
        }
        m_animator.activateViewAnimated(m_tweetsTab, dir, 200);
    }else if(tab==2){
        if(!m_mentionsTab){
            m_mentionsTab=new STTimelinePane(contentsView());
            if(m_user){
                // if m_user is not ready, mention tab cannot be displayed
                m_mentionsTab->setTimelineIdentifier(QString("%1.search.@%2").arg(m_account->userId())
                                                     .arg(m_user->data["screen_name"].toString()));
            }
        }
        m_animator.activateViewAnimated(m_mentionsTab, dir, 200);
    }else if(tab==3){
        if(!m_favoritesTab){
            m_favoritesTab=new STTimelinePane(contentsView());
            m_favoritesTab->setTimelineIdentifier(QString("%1.favorite.%2").arg(m_account->userId()).arg(m_userId));
        }
        m_animator.activateViewAnimated(m_favoritesTab, dir, 200);
    }
    relayout();

    m_lastTab=tab;
}

void STUserPane::setRelationship(QVariantMap rel){
    m_relationship=rel;
    m_errorMessage.clear();
    updateStatus();

    m_animViewAnimator->setValueAnimated(0.f, 200);
   // m_animView->setVisible(false);
}

void STUserPane::setUser(STObjectManager::UserRef user, bool animated){
    if(!m_user && user){
        if(animated)
            m_displayAnimator->setValueAnimated(1.f, 200);
        else
            m_displayAnimator->setValue(1.f);
    }

    m_requestedTime=QDateTime::currentMSecsSinceEpoch();

    m_userId=user->id;
    m_user=user;
    m_profileTab->setUser(user);

    if(m_mentionsTab){
        // mention tab is created, but not initialized?
        m_mentionsTab->setTimelineIdentifier(QString("%1.search.@%2").arg(m_account->userId())
                                             .arg(m_user->data["screen_name"].toString()));
    }
    m_iconView->setUser(m_user);
    if(m_user){
        m_iconView->setZoomedImageUrl(largeAvatarUrl());
    }
    m_errorMessage.clear();
    updateStatus();
    updateNameView();

    emit headerViewContentsChanged(0);
    m_animViewAnimator->setValueAnimated(0.f, animated?200:0);
    //m_animView->setVisible(false);
}

void STUserPane::showError(QString er){
    qDebug()<<"STUserPane error: "<<er;
    m_errorMessage=er;
    updateStatus();

    m_animViewAnimator->setValueAnimated(0.f, 200);
    //m_animView->setVisible(false);
}

void STUserPane::updateNameView(){
    QRectF boundRect(0,0,contentsWidth()-49, 48);
    if(boundRect.isEmpty())
        return;

    QImage image(boundRect.width(),boundRect.height(),QImage::Format_RGB32);
    image.fill(QColor(70, 70, 70));

    QPainter painter(&image);

    if(m_user){
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setRenderHint(QPainter::HighQualityAntialiasing);

        painter.save();
        {
            QPainterPath path;
            QFont font("Stella Sans");
            font.setPixelSize(18);
            font.setWeight(QFont::Bold);
            font.setStyleHint(QFont::Helvetica, QFont::ForceOutline);
            path.addText(0,0,font, m_user->data["name"].toString());

            painter.translate(4, 21);
            float wid=path.boundingRect().right();
            float maxWid=(float)(boundRect.width()-8);
            if(wid>maxWid)
                painter.scale(maxWid/wid, 1.f);
            painter.fillPath(path, QBrush(QColor(230,230,230)));
        }
        painter.restore();

        QString scrName="@"+m_user->data["screen_name"].toString();
        {
            QPainterPath path;
            QFont font("Stella Sans");
            font.setPixelSize(14);
            font.setWeight(QFont::DemiBold);
            font.setStyleHint(QFont::Helvetica, QFont::ForceOutline);
            path.addText(4,40,font, scrName);

            painter.fillPath(path, QBrush(QColor(200, 200, 200)));
        }
        /*STFont::defaultFont()->drawString(image, QPointF(4, 32),
                                          QColor(190,190,190), scrName);*/
    }


    m_nameView->setPixmap(QPixmap::fromImage(image));
}

void STUserPane::updateIcons(){
    if(!m_user){
        m_protectedIcon->setVisible(false);
        m_verifiedIcon->setVisible(false);
        m_translatorIcon->setVisible(false);
    }else{
        const QVariantMap mp=m_user->data;
        float cx=2.f;
        float cy=50.f;
        if(mp["protected"].toBool()){
            m_protectedIcon->setVisible(true);
            m_protectedIcon->setPos(cx, cy);
            cx+=16;
        }else{
            m_protectedIcon->setVisible(false);
        }
        if(mp["verified"].toBool()){
            m_verifiedIcon->setVisible(true);
            m_verifiedIcon->setPos(cx, cy);
            cx+=16;
        }else{
            m_verifiedIcon->setVisible(false);
        }
        if(mp["is_translator"].toBool()){
            m_translatorIcon->setVisible(true);
            m_translatorIcon->setPos(cx, cy);
            cx+=16;
        }else{
            m_translatorIcon->setVisible(false);
        }
    }
}

void STUserPane::updateStatus(){
    QRectF boundRect(0,0,contentsWidth(), 20);
    if(boundRect.isEmpty())
        return;

    QImage image(boundRect.width(),boundRect.height(),QImage::Format_RGB32);
    QString msg;

    if(!m_errorMessage.isEmpty()){
        image.fill(QColor(100, 10, 20));
        msg=m_errorMessage;
    }else if(m_userId==m_account->userId() && m_user){
        image.fill(QColor(70, 70, 70));
        msg=tr("@%1 is you.").arg(m_user->data["screen_name"].toString());
    }else{
        if(m_relationship.isEmpty()){
            image.fill(QColor(70, 70, 70));
        }else{
            QVariantMap srcstate=m_relationship["source"].toMap();
            bool blocking=srcstate["blocking"].toBool();
            if(blocking){
                image.fill(QColor(100, 10, 20));
                msg=tr("Blocking");
            }else{
                bool followed=srcstate["followed_by"].toBool();
                image.fill(QColor(70, 70, 70));
                if(followed){
                    msg=tr("@%1 is following you.").arg(m_user->data["screen_name"].toString());
                }else{
                    msg=tr("@%1 is not following you.").arg(m_user->data["screen_name"].toString());
                }
            }
        }
    }

    QPainter painter(&image);

    QLinearGradient grad(0,0,0,20);
    grad.setColorAt(0, QColor(0,0,0,0));
    grad.setColorAt(1, QColor(0,0,0,30));
    //painter.fillRect(boundingRect(), grad);

    QRectF bnd=STFont::defaultFont()->boundingRectForString(msg);
    float cx=((float)boundRect.width()-bnd.right())*.5f;

    STFont::defaultFont()->drawString(image, QPointF(cx, (20-12)/2),
                                      QColor(0,0,0,60),
                                      msg);
    STFont::defaultFont()->drawString(image, QPointF(cx, (20-12)/2-1),
                                      QColor(255,255,255,255),
                                      msg);

    painter.fillRect(0,0, boundRect.width(), 1, QColor(0,0,0,36));
    painter.fillRect(0,boundRect.height()-1, boundRect.width(), 1, QColor(0,0,0,36));

    m_statusView->setPixmap(QPixmap::fromImage(image));
}

STHeaderView::Contents STUserPane::headerViewContents(){
    STHeaderView::Contents hc=STPane::headerViewContents();
    if(m_user){
        hc.title="@"+m_user->data["screen_name"].toString();
    }else{
        hc.title=QString("#%1").number(m_userId);
    }
    return hc;
}

QVariant STUserPane::itemChange(GraphicsItemChange change, const QVariant &value){
    if(change==ItemVisibleHasChanged){
        if(value.toBool()){
            startRequest();
            if(m_user){
                m_profileTab->setUser(m_user);
            }
        }
    }
    return value;
}

void STUserPane::showTweets(){
    m_tabView->setCurrentTab(1);
}

void STUserPane::showFavorites(){
    m_tabView->setCurrentTab(3);
}

QUrl STUserPane::largeAvatarUrl(){
    if(!m_user)
        return QUrl();

    QString st=m_user->data["profile_image_url"].toString();
    QRegExp reg("(.+)_normal([.][a-zA-Z0-9]+|)");
    if(reg.exactMatch(st)){
        st=reg.cap(1)+reg.cap(2);
        return QUrl(st);
    }else{
        // failed
        return QUrl(st);
    }
}

void STUserPane::showLargeAvatar(){
    QUrl ur=largeAvatarUrl();
    if(!ur.isEmpty()){
        STZoomedImageManager::sharedManager()->zoomImageWithUrl(ur, false);/*
        STObjectManager::EntityRange range;
        range.entityType="st_image";

        QVariantMap mp;
        mp.insert("media_url", ur.toString());
        mp.insert("url", ur.toString());
        range.entity=mp;

        m_iconView->setZoomedImageUrl(QUrl(ur.toString()));

        linkActivate(&range);*/
    }
}

void STUserPane::showOptions(){

}
