#include "ststdafx.h"
#include "stnewslistview.h"
#include "stuserstreamer.h"
#include "staccountsmanager.h"
#include "staccount.h"
#include "stnewslistitemview.h"
#include <QDebug>
#include "stzoomedimagedownloader.h"
#include "sttimeline.h"
#include "stmentionstimeline.h"

STNewsListView::STNewsListView(QGraphicsItem *parent) :
    STDynamicListView(parent)
{
    foreach(STAccount *account, STAccountsManager::sharedManager()->accounts()){
        connectAccount(account);
    }

    connect(STZoomedImageDownloader::sharedDownloader(),
            SIGNAL(imageLoadFailed(QUrl,QString)),
            this, SLOT(imageLoadFailed(QUrl,QString)));

    m_emptyItemId=1;
    m_nextItemId=2;
    m_itemHeight=20;

    this->scrollBarView()->setStyle(STScrollBarView::ClearBlack);
    this->insertItem(m_emptyItemId, (double)m_itemHeight);

}

void STNewsListView::addNewsEntry(NewsEntry entry){
    // there might be a duplicate entry.
    int searchedCount=96;
    QMap<quint64, NewsEntry>::iterator it=m_entries.end();
    while(it!=m_entries.begin() && searchedCount>0){
        it--;

        const NewsEntry& ent=it.value();
        if(ent.message==entry.message && ent.account==entry.account &&
                ent.type==entry.type){
            // duplicate!
            qint64 diff=ent.timestamp.msecsTo(entry.timestamp);
            if(diff<0)diff=-diff;
            if(diff<1000){
                // ignore.
                return;
            }else{
                // remove old one.
                this->removeItem(it.key());
                m_entries.erase(it);
            }
            break;
        }

        searchedCount--;
    }

    entry.timestamp=QDateTime::currentDateTime();
    quint64 id=m_nextItemId;
    m_entries.insert(id, entry);

    this->insertItem(id, m_itemHeight);
    m_nextItemId++;

    if(m_emptyItemId){
        this->removeItem(m_emptyItemId);
        m_emptyItemId=0;
    }
}

QGraphicsItem *STNewsListView::createView(quint64 id, double height, STClipView *parent){
    int width=size().width();
    if(id==m_emptyItemId){
        QSize size(width, (int)height);
        QImage img(size, QImage::Format_RGB32);
        img.fill(QColor(90,90,90));


        static const QString text=tr("No notifications available");
        STFont::defaultFont()->drawString(img, QPointF(4, (height-12.f)/2.f),
                                          QColor(255,255,255,100), text);


        QGraphicsPixmapItem *item=new QGraphicsPixmapItem(QPixmap::fromImage(img), parent);

        return item;
    }else{
        QMap<quint64, NewsEntry>::iterator it=m_entries.find(id);
        if(it==m_entries.end()){
           // qWarning()<<"STNewsListView::createView: item request for purged news entry "<<id;
            return NULL;
        }

        NewsEntry *entry=&(it.value());
        STNewsListItemView *v=new STNewsListItemView(entry, QSize(width, (int)height), parent);
        v->setTriggerTime(entry->timestamp);
        connect(v, SIGNAL(linkActivated(STObjectManager::EntityRange*,
                                        STObjectManager::LinkActivationInfo)),
                this, SLOT(itemLinkActivated(STObjectManager::EntityRange*,
                                             STObjectManager::LinkActivationInfo)));
        return v;
    }
}

void STNewsListView::itemWasPurged(quint64 id, double ){
    m_entries.remove(id);
}

void STNewsListView::accountWasAdded(quint64 id){
    connectAccount(STAccountsManager::sharedManager()->account(id));
}

void STNewsListView::itemLinkActivated(STObjectManager::EntityRange *er,
                                       STObjectManager::LinkActivationInfo i){
    emit linkActivated(er,i);
}

