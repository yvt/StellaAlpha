#include "stuserlistmodel.h"
#include "stuserlistmodelfetcher.h"
#include "stuserlistmodelloader.h"
#include "staccount.h"
#include <QDebug>

STUserListModel::STUserListModel(STAccount *account, QObject *parent) :
    QObject(parent), m_account(account)
{
    m_maxPageUserIds=25;
    m_maxLoadedPages=16;

    m_fetcher=new STUserListModelFetcher();
    m_fetching=false;

    m_loader=new STUserListModelLoader();

    connect(m_fetcher, SIGNAL(userIdsFetched(QList<quint64>)),
            this, SLOT(userIdsFetched(QList<quint64>)));
    connect(m_fetcher, SIGNAL(usersFetched(QList<STObjectManager::UserRef>)),
            this, SLOT(usersFetched(QList<STObjectManager::UserRef>)));
    connect(m_fetcher, SIGNAL(fetchFailed(QString)),
            this, SLOT(fetchFailed(QString)));
    connect(m_loader, SIGNAL(loaded(int,STObjectManager::UserRefList)),
            this, SLOT(pageLoaded(int,STObjectManager::UserRefList)));
    connect(m_loader, SIGNAL(loadFailed(QString,int)),
            this, SLOT(pageLoadFailed(QString,int)));

    qRegisterMetaType<QList<quint64> >("QList<quint64>");
    qRegisterMetaType<QList<qint64> >("QList<qint64>");
    qRegisterMetaType<QSet<quint64> >("QSet<quint64>");
}

STUserListModel::~STUserListModel(){
    m_fetcher->blockSignals(true);
    m_loader->blockSignals(true);
    //QThread *th=m_fetcher->thread();
    m_fetcher->deleteLater();;
    //th->wait();
    //delete th;

    //th=m_loader->thread();
    m_loader->deleteLater();
    //th->wait();
    //delete th;
}

int STUserListModel::findFreePage(bool loaded){
    // check last page, if exists
    if(!m_pages.isEmpty()){
        Page& page=m_pages.last();
        if(page.userIds.count()<m_maxPageUserIds &&
                page.isLoaded()==loaded &&
                !page.loading){
            return m_pages.count()-1;
        }
    }
    if(m_pages.count()>=2){
        Page& page=m_pages[m_pages.count()-2];
        if(page.userIds.count()<m_maxPageUserIds &&
                page.isLoaded()==loaded &&
                !page.loading){
            return m_pages.count()-2;
        }
    }

    // new page
    Page page;
    page.loading=false;
    m_pages.push_back(page);
    return m_pages.count()-1;
}

qint64 STUserListModel::indexForUserId(quint64 userId) const{
    qint64 rowId=rowIdForUserId(userId);
    if(rowId==-1)
        return -1;
    for(int i=0;i<m_rowIds.count();i++){
        if(m_rowIds[i]==rowId)
            return i;
    }
    Q_ASSERT(false);
    return -1;
}

qint64 STUserListModel::rowIdForUserId(quint64 userId) const{
    for(QMap<qint64, UserIdItem>::const_iterator it=m_userIds.begin();
        it!=m_userIds.end();it++){
        if(it.value().userId==userId)
            return it.key();
    }
    return -1;
}

qint64 STUserListModel::lastRowId(){
    if(m_rowIds.isEmpty())
        return 0x4000000000000000LL;
    else
        return m_rowIds.last();
}

qint64 STUserListModel::firstRowId(){
    if(m_rowIds.isEmpty())
        return 0x4000000000000000LL;
    else
        return m_rowIds.first();
}

void STUserListModel::userIdsFetched(QList<quint64> userIds){
    emit userIdFetchDone();
    qDebug()<<userIds.count()<<" user IDs fetched";
    foreach(quint64 userId, userIds){

        if(m_containingUserIds.contains(userId))
            return;

        m_containingUserIds.insert(userId);

        UserIdItem item;
        item.userId=userId;
        item.pageId=findFreePage(false);

        Page& page=m_pages[item.pageId];
        page.userIds.insert(userId);

        qint64 rowId=lastRowId()+1;
        m_userIds.insert(rowId, item);
        m_rowIds.append(rowId);

        pageLoadedOrUsed(item.pageId);
    }

    emit userIdsAppended(userIds.count());
}

void STUserListModel::usersFetched(QList<STObjectManager::UserRef> users){
    emit userIdFetchDone();
    qDebug()<<users.count()<<" user objects fetched";
    foreach(STObjectManager::UserRef user, users){
        if(m_containingUserIds.contains(user->id))
            return;

        m_containingUserIds.insert(user->id);

        UserIdItem item;
        item.userId=user->id;
        item.pageId=findFreePage(true);

        Page& page=m_pages[item.pageId];
        page.userIds.insert(user->id);
        page.users.insert(user->id, user);

        qint64 rowId=lastRowId()+1;
        m_userIds.insert(rowId, item);
        m_rowIds.append(rowId);

        pageLoadedOrUsed(item.pageId);
    }

    emit userIdsAppended(users.count());
}

