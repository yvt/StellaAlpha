#include "stuserprofileview.h"
#include "stbuttonview.h"
#include "stuserprofilenumberview.h"
#include <QLocale>
#include "stuserpane.h"
#include "stfollowersuserlistmodel.h"
#include "stfriendsuserlistmodel.h"
#include "stuserlistpane.h"
#include "stlinkedlabelview.h"
#include "stmath.h"
#include <QDateTime>
#include "stdateformatter.h"

STUserProfileView::STUserProfileView(STUserPane *parent) :
    STScrolledPane(parent)
{
    m_followersView=new STUserProfileNumberView(contentsView());
    m_followersView->setLabelText(":/stella/res/FollowersText.png");
    m_followersView->setToolTip(tr("Number of Followers"));

    m_friendsView=new STUserProfileNumberView(contentsView());
    m_friendsView->setLabelText(":/stella/res/FollowingText.png");
    m_friendsView->setToolTip(tr("Number of Following Users"));

    m_tweetsView=new STUserProfileNumberView(contentsView());
    m_tweetsView->setLabelText(":/stella/res/TweetsText.png");
    m_tweetsView->setToolTip(tr("Number of Tweets"));

    m_favoritesView=new STUserProfileNumberView(contentsView());
    m_favoritesView->setLabelText(":/stella/res/FavoritesText.png");
    m_favoritesView->setToolTip(tr("Number of Favorites"));

    m_listedView=new STUserProfileNumberView(contentsView());
    m_listedView->setLabelText(":/stella/res/ListedText.png");
    m_listedView->setToolTip(tr("Number of Lists User is Added to"));

    m_bioLabel=new STLinkedLabelView(contentsView());
    m_bioLabel->setBgColor(QColor(70,70,70));
    m_bioLabel->setTextColor(QColor(140,140,140));
    m_bioLabel->setContents(STFont::defaultBoldFont(),
                            tr("BIO"));

    m_bioText=new STLinkedLabelView(contentsView());
    m_bioText->setBgColor(QColor(70,70,70));
    m_bioText->setTextColor(QColor(240,240,240));
    m_bioText->setSelectable(true);
    m_bioText->setFlag(ItemIsFocusable);

    m_urlLabel=new STLinkedLabelView(contentsView());
    m_urlLabel->setBgColor(QColor(70,70,70));
    m_urlLabel->setTextColor(QColor(140,140,140));
    m_urlLabel->setContents(STFont::defaultBoldFont(),
                            tr("URL"));

    m_urlText=new STLinkedLabelView(contentsView());
    m_urlText->setBgColor(QColor(70,70,70));
    m_urlText->setTextColor(QColor(240,240,240));
    m_urlText->setSelectable(true);
    m_urlText->setFlag(ItemIsFocusable);

    m_locLabel=new STLinkedLabelView(contentsView());
    m_locLabel->setBgColor(QColor(70,70,70));
    m_locLabel->setTextColor(QColor(140,140,140));
    m_locLabel->setContents(STFont::defaultBoldFont(),
                            tr("LOC"));

    m_locText=new STLinkedLabelView(contentsView());
    m_locText->setBgColor(QColor(70,70,70));
    m_locText->setTextColor(QColor(240,240,240));
    m_locText->setSelectable(true);
    m_locText->setFlag(ItemIsFocusable);

    m_sinceLabel=new STLinkedLabelView(contentsView());
    m_sinceLabel->setBgColor(QColor(70,70,70));
    m_sinceLabel->setTextColor(QColor(140,140,140));
    m_sinceLabel->setContents(STFont::defaultBoldFont(),
                            tr("SINCE"));

    m_sinceText=new STLinkedLabelView(contentsView());
    m_sinceText->setBgColor(QColor(70,70,70));
    m_sinceText->setTextColor(QColor(240,240,240));
    m_sinceText->setSelectable(true);
    m_sinceText->setFlag(ItemIsFocusable);

    connect(m_followersView, SIGNAL(activated()),
            this, SLOT(showFollowers()));
    connect(m_friendsView, SIGNAL(activated()),
            this, SLOT(showFriends()));
    connect(m_tweetsView, SIGNAL(activated()),
            parent, SLOT(showTweets()));
    connect(m_favoritesView, SIGNAL(activated()),
            parent, SLOT(showFavorites()));
    connect(m_listedView, SIGNAL(activated()),
            this, SLOT(showAddedLists()));

    connect(m_bioText, SIGNAL(linkActivated(STObjectManager::EntityRange*,
                                            STObjectManager::LinkActivationInfo)),
            this, SLOT(linkActivate(STObjectManager::EntityRange*,
                                    STObjectManager::LinkActivationInfo)));
    connect(m_urlText, SIGNAL(linkActivated(STObjectManager::EntityRange*,
                                            STObjectManager::LinkActivationInfo)),
            this, SLOT(linkActivate(STObjectManager::EntityRange*,
                                    STObjectManager::LinkActivationInfo)));
    connect(m_locText, SIGNAL(linkActivated(STObjectManager::EntityRange*,
                                            STObjectManager::LinkActivationInfo)),
            this, SLOT(linkActivate(STObjectManager::EntityRange*,
                                    STObjectManager::LinkActivationInfo)));
    connect(m_sinceText, SIGNAL(linkActivated(STObjectManager::EntityRange*,
                                              STObjectManager::LinkActivationInfo)),
            this, SLOT(linkActivate(STObjectManager::EntityRange*,
                                    STObjectManager::LinkActivationInfo)));


    m_lastWidth=0;

}