void STNewsListView::connectAccount(STAccount *account){
    connect(account, SIGNAL(errorReported(QString,STAccount*)),
            this, SLOT(errorReported(QString,STAccount*)));

    STUserStreamer *streamer=account->userStreamer();
    connect(streamer, SIGNAL(statusFavorited(STObjectManager::StatusRef,STObjectManager::UserRef)),
            this, SLOT(favorited(STObjectManager::StatusRef,STObjectManager::UserRef)));
    connect(streamer, SIGNAL(statusRetweeted(STObjectManager::StatusRef)),
            this, SLOT(retweeted(STObjectManager::StatusRef)));
    connect(streamer, SIGNAL(userSubscribedToList(STObjectManager::ListRef,STObjectManager::UserRef)),
            this, SLOT(listSubscribed(STObjectManager::ListRef,STObjectManager::UserRef)));
    connect(streamer, SIGNAL(userFollowedAccount(STObjectManager::UserRef,STObjectManager::UserRef)),
            this, SLOT(followed(STObjectManager::UserRef,STObjectManager::UserRef)));
    connect(streamer, SIGNAL(accountAddedToList(STObjectManager::ListRef,STObjectManager::UserRef)),
            this, SLOT(addedToList(STObjectManager::ListRef,STObjectManager::UserRef)));

    STTimeline *mentionsTimeline=account->mentionsTimeline();
    connect(mentionsTimeline, SIGNAL(rowAdded(quint64)),
            this, SLOT(mentionRowAdded(quint64)));
}

void STNewsListView::errorReported(QString msg, STAccount *account){
    NewsEntry entry;
    entry.type=Error;
    entry.message=msg;
    entry.account=account->userId();
    addNewsEntry(entry);
}

void STNewsListView::favorited(STObjectManager::StatusRef status, STObjectManager::UserRef userBy){
    STAccount *account=STAccountsManager::sharedManager()->account(status->user->id);
    if(!account)
        return;

    NewsEntry entry;
    {
        QString text;
        {
            QString un="@"+userBy->data["screen_name"].toString();
            static QString fmt=tr("%1 fav'd ");
            STObjectManager::EntityRange range;
            range.charIndexStart=text.length()+fmt.indexOf(QLatin1String("%1"));
            range.charIndexEnd=range.charIndexStart+un.length();
            range.entityType="user_stobject";
            range.stObject=userBy;
            entry.links.append(range);
            text+=fmt.arg(un);
        }
        {
            QString un=status->displayText;
            STObjectManager::EntityRange range;
            range.charIndexStart=text.length();
            range.charIndexEnd=range.charIndexStart+un.length();
            range.entityType="status_stobject";
            range.stObject=status;
            entry.links.append(range);
            text+=un;
        }
        entry.message=text;
    }
    entry.account=account->userId();
    entry.type=Favorite;
    addNewsEntry(entry);
}

void STNewsListView::retweeted(STObjectManager::StatusRef rtStatus){
    Q_ASSERT(rtStatus->retweetedStatus);
    STAccount *account=STAccountsManager::sharedManager()->account(rtStatus->retweetedStatus->user->id);
    if(!account)
        return;

    NewsEntry entry;
    {
        QString text;
        {
            QString un="@"+rtStatus->user->data["screen_name"].toString();
            static QString fmt=tr("%1 RT'd ");
            STObjectManager::EntityRange range;
            range.charIndexStart=text.length()+fmt.indexOf(QLatin1String("%1"));
            range.charIndexEnd=range.charIndexStart+un.length();
            range.entityType="user_stobject";
            range.stObject=rtStatus->user;
            entry.links.append(range);
            text+=fmt.arg(un);
        }
        {
            QString un=rtStatus->retweetedStatus->displayText;
            STObjectManager::EntityRange range;
            range.charIndexStart=text.length();
            range.charIndexEnd=range.charIndexStart+un.length();
            range.entityType="status_stobject";
            range.stObject=rtStatus->retweetedStatus;
            entry.links.append(range);
            text+=un;
        }
        entry.message=text;
    }
    entry.account=account->userId();
    entry.type=Retweet;
    addNewsEntry(entry);
}

