#include "stuserlistview.h"
#include "stuserlistmodel.h"
#include "stuserlistfetchmoreview.h"
#include "stuserlistitemview.h"
#include "stclipview.h"

STUserListView::STUserListView(STUserListModel *model, QGraphicsItem *parent) :
    STStaticListView(parent), m_model(model)
{
    setItemHeight(65);

    this->appendItem(); //fetch more

    connect(model, SIGNAL(someUsersLoaded()),
            this, SLOT(someUsersLoaded()));
    connect(model, SIGNAL(userIdsAppended(qint64)),
            this, SLOT(userIdsAppended(qint64)));
    connect(model, SIGNAL(userIdsPrepended(quint64)),
            this, SLOT(userIdsPrepended(quint64)));
    connect(model, SIGNAL(userIdRemoved(qint64,qint64,quint64)),
            this, SLOT(userIdRemoved(qint64,qint64,quint64)));
    connect(model, SIGNAL(userIdFetchDone()),
            this, SLOT(userIdFetchDone()));
    connect(model, SIGNAL(userIdFetchFailed()),
            this, SLOT(userIdFetchFailed()));
}

void STUserListView::itemLinkActivated(STObjectManager::EntityRange *er,
                                       STObjectManager::LinkActivationInfo i){
    emit linkActivated(er,i);
}

QGraphicsItem *STUserListView::createView(qint64 index, STClipView *parent){
    if(index>=m_model->rowsCount()){
        // fetch more view
        STUserListFetchMoreView *fmv=new STUserListFetchMoreView(QSize(itemPreferredWidth(), itemHeight()),
                                                                 parent);
        fmv->setFetching(m_model->isFetching());
        connect(fmv, SIGNAL(fetchMoreRequested()),
                this, SLOT(startUserIdFetch()));
        connect(this, SIGNAL(fetchingUpdated(bool)),
                fmv, SLOT(setFetching(bool)));
        return fmv;
    }else{
        STUserListItemView *item;
        item=new STUserListItemView(m_model->rowIdAtIndex(index),
                                    QSize(itemPreferredWidth(), itemHeight()),
                                    m_model, parent);
        connect(item, SIGNAL(linkActivated(STObjectManager::EntityRange*,
                                           STObjectManager::LinkActivationInfo)),
                this, SLOT(itemLinkActivated(STObjectManager::EntityRange*,
                                             STObjectManager::LinkActivationInfo)));
        return item;
    }
}

void STUserListView::someUsersLoaded(){

}

void STUserListView::userIdsAppended(qint64 count){
    if(count==m_model->rowsCount()){
        this->removeItem(0);
        this->appendItem(count+(m_model->hasMoreUserIds()?1:0));
    }else{
        this->appendItem(count+(m_model->hasMoreUserIds()?1:0)); // add items with fetch more
        this->removeItem(m_model->rowsCount()-count);// remove old fetch more
    }
    Q_ASSERT(itemCount()==m_model->rowsCount()+(m_model->hasMoreUserIds()?1:0));
}

void STUserListView::userIdsPrepended(quint64 count){
    this->prependItem(count);
}

void STUserListView::userIdRemoved(qint64 index, qint64, quint64){
    this->removeItem(index);
}

void STUserListView::userIdFetchDone(){
    emit fetchingUpdated(false);
}

void STUserListView::userIdFetchFailed(){
    emit fetchingUpdated(false);
}

void STUserListView::startUserIdFetch(){
    emit fetchingUpdated(true);
    m_model->fetchMore();
}

QVariant STUserListView::itemChange(GraphicsItemChange change, const QVariant &value){
    if(change==ItemVisibleHasChanged){
        if(isVisible()){
            if(m_model->rowsCount()==0)
                startUserIdFetch();
        }
    }
    return STStaticListView::itemChange(change, value);
}