void STUserProfileView::relayoutContents(){

    if(size().isEmpty())
        return;

    bool loaded=false;
    if(m_lastWidth!=contentsWidth() && m_user){
        m_lastWidth=contentsWidth();
        loaded=true;
    }

    int right=(int)contentsWidth()-10;
    int zero=10;
    int width=right-zero;
    int oneThird=width/3+zero;
    int twoThird=(width*2)/3+zero;
    int half=width/2+zero;

    m_followersView->setPos(zero, 10);
    m_followersView->setSize(QSize(half-zero, 43));
    m_friendsView->setPos(half-1, 10);
    m_friendsView->setSize(QSize(right-(half-1), 43));

    m_tweetsView->setPos(zero, 58);
    m_tweetsView->setSize(QSize(oneThird-zero, 43));
    m_favoritesView->setPos(oneThird-1, 58);
    m_favoritesView->setSize(QSize(twoThird-(oneThird-1), 43));
    m_listedView->setPos(twoThird-1, 58);
    m_listedView->setSize(QSize(right-(twoThird-1), 43));

    int cy=58+43+10;
    int labelPos=10;
    int textPos=0;
    textPos=qMax(textPos, (int)m_bioLabel->boundingRect().right());
    textPos=qMax(textPos, (int)m_urlLabel->boundingRect().right());
    textPos=qMax(textPos, (int)m_locLabel->boundingRect().right());
    textPos=qMax(textPos, (int)m_sinceLabel->boundingRect().right());
    textPos+=10+labelPos;

    float textWidth=(float)(right-textPos);

    m_bioLabel->setPos(labelPos, cy+3);
    if(loaded){
        QMutexLocker locker(&(m_user->mutex));
        m_bioText->setContents(STFont::defaultFont(),
                               m_user->displayDescription,
                               m_user->descriptionEntityRanges,
                               textWidth, false, 3.f);
    }
    m_bioText->setPos(textPos, (float)cy-m_bioText->boundingRect().top());

    cy+=(int)qMax(m_bioText->boundingRect().height(),m_bioLabel->boundingRect().bottom());
    cy+=10;


    m_urlLabel->setPos(labelPos, cy+3);
    if(loaded){
        QMutexLocker locker(&(m_user->mutex));
        m_urlText->setContents(STFont::defaultFont(),
                               m_user->displayUrl,
                               m_user->urlEntityRanges,
                               textWidth, false, 3.f);
    }
    m_urlText->setPos(textPos, (float)cy-m_urlText->boundingRect().top());

    cy+=(int)qMax(m_urlText->boundingRect().height(),m_urlLabel->boundingRect().bottom());
    cy+=10;


    m_locLabel->setPos(labelPos, cy+3);
    if(loaded){
        QString text;
        {
            QMutexLocker locker(&(m_user->mutex));
            text=m_user->data["location"].toString();
        }
        m_locText->setContents(STFont::defaultFont(),
                               text,
                               textWidth, false, 3.f);
    }
    m_locText->setPos(textPos, (float)cy-m_locText->boundingRect().top());

    cy+=(int)qMax(m_locText->boundingRect().height(),m_locLabel->boundingRect().bottom());
    cy+=10;


    m_sinceLabel->setPos(labelPos, cy+3);
    if(loaded){
        QString text;
        {
            QMutexLocker locker(&(m_user->mutex));
            text=m_user->data["created_at"].toString();
        }
        QDateTime dat=STParseDateTimeRFC2822(text);
        text=STDateFormatter::toLongDateTimeString(dat); //QLocale::system().toString(dat, QLocale::ShortFormat);
        m_sinceText->setContents(STFont::defaultFont(),
                               text,
                                 textWidth, false, 3.f);
    }
    m_sinceText->setPos(textPos, (float)cy-m_sinceText->boundingRect().top());

    cy+=(int)qMax(m_sinceText->boundingRect().height(),m_sinceLabel->boundingRect().bottom());
    cy+=10;


    setContentsHeight(cy);
}

void STUserProfileView::setUser(STObjectManager::UserRef user){
    m_user=user;

    {
        QLocale loc=QLocale::system();
        QVariantMap data;
        {
            QMutexLocker locker(&(user->mutex));
            data=user->data;
        }
        m_followersView->setNumberText(loc.toString(data["followers_count"].toULongLong()));
        m_friendsView->setNumberText(loc.toString(data["friends_count"].toULongLong()));
        m_tweetsView->setNumberText(loc.toString(data["statuses_count"].toULongLong()));
        m_favoritesView->setNumberText(loc.toString(data["favourites_count"].toULongLong()));
        m_listedView->setNumberText(loc.toString(data["listed_count"].toULongLong()));

    }

    m_lastWidth=0;
    relayout();
}

void STUserProfileView::showFollowers(){
    if(!isPaneActive())
        return;
    STUserPane *p=dynamic_cast<STUserPane *>(parentItem());
    Q_ASSERT(p!=NULL);
    STUserListModel *model=new STFollowersUserListModel(p->account(), m_user->id);
    STUserListPane *pane=new STUserListPane(model);
    model->setParent(pane);

    pushPaneAnimated(pane);
}

void STUserProfileView::showFriends(){
    if(!isPaneActive())
        return;
    STUserPane *p=dynamic_cast<STUserPane *>(parentItem());
    Q_ASSERT(p!=NULL);
    STUserListModel *model=new STFriendsUserListModel(p->account(), m_user->id);
    STUserListPane *pane=new STUserListPane(model);
    model->setParent(pane);

    pushPaneAnimated(pane);
}

void STUserProfileView::showAddedLists(){

}
