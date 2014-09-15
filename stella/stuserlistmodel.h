#ifndef STUSERLISTMODEL_H
#define STUSERLISTMODEL_H

#include "ststdafx.h"
#include <QObject>
#include "stobjectmanager.h"

class STUserListModelFetcher;
class STUserListModelLoader;
class STAccount;

class STUserListModel : public QObject
{
    Q_OBJECT
public:
    explicit STUserListModel(STAccount *,QObject *parent = 0);
    virtual ~STUserListModel();

    struct Page{
        QSet<quint64> userIds;
        QMap<quint64, STObjectManager::UserRef> users;
        bool loading;

        bool isLoaded() const{
            return !users.isEmpty();
        }
    };

    struct UserIdItem{
        quint64 userId;
        int pageId;
    };

    // these are both equal
    qint64 userIdsCount() const{return m_userIds.count();}
    qint64 rowsCount() const{return m_rowIds.count();}

    bool hasMoreUserIds() const;

    qint64 indexForUserId(quint64) const;
    qint64 rowIdForUserId(quint64) const;
    STObjectManager::UserRef userAtIndex(qint64);
    STObjectManager::UserRef userAtRowId(qint64);
    quint64 userIdAtIndex(qint64 index) const{return m_userIds[m_rowIds[index]].userId;}
    quint64 userIdAtRowId(qint64 rowId) const{return m_userIds[rowId].userId;}
    qint64 rowIdAtIndex(qint64 index) const{return m_rowIds[index];}

    void setFetchEndpointUrl(QUrl u){m_url=u;}

    // fetch = loading user ID (user objects too for some endpoints)
    void fetchMore();
    bool isFetching() const{return m_fetching;}

    // load = loading user objects
    void loadPageForRowId(quint64 rowId);

signals:

    void someUsersLoaded();

    void userIdsPrepended(quint64 count);
    void userIdsAppended(qint64 count);
    void userIdRemoved(qint64 index, qint64 rowId, quint64 userId);

    void userIdFetchDone();
    void userIdFetchFailed();

protected slots:

    void userIdsFetched(QList<quint64>);
    void usersFetched(QList<STObjectManager::UserRef>);
    void fetchFailed(QString);
    void streamUserIdAdded(quint64);
    void streamUserAdded(STObjectManager::UserRef);
    void streamUserIdRemoved(quint64);
    void streamUserRemoved(STObjectManager::UserRef);

    void pageLoaded(int pageId, STObjectManager::UserRefList);
    void pageLoadFailed(QString ,int pageId);

private:
    STUserListModelFetcher *m_fetcher;
    STUserListModelLoader *m_loader;
    bool m_fetching;

    QUrl m_url;

    STAccount *m_account;

    QMap<qint64, UserIdItem> m_userIds;
    QList<qint64> m_rowIds;
    QList<Page> m_pages;
    QLinkedList<int> m_loadedPages;
    QSet<quint64> m_containingUserIds;
    int m_maxPageUserIds;
    int m_maxLoadedPages;

    int findFreePage(bool loaded=false);
    void pageLoadedOrUsed(int pageId);

    qint64 lastRowId();
    qint64 firstRowId();

    void loadPage(int pageId);

};

#endif // STUSERLISTMODEL_H