void STNewsListView::listSubscribed(STObjectManager::ListRef lst, STObjectManager::UserRef byUser){
    STAccount *account=STAccountsManager::sharedManager()->account(lst->owner->id);
    if(!account)
        return;

    NewsEntry entry;
    {
        QString text;
        {
            QString un="@"+byUser->data["screen_name"].toString();
            static QString fmt=tr("%1 subscribed ");
            STObjectManager::EntityRange range;
            range.charIndexStart=text.length()+fmt.indexOf(QLatin1String("%1"));
            range.charIndexEnd=range.charIndexStart+un.length();
            range.entityType="user_stobject";
            range.stObject=byUser;
            entry.links.append(range);
            text+=fmt.arg(un);
        }
        {
            QString un=lst->data["name"].toString();
            STObjectManager::EntityRange range;
            range.charIndexStart=text.length();
            range.charIndexEnd=range.charIndexStart+un.length();
            range.entityType="list_stobject";
            range.stObject=lst;
            entry.links.append(range);
            text+=un;
        }
        entry.message=text;
    }
    entry.account=account->userId();
    entry.type=ListSubscribed;
    addNewsEntry(entry);
}

void STNewsListView::followed(STObjectManager::UserRef byUser, STObjectManager::UserRef thisUser){
    STAccount *account=STAccountsManager::sharedManager()->account(thisUser->id);
    if(!account)
        return;

    NewsEntry entry;
    {
        QString text;
        {
            QString un="@"+byUser->data["screen_name"].toString();
            static QString fmt=tr("%1 followed you.");
            STObjectManager::EntityRange range;
            range.charIndexStart=text.length()+fmt.indexOf(QLatin1String("%1"));
            range.charIndexEnd=range.charIndexStart+un.length();
            range.entityType="user_stobject";
            range.stObject=byUser;
            entry.links.append(range);
            text+=fmt.arg(un);
        }
        entry.message=text;
    }
    entry.account=account->userId();
    entry.type=Follow;
    addNewsEntry(entry);
}

void STNewsListView::addedToList(STObjectManager::ListRef lst, STObjectManager::UserRef thisUser){
    STAccount *account=STAccountsManager::sharedManager()->account(thisUser->id);
    if(!account)
        return;

    NewsEntry entry;
    {
        QString text;
        {
            QString un="@"+lst->owner->data["screen_name"].toString();
            static QString fmt=tr("%1 added you to ");
            STObjectManager::EntityRange range;
            range.charIndexStart=text.length()+fmt.indexOf(QLatin1String("%1"));
            range.charIndexEnd=range.charIndexStart+un.length();
            range.entityType="user_stobject";
            range.stObject=lst->owner;
            entry.links.append(range);
            text+=fmt.arg(un);
        }
        {
            QString un=lst->data["name"].toString();
            STObjectManager::EntityRange range;
            range.charIndexStart=text.length();
            range.charIndexEnd=range.charIndexStart+un.length();
            range.entityType="list_stobject";
            range.stObject=lst;
            entry.links.append(range);
            text+=un;
        }
        entry.message=text;
    }
    entry.account=account->userId();
    entry.type=AddedToList;
    addNewsEntry(entry);
}

void STNewsListView::imageLoadFailed(QUrl, QString err){
    NewsEntry entry;
    entry.type=Error;
    entry.message=err;
    entry.account=0;
    addNewsEntry(entry);
}

void STNewsListView::mentionRowAdded(quint64 rowId){
    STTimeline *timeline=dynamic_cast<STTimeline *>(sender());
    if(!timeline)
        return;

    STAccount *account=timeline->accountForIdentifier(timeline->identifier());
    if(!account)
        return;

    const STTimeline::Row *row=timeline->row(rowId);
    if(!row)
        return;

    STObjectManager::StatusRef status=row->status;
    if(!status)
        return;

    if(status->retweetedStatus)
        return;

    NewsEntry entry;
    {
        QString text;
        {
            QString un="@"+status->user->data["screen_name"].toString();
            static QString fmt=tr("%1: ");
            STObjectManager::EntityRange range;
            range.charIndexStart=text.length()+fmt.indexOf(QLatin1String("%1"));
            range.charIndexEnd=range.charIndexStart+un.length();
            range.entityType="user_stobject";
            range.stObject=status->user;
            entry.links.append(range);
            text+=fmt.arg(un);
        }
        {
            QString un=status->displayText;
            STObjectManager::EntityRange range;
            range.charIndexStart=text.length();
            range.charIndexEnd=range.charIndexStart+un.length();
            range.entityType="status_stobject";
            range.stObject=status;
            entry.links.append(range);
            text+=un;
        }
        entry.message=text;
    }
    entry.account=account->userId();
    entry.type=Tweet;
    addNewsEntry(entry);
}
