#include "staccountsview.h"
#include "staccountitemview.h"
#include "staccountsmanager.h"
#include "staccount.h"
#include <QTimer>
#include <QDebug>

STAccountsView::STAccountsView(QGraphicsItem *parent) :
    STDynamicListView(parent)
{
    setFlag(ItemClipsChildrenToShape, false);
    //this->scrollBarView()->setStyle(STScrollBarView::StellaBright); bright sidebar
    setMaxItemCount(100000); // shouldn't limit...

    /*
    QTimer *timer=new QTimer(this);
    timer->setSingleShot(true);
    connect(timer, SIGNAL(timeout()),
            this, SLOT(loadView()));
    timer->start(10);*/

    m_selectedAccountId=0;

    this->setShadowSize(7.f);
    this->setShadowOpacity(0.23f);


}

bool STAccountsView::validateStructure(){
    Q_ASSERT(m_rowIdForAccountId.count()==m_accountIdForRowId.count());

    QList<quint64> a, b;
    STAccountList lst=STAccountsManager::sharedManager()->accounts();
    Q_ASSERT(lst.count()==m_rowIdForAccountId.count());
    a.reserve(lst.count());
    b.reserve(lst.count());
    for(QMap<quint64, quint64>::iterator it=m_accountIdForRowId.begin();it!=m_accountIdForRowId.end();it++){
        a.append(it.value());
        Q_ASSERT(m_rowIdForAccountId[it.value()]==it.key());
    }
    for(int i=lst.count()-1;i>=0;i--)
        b.append(lst[i]->userId());

    for(int i=0;i<lst.count();i++){
        if(a[i]!=b[i]){
            // dump structure
            qDebug()<<"fatal: accounts list structure broken";
            qDebug()<<" list from Accounts Manager:";
            foreach(quint64 i, b){
                qDebug()<<i<<" - "<<STAccountsManager::sharedManager()->account(i)->userObject()->data["screen_name"].toString();
            }
            qDebug()<<"---";
            qDebug()<<" list from Accounts List View";
            foreach(quint64 i, a){
                qDebug()<<i<<" - "<<STAccountsManager::sharedManager()->account(i)->userObject()->data["screen_name"].toString();
            }

            return false;
        }
    }
    return true;
}

void STAccountsView::loadView(){
    qint64 rowId=0x4000000000000000ULL;

    // TODO: add from last item to first one, to ensure animation work properly
    STAccountList lst=STAccountsManager::sharedManager()->accounts();
    for(int i=lst.count()-1;i>=0;i--){
        STAccount *acc=lst[i];
        m_rowIdForAccountId[acc->userId()]=rowId;
        m_accountIdForRowId[rowId]=acc->userId();

        insertItem(rowId, itemSizeForAccount(acc).height());

        rowId++;
    }
    connect(STAccountsManager::sharedManager(),
            SIGNAL(accountWasAdded(quint64)),
            this, SLOT(accountWasAdded(quint64)));
    connect(STAccountsManager::sharedManager(),
            SIGNAL(accountBeingRemoved(quint64)),
            this, SLOT(accountBeingRemoved(quint64)));
    Q_ASSERT(validateStructure());
}

void STAccountsView::accountWasAdded(quint64 acId){

    quint64 rowId=minRowId()-1;
    Q_ASSERT(!m_accountIdForRowId.contains(rowId));
    Q_ASSERT(!m_rowIdForAccountId.contains(acId));
    STAccount *ac=STAccountsManager::sharedManager()->account(acId);
    Q_ASSERT(ac);
    m_rowIdForAccountId.insert(ac->userId(), rowId);
    m_accountIdForRowId.insert(rowId, ac->userId());
    insertItem(rowId, itemSizeForAccount(ac).height());

    Q_ASSERT(validateStructure());
}

void STAccountsView::accountBeingRemoved(quint64 acId){
    quint64 rowId=m_rowIdForAccountId[acId];
    Q_ASSERT(rowId);
    removeItem(rowId);
    m_rowIdForAccountId.remove(acId);
    m_accountIdForRowId.remove(rowId);

    Q_ASSERT(validateStructure());
}

void STAccountsView::accountScreenNameChanged(){

    STAccount *ac=dynamic_cast<STAccount *>(sender());
    Q_ASSERT(ac);

    if(!m_rowIdForAccountId.contains(ac->userId()))
        return;
    quint64 rowId=m_rowIdForAccountId[ac->userId()];
    Q_ASSERT(rowId);

    // reinsertion
    insertItem(rowId, itemSizeForAccount(ac).height());
}

