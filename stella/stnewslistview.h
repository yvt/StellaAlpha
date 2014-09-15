#ifndef STNEWSLISTVIEW_H
#define STNEWSLISTVIEW_H

#include "ststdafx.h"
#include "stdynamiclistview.h"
#include "stobjectmanager.h"

class STUserStreamer;
class STAccount;

class STNewsListView : public STDynamicListView
{
    Q_OBJECT

    void connectAccount(STAccount *);
public:
    explicit STNewsListView(QGraphicsItem *parent = 0);
    
    enum NewsType{
        NoType,
        Error,
        Favorite,
        Retweet,
        Follow,
        AddedToList,
        ListSubscribed,
        Tweet
    };;

    struct NewsEntry{
        QDateTime timestamp;
        QString message;
        QList<STObjectManager::EntityRange> links;
        NewsType type;
        quint64 account;
    };

    void addNewsEntry(NewsEntry entry);

private:

    quint64 m_emptyItemId;
    quint64 m_nextItemId;
    int m_itemHeight;

    QMap<quint64, NewsEntry> m_entries;


protected:
    virtual QGraphicsItem *createView(quint64 id, double height, STClipView *parent);
    virtual void itemWasPurged(quint64 id, double height);  // deleted

signals:
    void linkActivated(STObjectManager::EntityRange *,
                       STObjectManager::LinkActivationInfo);
private slots:
    void accountWasAdded(quint64);
    void itemLinkActivated(STObjectManager::EntityRange *,
                           STObjectManager::LinkActivationInfo);

    void errorReported(QString, STAccount *account);
    void favorited(STObjectManager::StatusRef, STObjectManager::UserRef);
    void retweeted(STObjectManager::StatusRef);
    void listSubscribed(STObjectManager::ListRef, STObjectManager::UserRef);
    void addedToList(STObjectManager::ListRef, STObjectManager::UserRef accountUser);
    void followed(STObjectManager::UserRef byUser, STObjectManager::UserRef thisUser);
    void mentionRowAdded(quint64);

    void imageLoadFailed(QUrl, QString);

};

#endif // STNEWSLISTVIEW_H