void STUserListModel::fetchFailed(QString msg){
    m_account->reportError(msg);
    emit userIdFetchFailed();

}

void STUserListModel::streamUserIdAdded(quint64 userId){
    if(m_containingUserIds.contains(userId))
        return;

    m_containingUserIds.insert(userId);

    UserIdItem item;
    item.userId=userId;
    item.pageId=findFreePage(false);

    Page& page=m_pages[item.pageId];
    page.userIds.insert(userId);

    qint64 rowId=firstRowId()-1;
    m_userIds.insert(rowId, item);
    m_rowIds.prepend(rowId);

    pageLoadedOrUsed(item.pageId);

    emit userIdsPrepended(1);
}

void STUserListModel::streamUserAdded(STObjectManager::UserRef user){
    if(m_containingUserIds.contains(user->id))
        return;

    m_containingUserIds.insert(user->id);

    UserIdItem item;
    item.userId=user->id;
    item.pageId=findFreePage(true);

    Page& page=m_pages[item.pageId];
    page.userIds.insert(user->id);
    page.users.insert(user->id, user);

    qint64 rowId=firstRowId()-1;
    m_userIds.insert(rowId, item);
    m_rowIds.prepend(rowId);

    pageLoadedOrUsed(item.pageId);

    emit userIdsPrepended(1);
}

void STUserListModel::streamUserIdRemoved(quint64 userId){
    qint64 index=indexForUserId(userId);
    if(index==-1)
        return;

    m_containingUserIds.remove(userId);

    qint64 rowId=m_rowIds[index];

    UserIdItem& item=m_userIds[rowId];
    Q_ASSERT(item.userId==userId);

    Page& page=m_pages[item.pageId];
    page.userIds.remove(userId);
    page.users.remove(userId);

    m_userIds.remove(rowId);
    m_rowIds.removeAt(index);

    emit userIdRemoved(index, rowId, userId);
}

void STUserListModel::streamUserRemoved(STObjectManager::UserRef user){
    streamUserIdRemoved(user->id);
}

void STUserListModel::pageLoadedOrUsed(int pageId){
    Q_ASSERT(pageId>=0);
    Q_ASSERT(pageId<m_pages.count());

    //Page& page=m_pages[pageId];
    if(!m_loadedPages.isEmpty()){
        if(m_loadedPages.last()==pageId)
            // already loaded, and most recently used.
            return;
        else{
            QLinkedList<int>::iterator it=qFind(m_loadedPages.begin(), m_loadedPages.end(), pageId);
            if(it!=m_loadedPages.end()){
                // it was not the most recently used one, but now it is.
                m_loadedPages.erase(it);
                m_loadedPages.push_back(pageId);
                return;
            }
        }
    }

    // newly loaded.
    m_loadedPages.push_back(pageId);

    // too many loaded pages?
    while(m_loadedPages.count()>m_maxLoadedPages){
        // unload the least recently used page.
        Page& page=m_pages[m_loadedPages.takeFirst()];
        page.users.clear();
        Q_ASSERT(!page.isLoaded());
    }


}

bool STUserListModel::hasMoreUserIds() const{
    return m_fetcher->hasMore();
}

void STUserListModel::fetchMore(){
    if(!hasMoreUserIds())
        return;
    if(m_fetching)
        return;

    m_fetcher->doRequest(m_url, m_account);
}

STObjectManager::UserRef STUserListModel::userAtRowId(qint64 rowId){
    UserIdItem& item=m_userIds[rowId];
    Page& page=m_pages[item.pageId];
    if(page.users.contains(item.userId)){
        return page.users[item.userId];
    }else{
        return STObjectManager::UserRef();
    }
}

STObjectManager::UserRef STUserListModel::userAtIndex(qint64 index){
    return userAtRowId(m_rowIds[index]);
}

void STUserListModel::loadPageForRowId(quint64 rowId){
    UserIdItem& item=m_userIds[rowId];
    loadPage(item.pageId);
}

void STUserListModel::loadPage(int pageId){
    Page& page=m_pages[pageId];
    if(page.loading||page.isLoaded())
        return;
    if(page.userIds.isEmpty())
        return;

    m_loader->doRequest(page.userIds, m_account, pageId);
    page.loading=true;
}

void STUserListModel::pageLoaded(int pageId, QList<STObjectManager::UserRef> users){
    Page& page=m_pages[pageId];
    qDebug()<<users.count()<<" users loaded for page "<<pageId;
    foreach(STObjectManager::UserRef user, users){
        page.users.insert(user->id, user);
    }

    pageLoadedOrUsed(pageId);
    emit someUsersLoaded();
}

void STUserListModel::pageLoadFailed(QString msg, int pageId){
    Page& page=m_pages[pageId];
    page.loading=false;
    m_account->reportError(msg);
}