quint64 STAccountsView::minRowId(){
    if(m_accountIdForRowId.isEmpty())return 0x4000000000000000ULL;
    return m_accountIdForRowId.begin().key();/*
    STAccountList lst=STAccountsManager::sharedManager()->accounts();
    if(lst.isEmpty())
        return 0x4000000000000000ULL;
    quint64 accId=lst.last()->userId();
    return m_rowIdForAccountId[accId];*/
}

void STAccountsView::relayoutScrollBar(){
    scrollBarView()->setPos((float)(size().width()-7)+.01f, -1);
    scrollBarView()->setSize(QSize(7, size().height()+2+32));

}

QSize STAccountsView::itemSizeForAccount(STAccount *acc){
    return STAccountItemView::estimateSize(acc->userObject(), itemPreferredWidth());
}

QGraphicsItem *STAccountsView::createView(quint64 id, double , STClipView *parent){
    quint64 acId;
    if(!m_accountIdForRowId.contains(id))return NULL;
    acId=m_accountIdForRowId[id];

    STAccount *account=STAccountsManager::sharedManager()->account(acId);
    STAccountItemView *vw=new STAccountItemView(account, itemSizeForAccount(account), parent);
    connect(vw, SIGNAL(triggered()),
            this, SLOT(accountButtonTriggered()));
    connect(this, SIGNAL(selectedAccountChanged(quint64)),
            vw, SLOT(accountSelected(quint64)));
    connect(vw, SIGNAL(dragInitated(quint64,QPointF)),
            this, SLOT(dragStarting(quint64,QPointF)));
    vw->accountSelected((m_selectedAccountId));
    return vw;
}

void STAccountsView::accountButtonTriggered(){
    STAccountItemView *button=dynamic_cast<STAccountItemView *>(sender());
    Q_ASSERT(button);

    quint64 acId=button->accountId();
    if(!m_rowIdForAccountId.contains(acId)){
        // already removed, but still remains in list because
        // it is fading away now.
        // just ignore this becaus such account no longer does exist
        return;
    }

    emit accountSelected(acId);

}

void STAccountsView::setSelectedAccount(quint64 ac){
    if(ac==m_selectedAccountId)return;
    m_selectedAccountId=ac;
    emit selectedAccountChanged(ac);
    if(!m_rowIdForAccountId.contains(ac)){
        return;
    }
    this->autoScrollToRow(m_rowIdForAccountId[ac]);
    Q_ASSERT(validateStructure());
}

void STAccountsView::dragStarting(quint64 acId, QPointF shift){
    this->initiateDragItem(m_rowIdForAccountId[acId], shift.y());
    Q_ASSERT(validateStructure());
}

static int indexForAccountId(quint64 id){
    STAccountList lst=STAccountsManager::sharedManager()->accounts();
    for(int i=0;i<lst.count();i++)
        if(lst[i]->userId()==id)
            return i;
    return -1;
}

void STAccountsView::itemMoved(quint64 oldId, quint64 newId){
    STAccountList lst=STAccountsManager::sharedManager()->accounts();


    // movement in account list (this must be done before reordering of data structure)
    int oldIndex=indexForAccountId(m_accountIdForRowId[oldId]);
    int newIndex=indexForAccountId(m_accountIdForRowId[newId]);
    Q_ASSERT(oldIndex!=-1);
    Q_ASSERT(newIndex!=-1);
    STAccountsManager::sharedManager()->reorder(oldIndex, newIndex);

    typedef QMap<quint64, quint64>::iterator Iter;

    // movement in data structure
    Iter oldIter=m_accountIdForRowId.find(oldId);
    Iter newIter=m_accountIdForRowId.find(newId);
    if(newId>oldId){
        for(Iter it=oldIter;it!=newIter;it++){
            m_rowIdForAccountId.remove(it.value());
        }
        m_rowIdForAccountId.remove(newIter.value());

        quint64 dragAccId=oldIter.value();
        for(Iter it=oldIter;it!=newIter;it++){
            it.value()=(it+1).value();
        }
        newIter.value()=dragAccId;

        for(Iter it=oldIter;it!=newIter;it++){
            m_rowIdForAccountId.insert(it.value(), it.key());
        }
        m_rowIdForAccountId.insert(newIter.value(), newIter.key());
    }else{
        for(Iter it=oldIter;it!=newIter;it--){
            m_rowIdForAccountId.remove(it.value());
        }
        m_rowIdForAccountId.remove(newIter.value());

        quint64 dragAccId=oldIter.value();
        for(Iter it=oldIter;it!=newIter;it--){
            it.value()=(it-1).value();
        }
        newIter.value()=dragAccId;

        for(Iter it=oldIter;it!=newIter;it--){
            m_rowIdForAccountId.insert(it.value(), it.key());
        }
        m_rowIdForAccountId.insert(newIter.value(), newIter.key());
    }


    Q_ASSERT(validateStructure());
}
