#include "ststatusmenu.h"
#include "stactionmanager.h"
#include "sttweetview.h"
#include "stbasetabpageview.h"
#include "ststandardview.h"
#include "staccount.h"
#include "staccountsmanager.h"
#include <QDebug>
#include <QApplication>
#include <QClipboard>
#include <QDesktopServices>

STStatusMenu::STStatusMenu(STBaseTabPageView *tabPage,
                           STObjectManager::StatusRef status,
                           MenuType type) :
    QMenu(NULL),m_tabPage(tabPage),m_status(status)
{
    if(m_status->retweetedStatus)
        m_status=m_status->retweetedStatus;

    m_account=tabPage->account();
    m_accountId=m_account->userId();

    if(type==Retweet){
        QAction *retweetAction=new QAction(tr("Retweet"), this);
        QAction * undoRetweetAction=new QAction(tr("Undo Retweet"), this);
        QAction *quoteRetweetAction=new QAction(tr("Quote Retweet..."), this);

        connect(retweetAction, SIGNAL(triggered()),
                this, SLOT(retweet()));
        connect(undoRetweetAction, SIGNAL(triggered()),
                this, SLOT(undoRetweet()));
        connect(quoteRetweetAction, SIGNAL(triggered()),
                this, SLOT(quoteRetweet()));

        STObjectManager::StatusRef ref=m_status;
        if(ref->retweetStatusId(m_accountId)){
            // already retweeted.
            addAction(undoRetweetAction);
        }else if(ref->user->id==m_accountId ||
                ref->user->data["protected"].toBool()){
            // cannot RT.
            retweetAction->setEnabled(false);
            addAction(retweetAction);
        }else{
            retweetAction->setEnabled(true);
            addAction(retweetAction);
        }
        addAction(quoteRetweetAction);
    }else if(type==Options||
             type==OptionsWithoutViewDetail){
        QAction *viewDetailAction=new QAction(tr("View Details"), this);
        QAction *viewConversationAction=new QAction(tr("View Conversations"), this);
        QAction *viewRetweetsAction=new QAction(tr("View Retweets"), this);
        QAction *deleteTweetAction=new QAction(tr("Delete Tweet"), this);
        QAction *copyTweetAction=new QAction(tr("Copy Tweet"), this);
        QAction *copyLinkToTweetAction=new QAction(tr("Copy Link to Tweet"), this);
        QAction *viewTweetInWebsie=new QAction(tr("View in Twitter.com"), this);


        connect(viewDetailAction, SIGNAL(triggered()),
                this, SLOT(viewDetail()));
        connect(viewConversationAction, SIGNAL(triggered()),
                this, SLOT(viewConversation()));
        connect(viewRetweetsAction, SIGNAL(triggered()),
                this, SLOT(viewRetweets()));
        connect(deleteTweetAction, SIGNAL(triggered()),
                this, SLOT(deleteTweet()));
        connect(copyTweetAction, SIGNAL(triggered()),
                this, SLOT(copyTweet()));
        connect(copyLinkToTweetAction, SIGNAL(triggered()),
                this, SLOT(copyLinkToTweet()));
        connect(viewTweetInWebsie, SIGNAL(triggered()),
                this, SLOT(viewTweetInWebsite()));

        if(type!=OptionsWithoutViewDetail)
            addAction(viewDetailAction);
        // menu.addAction(viewConversationAction); not available in API 1.1
        //addAction(viewRetweetsAction);
        addSeparator();
        if(!STAccountsManager::sharedManager()->account(m_status->user->id)){
            deleteTweetAction->setEnabled(false);
        }
        addAction(deleteTweetAction);

        addSeparator();

        addAction(copyTweetAction);
        addAction(copyLinkToTweetAction);

        addSeparator();

        addAction(viewTweetInWebsie);

    }
}

void STStatusMenu::retweet(){
    STObjectManager::StatusRef ref=m_status;
    QMutexLocker locker(&(ref->mutex));

    STActionManager::sharedManager()->statusAction(ref->id, m_accountId,
                                                   STActionManager::Retweet);
}


void STStatusMenu::undoRetweet(){
    STObjectManager::StatusRef ref=m_status;

    QMutexLocker locker(&(ref->mutex));
    quint64 retweetStatusId=ref->retweetStatusId(m_accountId);
    if(!retweetStatusId){
        qWarning()<<"STStatusMenu::undoRetweet: retweetStatusId==0";
        return;
    }
    STActionManager::sharedManager()->statusAction(retweetStatusId, m_accountId,
                                                   STActionManager::DeleteStatus);
}


STTweetView *STStatusMenu::tweetView() {
    QGraphicsItem *item=m_tabPage;
    while(item){
        STStandardView *vw=dynamic_cast<STStandardView *>(item);
        if(vw)
            return vw->tweetView();
        item=item->parentItem();
    }
    return NULL;
}


void STStatusMenu::quoteRetweet(){
    STTweetView *twv=tweetView();
    if(!twv){
        qWarning()<<"STStatusMenu::reply: tweetView()==NULL";
        return;
    }

    STObjectManager::StatusRef ref=m_status;


    twv->quoteRetweet(ref);
}


void STStatusMenu::viewDetail(){
    STObjectManager::StatusRef ref=m_status;
    STObjectManager::EntityRange range;
    range.charIndexStart=0;
    range.charIndexEnd=0;
    range.entityType="status_stobject";
    range.stObject=ref;

    STObjectManager::LinkActivationInfo info;
    m_tabPage->linkActivate(&range,info);
}

void STStatusMenu::viewConversation(){

}

void STStatusMenu::viewRetweets(){

}

void STStatusMenu::deleteTweet(){
    STObjectManager::StatusRef ref=m_status;
    QMutexLocker locker(&(ref->mutex));
    Q_ASSERT(STAccountsManager::sharedManager()->account(m_status->user->id));
    STActionManager::sharedManager()->statusAction(ref->id, m_status->user->id,
                                                   STActionManager::DeleteStatus);
}

void STStatusMenu::copyTweet(){
    QApplication::clipboard()->setText(m_status->displayText);
}

void STStatusMenu::copyLinkToTweet(){
    QString tmpl="https://twitter.com/%1/status/%2";
    tmpl=tmpl.arg(m_status->user->data["screen_name"].toString());
    tmpl=tmpl.arg(m_status->id);
    QApplication::clipboard()->setText(tmpl);
}

void STStatusMenu::viewTweetInWebsite(){
    QString tmpl="https://twitter.com/%1/status/%2";
    tmpl=tmpl.arg(m_status->user->data["screen_name"].toString());
    tmpl=tmpl.arg(m_status->id);
    QDesktopServices::openUrl(QUrl(tmpl));
}
