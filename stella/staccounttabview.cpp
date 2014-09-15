#include "staccounttabview.h"
#include "stbasetabpageview.h"
#include "stpane.h"
#include "sttestpane.h"
#include "sttimelinepane.h"
#include "stuserpane.h"
#include "staccount.h"
#include "staccountsmanager.h"
#include "stuserlistpane.h"
#include "stuserlistmodel.h"

STAccountTabView::STAccountTabView(QGraphicsItem *parent,quint64 accountId) :
    QGraphicsObject(parent), m_accountId(accountId)
{
    setFlag(ItemHasNoContents);
    setFlag(ItemClipsChildrenToShape);

    STBaseTabPageView *tabPage;
    STTimelinePane *p;

    m_account=STAccountsManager::sharedManager()->account(accountId);

    tabPage=new STBaseTabPageView(accountId, this);
    p=new STTimelinePane(tabPage);
    p->setTimelineIdentifier(QString("%1.home").arg(accountId));
    tabPage->pushPane(p);
    m_tabPages.push_back(tabPage);

    tabPage=new STBaseTabPageView(accountId, this);
    p=new STTimelinePane(tabPage);
    p->setTimelineIdentifier(QString("%1.mentions").arg(accountId));
    tabPage->pushPane(p);
    m_tabPages.push_back(tabPage);

/*
    tabPage=new STBaseTabPageView(accountId, this);
    tabPage->pushPane(new STPane(tabPage));
    m_tabPages.push_back(tabPage);

    tabPage=new STBaseTabPageView(accountId, this);
    tabPage->pushPane(new STTestPane(tabPage));
    m_tabPages.push_back(tabPage);
*/
    tabPage=new STBaseTabPageView(accountId, this);
    tabPage->pushPane(new STUserPane(accountId, m_account, tabPage));
    m_tabPages.push_back(tabPage);

    foreach(STBaseTabPageView *tab, m_tabPages){
        tab->setVisible(false);
        connect(tab, SIGNAL(headerViewContentsChanged(int)),
                this, SLOT(paneHeaderViewContentsChanged(int)));
    }

    m_animator.setVisibleOnlyWhenParentIsVisible(true);

    setCurrentTab(0);
    m_size=QSize(0,0);
    relayout();
}

void STAccountTabView::relayout(){
    int i=0;
    m_animator.setWidth(m_size.width());
    m_animator.relayout();
    foreach(STBaseTabPageView *tab, m_tabPages){
        tab->setSize(m_size);
        if(isVisible()){
            tab->setVisible(m_animator.isViewVisible(tab));
        }else{
            tab->setVisible(false);
        }
        i++;
    }
}

QVariant STAccountTabView::itemChange(GraphicsItemChange change, const QVariant &value){
    if(change==ItemVisibleHasChanged){
        relayout();
    }
    return value;
}

void STAccountTabView::setSize(const QSize &size){
    if(size==m_size)return;
    prepareGeometryChange();

    m_size=size;
    relayout();
}

void STAccountTabView::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *){

}

QRectF STAccountTabView::boundingRect() const{
    return QRectF(0,0,m_size.width(),m_size.height());
}

STPane *STAccountTabView::currentPane(){
    return m_tabPages[m_currentTab]->currentPane();
}

void STAccountTabView::paneHeaderViewContentsChanged(int opening){
    if(sender()!=m_tabPages[m_currentTab])
        return;
    emit headerViewContentsChanged(opening);
}

void STAccountTabView::setCurrentTab(int tab){
    m_currentTab=tab;
    m_animator.activateView(m_tabPages[tab]);
    emit headerViewContentsChanged(0);
}

STBaseTabPageView *STAccountTabView::currentTabView(){
    return m_tabPages[m_currentTab];
}

void STAccountTabView::setCurrentTabAnimated(int tab){
    if(tab==m_currentTab)
        return;

    int dir=(tab>m_currentTab)?1:-1;
    m_currentTab=tab;

    m_animator.activateViewAnimated(m_tabPages[m_currentTab], dir, 200);
    emit headerViewContentsChanged(dir);
}
