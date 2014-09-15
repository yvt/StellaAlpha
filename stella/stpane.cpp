#include "stpane.h"
#include <QPainter>
#include "stheaderview.h"
#include "stbasetabpageview.h"

STPane::STPane(QGraphicsItem *parent) :
    QGraphicsObject(parent),m_size(0,0)
{
    setCacheMode(DeviceCoordinateCache);
    setFlag(ItemHasNoContents);
}

void STPane::setSize(const QSize &size){
    if(size==m_size)return;

    prepareGeometryChange();
    m_size=size;
    relayout();;
}

void STPane::relayout(){
    relayoutContents();
}

void STPane::relayoutContents(){

}

void STPane::setContentsHeight(float){}

int STPane::contentsWidth(){
    return m_size.width();
}


void STPane::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    //painter->drawEllipse(boundingRect());
}

QRectF STPane::boundingRect() const{
    return QRectF(0,0,m_size.width(),m_size.height());
}

STHeaderView::Contents STPane::headerViewContents(){
    STHeaderView::Contents cont;
    cont.title="No Title";
    cont.hasBackButton=canPopPane();
    if(cont.hasBackButton){
        STPane *prev=previousPane();
        if(prev){
            cont.backButtonText=prev->headerViewContents().title;
        }
    }
    return cont;
}

STBaseTabPageView *STPane::controller(){
    QGraphicsItem *item=parentItem();
    STBaseTabPageView *controller=NULL;
    while(item){
        controller=dynamic_cast<STBaseTabPageView *>(item);
        if(controller)
            break;
        item=item->parentItem();
    }
    Q_ASSERT(controller);
    return controller;
}

bool STPane::canPopPane(){
    return controller()->canPopPane();
}

void STPane::popPane(){
    controller()->popPane();
}

void STPane::popPaneAnimated(){
    controller()->popPaneAnimated();
}

void STPane::pushPane(STPane *p){
    controller()->pushPane(p);
}

void STPane::pushPaneAnimated(STPane *p){
    controller()->pushPaneAnimated(p);
}

bool STPane::isPaneActive(){
    QGraphicsItem *item=this;
    STBaseTabPageView *ctrl=controller();
    STPane *curPane=ctrl->currentPane();
    while(item!=ctrl && item){
        if(item==curPane)
            return true;
        item=item->parentItem();
    }
    return false;
}

STPane *STPane::previousPane(){
    return controller()->previousPaneFor(this);
}

void STPane::linkActivate(STObjectManager::EntityRange *er,
                          STObjectManager::LinkActivationInfo i){
    if(!isPaneActive())
        return;
    controller()->linkActivate(er,i);
}


void STPane::swapPane(STPane *p){
    if(!isPaneActive())
        return;
    controller()->swapPane(p);
}

void STPane::backButtonActivated(){
    popPaneAnimated();
}

void STPane::mousePressEvent(QGraphicsSceneMouseEvent *event){
    //pushPaneAnimated(new STPane(parentItem()));
}

