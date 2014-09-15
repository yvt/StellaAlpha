#include "stbasetabpageview.h"
#include "stpane.h"
#include "sttimelinepane.h"
#include "sttimeline.h"
#include <QDebug>
#include "stuserpane.h"
#include "staccount.h"
#include "staccountsmanager.h"
#include "ststatuspane.h"
#include <QDesktopServices>
#include "stzoomedimagemanager.h"

STBaseTabPageView::STBaseTabPageView(quint64 account, QGraphicsItem *parent) :
    QGraphicsObject(parent), m_size(0,0), m_accountId(account)
{
    setFlag(ItemHasNoContents);

    m_animator.setVisibleOnlyWhenParentIsVisible(true);
}

void STBaseTabPageView::setSize(const QSize &size){
    if(size==m_size)return;
    prepareGeometryChange();
    m_size=size;
    relayout();;
}
void STBaseTabPageView::relayout(){
    m_animator.setWidth(m_size.width());
    m_animator.relayout();
    for(int i=0;i<m_paneStack.size();i++){
        STPane *pane=m_paneStack[i];
        pane->setSize(m_size);
        if(!isVisible()){
            pane->setVisible(false);
            return;
        }
        pane->setVisible(m_animator.isViewVisible(pane));
    }
}

void STBaseTabPageView::paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *){

}

QRectF STBaseTabPageView::boundingRect() const{
    return QRectF(0.f, 0.f, m_size.width(), m_size.height());
}

void STBaseTabPageView::pushPane(STPane *pn){
    pn->setParentItem(this);
    connect(pn, SIGNAL(headerViewContentsChanged(int)),
            this, SLOT(paneHeaderViewContentsChanged(int)));
    m_paneStack.push_back(pn);
    m_animator.activateView(pn);
    relayout();
    emit headerViewContentsChanged(0);
}

void STBaseTabPageView::pushPaneAnimated(STPane *pn){
    pn->setParentItem(this);
    connect(pn, SIGNAL(headerViewContentsChanged(int)),
            this, SLOT(paneHeaderViewContentsChanged(int)));
    m_paneStack.push_back(pn);
    m_animator.activateViewAnimated(pn, 1, 200);
    relayout();
    emit headerViewContentsChanged(1);
}

void STBaseTabPageView::popPaneAnimated(){
    Q_ASSERT(!m_paneStack.isEmpty());
    //STPane *pane=m_paneStack.last();
    m_paneStack.pop_back();
    m_animator.activateViewAnimated(m_paneStack.last(),-1,200,true); // old pane is deleted later
    relayout();
    emit headerViewContentsChanged(-1);
}

void STBaseTabPageView::popPane(){
    Q_ASSERT(!m_paneStack.isEmpty());
    STPane *pane=m_paneStack.last();
    m_paneStack.pop_back();
    m_animator.activateView(m_paneStack.last());
    pane->deleteLater();
    relayout();
    emit headerViewContentsChanged(0);
}

bool STBaseTabPageView::canPopPane(){
    return m_paneStack.size()>1;
}

void STBaseTabPageView::swapPane(STPane *pane){
    if(currentPane()==pane)
        return;

    m_animator.abortAnimation();

    STPane *oldPane=currentPane();
    m_paneStack.pop_back();
    m_paneStack.push_back(pane);
    m_animator.activateView(pane);
    relayout();

    oldPane->deleteLater();

    emit headerViewContentsChanged(0);
}

STPane *STBaseTabPageView::currentPane(){
    return m_paneStack.last();
}

STPane *STBaseTabPageView::previousPaneFor(STPane *p){
    QList<STPane *>::const_iterator it=qFind(m_paneStack, p);
    if(it==m_paneStack.end())return NULL;
    if(it==m_paneStack.begin())return NULL;
    it--;
    return *it;
}

void STBaseTabPageView::paneHeaderViewContentsChanged(int opening){
    if(sender()!=currentPane())
        return;
    emit headerViewContentsChanged(opening);
}
QVariant STBaseTabPageView::itemChange(GraphicsItemChange change, const QVariant &value){
    if(change==ItemVisibleHasChanged){
        relayout();
    }
    return value;
}

STAccount *STBaseTabPageView::account(){
    return STAccountsManager::sharedManager()->account(m_accountId);
}

void STBaseTabPageView::linkActivate(STObjectManager::EntityRange *er,
                                     STObjectManager::LinkActivationInfo info){
    if(m_animator.isAnimating())
        return;
    if(er->entityType=="hashtags"){
        QString tlName="%1.search.#%2";
        tlName=tlName.arg(QString::number(m_accountId), er->entity.toMap()["text"].toString());

        STTimelinePane *pane=new STTimelinePane(this);
        pane->setTimelineIdentifier(tlName);
        pushPaneAnimated(pane);
    }else if(er->entityType=="user_mentions"){
        quint64 id=er->entity.toMap()["id"].toULongLong();

        STUserPane *pane=new STUserPane(id,account(), this);
        pushPaneAnimated(pane);
    }else if(er->entityType=="user_object"){
        quint64 id=er->entity.toMap()["id"].toULongLong();

        STUserPane *pane=new STUserPane(id,account(), this);
        pushPaneAnimated(pane);
    }else if(er->entityType=="user_id"){
        quint64 id=er->entity.toULongLong();

        STUserPane *pane=new STUserPane(id,account(), this);
        pushPaneAnimated(pane);
    }else if(er->entityType=="user_stobject"){
        STObjectManager::UserRef user=er->stObject;
        if(!user){
            qWarning()<<"STBaseTabPageView::linkActivate: entityType=user_stobject, but no user given.";
            return;
        }
        quint64 id=user->id;

        STUserPane *pane=new STUserPane(id,account(), this);
        pane->setUser(user);
        pushPaneAnimated(pane);
    }else if(er->entityType=="status_stobject"){
        STObjectManager::StatusRef status=er->stObject;
        if(!status){
            qWarning()<<"STBaseTabPageView::linkActivate: entityType=status_stobject, but no status given.";
            return;
        }
        quint64 id=status->id;

        STStatusPane *pane=new STStatusPane(id,account(), this);
        pane->setStatus(status);
        pushPaneAnimated(pane);
    }else if(er->entityType=="urls"){
        QString urlStr=er->entity.toMap()["url"].toString();
        QDesktopServices::openUrl(QUrl(urlStr));
    }else if(er->entityType=="st_url"){
        QString urlStr=er->entity.toString();
        QDesktopServices::openUrl(QUrl(urlStr));
    }else if(er->entityType=="st_video"){
        QString urlStr=er->entity.toMap()["url"].toString();
        QDesktopServices::openUrl(QUrl(urlStr));
    }else if(er->entityType=="st_image"){
        QString urlStr=er->entity.toMap()["media_url"].toString();
        STZoomedImageManager::sharedManager()->zoomImageWithUrl(QUrl(urlStr), true);
    }else{
        qDebug()<<"unknown entity type: "<<er->entityType;
    }
}


