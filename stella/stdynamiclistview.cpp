#include "stdynamiclistview.h"
#include <QDateTime>
#include <QGraphicsPixmapItem>
#include "stfont.h"
#include <QLinearGradient>
#include <QPainter>
#include <QBrush>
#include <QDebug>
#include <QVarLengthArray>
#include <QGraphicsSceneWheelEvent>
#include <QPixmapCache>
#include <QGraphicsScene>
#include <QGraphicsView>

#ifndef HUGE
#define HUGE        1.e+100
#endif

STDynamicListView::STDynamicListView(QGraphicsItem *parent) :
    QGraphicsObject(parent), m_size(0,0)
{
    setFlag(ItemClipsChildrenToShape);
    setFlag(ItemHasNoContents);

    m_shadowView=new QGraphicsPixmapItem(this);
    m_dragTopShadowView=new QGraphicsPixmapItem(this);
    m_dragBottomShadowView=new QGraphicsPixmapItem(this);
    m_clipView=new STClipView(this);
    m_scrollBar=new STScrollBarView(this);

    m_scrollBar->setZValue(10.f);
    m_dragTopShadowView->setZValue(1.f);
    m_dragBottomShadowView->setZValue(1.f);
    m_clipView->setZValue(0.f);
    m_shadowView->setZValue(-0.5f);

    m_followMode=AlwaysFollow;

    m_shadowView->setOpacity(0.f);
    static QPixmapCache::Key shadowKey;
    QPixmap shadowPixmap;
    if(!QPixmapCache::find(shadowKey, &shadowPixmap)){
        shadowPixmap=QPixmap(1, 64);
        shadowPixmap.fill(QColor(0,0,0,0));
        {
            QPainter painter(&shadowPixmap);
            QLinearGradient grad(0,0,0,64);
            grad.setColorAt(0,QColor(0,0,0,255));
            grad.setColorAt(0.3f,QColor(0,0,0,128));
            grad.setColorAt(1, QColor(0,0,0,0));
            painter.fillRect(0,0,1,64,grad);
        }
        shadowKey=QPixmapCache::insert(shadowPixmap);
    }
    m_shadowView->setPixmap(shadowPixmap);
    m_dragBottomShadowView->setPixmap(shadowPixmap);
    m_dragTopShadowView->setPixmap(shadowPixmap);
    m_shadowSize=5.f;

    m_dragTopShadowView->setOpacity(0);
    m_dragBottomShadowView->setOpacity(0);
    m_dragging=NoDrag;

    m_ignoreValueChange=false;

    m_topPosAnimator.setPos(0.);
    m_bottomPosAnimator.setPos(0.);
    m_lastTopPos=0.;
    m_timer=0;
    m_maxItemCount=1024*16;
    m_pinned=true;

    m_autoScrolling=false;
    m_autoScroller.setPos(0.);

    connect(m_scrollBar, SIGNAL(valueChanged()),
            this, SLOT(scrollChanged()));
    connect(m_scrollBar, SIGNAL(scrolled()),
            this, SLOT(abortAutoScroll()));

    relayout();
}

void STDynamicListView::setShadowSize(float sz){
    m_shadowSize=sz;
    updateShadow();
}

void STDynamicListView::setShadowOpacity(float op){
    m_shadowView->setOpacity(op);
}

void STDynamicListView::updateShadow(){
    // scals shadow
    {
        QTransform trans;
        trans.scale((float)m_size.width(), m_shadowSize/64.f);
        m_shadowView->setTransform(trans);
    }
    {
        QTransform trans;
        trans.scale((float)m_size.width(), -m_shadowSize*.6f/64.f); // flip
        m_dragTopShadowView->setTransform(trans);
    }
    {
        QTransform trans;
        trans.scale((float)m_size.width(), m_shadowSize/64.f);
        m_dragBottomShadowView->setTransform(trans);
    }

}

void STDynamicListView::setSize(const QSize &sz){
    if(sz==m_size)
        return;
    prepareGeometryChange();
    m_size=sz;

    updateShadow();
    relayout();
}

quint64 STDynamicListView::currentTime(){
    return QDateTime::currentMSecsSinceEpoch();
}

double STDynamicListView::itemTopPos(){
    if(m_items.empty())
        return m_topPosAnimator.finalPos();
    else{
        const Item& it=m_items.front();
        if(m_dragging)
            return m_dragOldFinalPos;
        return it.posAnimator.finalPos();
    }
}

void STDynamicListView::relayoutScrollBar(){
    if(m_scrollBar->style()==STScrollBarView::StellaLight){
        m_scrollBar->setPos((float)(m_size.width()-15)+.01f, -1);
        m_scrollBar->setSize(QSize(16, m_size.height()+2));
    }else{
        m_scrollBar->setPos((float)(m_size.width()-10)-.2f, 1);
        m_scrollBar->setSize(QSize(9, m_size.height()-2));
    }
}

quint64 STDynamicListView::relayout(){
    quint64 curTime=currentTime();

    relayoutScrollBar();

    double listHeight=(double)m_size.height();

    double topOffset=m_topPosAnimator.currentPos(curTime)+m_scrollBar->value();


    // shadow view position calculation is done later

    // purge items if there are too many items
    {
        // fast
        int leftCount=qMax(0, m_itemsIndex.count()-m_maxItemCount);
        while(leftCount--){
            Q_ASSERT(!m_itemsIndex.isEmpty());
            QLinkedList<Item>::iterator it=m_itemsIndex.begin().value();
            Item& item=*it;
            Q_ASSERT(!item.beingRemoved);
            item.beingRemoved=true;
            item.wasPurged=true;

            if(item.id==m_dragId && m_dragging){
                m_dragging=NoDrag;
                ungrabMouse();
            }

            m_itemsIndex.erase(m_itemsIndex.begin());
        }
    }

    if(m_dragging==Dragging){
        // if dragging, move item to the proper pos
        QMap<quint64, QLinkedList<Item>::iterator>::iterator it=m_itemsIndex.find(m_dragId);
        Q_ASSERT(it!=m_itemsIndex.end());
        QLinkedList<Item>::iterator it2=it.value();
        Item& item=*it2;

        double newPos=m_draggedItemPos+topOffset+item.height*.5;
        double oldPos=itemTopPos(); // pos when it was not being dragged
        {
            QLinkedList<Item>::iterator it3=it2;
            while(it3!=m_items.begin()){
                it3--;
                Item& item2=*it3;
                if(!item2.beingRemoved){
                    oldPos=item2.posAnimator.finalPos();
                    oldPos+=item2.height;
                    break;
                }
            }
        }
        oldPos+=item.height*.5f;

        // moving up/down?
        if(newPos>oldPos){
            // down!
            QLinkedList<Item>::iterator it4=it2, it3=it4+1;
            while(it3!=m_items.end()){
                Item& item2=*it3;
                if(!item2.beingRemoved){
                    double pos=item2.posAnimator.finalPos()+item2.height*.5;
                    if(newPos<pos){
                        it3=it4;
                        break;
                    }
                    it4=it3;
                }
                it3++;
            }

            if(it3==m_items.end()){
                it3=it4;
            }
            if(it3!=it2){
                // moved!
                // delete index
                for(it4=it2;it4!=it3;it4++){
                    m_itemsIndex.remove(it4->id);
                }
                m_itemsIndex.remove(it3->id);

                // [2 4 6 8 9 12 15 17] E
                quint64 oldId=it2->id;
                quint64 newId=it3->id;
                for(it4=it3;it4!=it2;it4--){
                    it4->id=(it4-1)->id;
                }
                // [2 2 4 6 8 9 12 15]

                Item itm=*it2;
                m_itemsIndex.remove(it2->id);

                it4=it2+1;
                m_items.erase(it2);
                itm.id=newId;

                // 2 4 6 8 9 12 15]

                it3=m_items.insert(it3+1, itm);
                m_itemsIndex.insert(newId, it3);

                // 2 4 6 8 9 12 15] [17
                for(;it4!=it3;it4++){
                    m_itemsIndex.insert(it4->id, it4);
                }

                emit itemMoved(oldId, newId);

                m_dragId=newId;
            }

        }else{
            // up!
            QLinkedList<Item>::iterator it4=it2, it3=it4;
            while(it3!=m_items.begin()){
                it3--;
                Item& item2=*it3;
                if(!item2.beingRemoved){
                    double pos=item2.posAnimator.finalPos()+item2.height*.5;
                    if(newPos>pos){
                        it3=it4;
                        break;
                    }
                    it4=it3;
                }
            }

            if(it3!=it2){
                // moved!
                // delete index
                for(it4=it3;it4!=it2;it4++)
                    m_itemsIndex.remove(it4->id);
                m_itemsIndex.remove(it2->id);

                // [2 4 6 8 9 12 15 17]
                quint64 oldId=it2->id;
                quint64 newId=it3->id;
                for(it4=it3;it4!=it2;it4++){
                    it4->id=(it4+1)->id;
                }
                // [4 6 8 9 12 15 17 17]
                Item itm=*it2;
                it4=it2+1;
                m_items.erase(it2);
                itm.id=newId;

                // [4 6 8 9 12 15 17

                it3=m_items.insert(it3, itm);
                // 2] [4 6 8 9 12 15 17

                for(;it3!=it4;it3++)
                    m_itemsIndex.insert(it3->id, it3);

                emit itemMoved(oldId, newId);

                m_dragId=newId;

            }




        }

    }

    quint64 draggedId=(m_dragging!=NoDrag)?m_dragId:(quint64)-1; // make layout faster

    // layout items
    double pinShift=HUGE;
    {
        double yPos=itemTopPos();
        double removedYPos=yPos;

        for(QLinkedList<Item>::iterator it=m_items.begin();it!=m_items.end();it++){
            Item& item=*it;
            if(item.id==draggedId && m_dragging==Dragging){
                item.posAnimator.setPos(m_draggedItemPos+topOffset);
            }else{
                item.posAnimator.setPosAnimated(item.beingRemoved?removedYPos:yPos, 300, curTime);
            }

            double visiblePos=item.posAnimator.currentPos(curTime);
            double visualPos=visiblePos-topOffset;
            item.visiblePos=visiblePos;
            item.visualPos=visualPos;

            bool currentlyVisible=(visualPos+item.height>0.) && (visualPos<listHeight);
            if(pinShift==HUGE && currentlyVisible){
                pinShift=visiblePos-item.lastVisiblePos;
            }

            if(!item.beingRemoved){
                yPos+=item.height;
                removedYPos=yPos;
            }else{
                removedYPos+=item.height;
            }
        }

        m_bottomPosAnimator.setPosAnimated(yPos, 300, curTime);
    }

    double scrollMax=qMax(0.,m_bottomPosAnimator.currentPos(curTime)-
                          m_topPosAnimator.currentPos(curTime)-listHeight);
    double newValue=m_scrollBar->value();

    switch(m_followMode){
    case AlwaysFollow:
        m_pinned=true;
        break;
    case NeverFollow:
        m_pinned=false;
        break;
    case AutoFollow:
        m_pinned=(newValue<1.);
        if(m_pinned) newValue=0.;
        break;
    }

    if(!m_pinned && pinShift!=HUGE){
        newValue+=pinShift;
        newValue+=m_lastTopPos-m_topPosAnimator.currentPos(curTime);
    }
    if(m_pinned && m_autoScrolling){
        newValue=m_autoScroller.currentPos(curTime);
        if(!m_autoScroller.isAnimating(curTime)){
            m_autoScrolling=false;
        }
    }else{
        m_autoScrolling=false;
    }
    if(newValue>scrollMax){
        newValue=scrollMax;
    }
    if(newValue<0.){
        newValue=0.;
    }

    double deltaPos=newValue-m_scrollBar->value();
    for(QLinkedList<Item>::iterator it=m_items.begin();it!=m_items.end();it++){
        Item& item=*it;
        item.visualPos-=deltaPos;
    }

    m_lastTopPos=m_topPosAnimator.currentPos(curTime);

    double zIndex=0.;
    for(QLinkedList<Item>::iterator it=m_items.begin();it!=m_items.end();it++){
        Item& item=*it;

        //double visiblePos=item.visiblePos;
        double visualPos=item.visualPos;
        bool currentlyVisible=(visualPos+item.height>0.) && (visualPos<listHeight);
        if(draggedId==item.id)
            currentlyVisible=true;

        if(item.view){
            if(currentlyVisible){
                layoutView:
                if(curTime<item.appearTime+50 && false ){
                    item.view->setOpacity((float)(curTime-item.appearTime)/50.f);
                    startAnimation();
                }else{
                    item.view->setOpacity(1.f);
                }


                item.view->setPos(0.f, (float)(int)(visualPos+.5));
                if(draggedId==item.id){
                    // get outta clip view, so that not clipped and get above all other items
                    item.view->setParentItem(this);
                    item.view->setZValue(5.f); // between shadow and scrollbar
                }else{
                    item.view->setZValue(zIndex);
                    item.view->setParentItem(m_clipView);
                }
                zIndex+=1.;
            }else{
                delete item.view;
                item.view=NULL;
            }
        }else{
            if(currentlyVisible){
                item.view=createView(item.id, item.height, m_clipView);
                if(item.view){
                    item.appearTime=curTime-100; // disabling appear animation...
                    goto layoutView;
                }
            }
        }

        if(item.posAnimator.isAnimating(curTime)){
            startAnimation();
        }


        item.lastVisiblePos=item.visiblePos;
    }



    QVarLengthArray<QLinkedList<Item>::iterator, 256> removedItems;
    if(!m_items.empty()){
        QLinkedList<Item>::iterator it=m_items.end();
        double lastYPos=m_bottomPosAnimator.currentPos(curTime);
        do{
            it--;

            Item& item=*it;
            double visiblePos=item.posAnimator.currentPos(curTime);
            if(item.beingRemoved){
                if(visiblePos>=lastYPos){
                    removedItems.append(it);
                }
            }else{
                lastYPos=visiblePos;
            }
        }while(it!=m_items.begin());

        for(int j=0;j<removedItems.count();j++){
            QLinkedList<Item>::iterator i=removedItems[j];
            if(i->wasPurged){
                quint64 id=i->id;
                double h=i->height;
                itemBeingPurged(id, h);
                if(i->view){
                    delete i->view;
                }
                m_items.erase(i);
                itemWasPurged(id, h);
            }else{
                if(i->view){
                    delete i->view;
                }
                m_items.erase(i);

            }

        }
    }

    double clipHeight=m_bottomPosAnimator.currentPos(curTime)-topOffset;


    // bottom item or the second lowest item can be higher than clipHeight.
    {
        int count=2;
        double maxHeight=0.f;
        QLinkedList<Item>::iterator it=m_items.end();
        while(it!=m_items.begin() && count){
            it--;
            Item& item=*it;
            double y=item.visualPos+item.height;
            if(y>maxHeight)maxHeight=y;
            count--;
        }
        if(maxHeight<clipHeight || m_dragging!=NoDrag)
            clipHeight=maxHeight;
    }

    if(clipHeight<=0.){
        m_clipView->setVisible(false);
    }else if(clipHeight<listHeight){
        m_clipView->setRect(QRectF(0.,0.,m_size.width(),clipHeight));
        m_clipView->setFlag(ItemClipsChildrenToShape,true);
        m_clipView->setVisible(true);
    }else{
        m_clipView->setFlag(ItemClipsChildrenToShape,false);
        m_clipView->setVisible(true);
    }

    if(clipHeight<listHeight){
        m_shadowView->setVisible(true);
        m_shadowView->setPos(0,  clipHeight);
    }else{
        m_shadowView->setVisible(false);
    }

    // update dragging shadow.
    if(m_dragging!=NoDrag){

        QMap<quint64, QLinkedList<Item>::iterator>::iterator it=m_itemsIndex.find(m_dragId);
        Q_ASSERT(it!=m_itemsIndex.end());
        Item& item=*(it.value());
        m_dragTopShadowView->setPos(0,item.visualPos);
        m_dragBottomShadowView->setPos(0,item.visualPos+item.height);

        float op=(float)m_dragEndAnimator.currentPos(curTime);
        if(m_dragging==Dragging)op=1.f;
        m_dragTopShadowView->setOpacity(op*m_shadowView->opacity()*.6f);
        m_dragBottomShadowView->setOpacity(op*m_shadowView->opacity());
        if(fabs(item.visualPos+item.height-clipHeight)<.8)
            m_dragBottomShadowView->setOpacity(0.f);
        if(op<.01f){
            m_dragging=NoDrag;
        }
    }else{
        m_dragTopShadowView->setOpacity(0);
        m_dragBottomShadowView->setOpacity(0);
    }

    m_ignoreValueChange=true; // note: blockSignal is the better way
    m_scrollBar->setRange(0., scrollMax, newValue);
    m_ignoreValueChange=false;
    m_scrollBar->setLargeIncrement(listHeight);

    return curTime;
}

void STDynamicListView::paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *){

}

QRectF STDynamicListView::boundingRect() const{
    return QRectF(0,0,m_size.width(), m_size.height());
}

void STDynamicListView::timerEvent(QTimerEvent *){
    quint64 tm=relayout();
    if(!isAnimating(tm)){
        killTimer(m_timer);
        m_timer=0;
    }
}

void STDynamicListView::startAnimation(){
    if(m_timer)
        return;/*
    if(!isAnimating())
        return;*/
    m_timer=startTimer(15);
    //qDebug()<<m_timer;
}

bool STDynamicListView::isAnimating(quint64 tm){
    quint64 curTime=currentTime();
    if(tm)curTime=tm;
    if(m_topPosAnimator.isAnimating(curTime)){
       // qDebug()<<"m_topPosAnimator"<<rand();
        return true;
    }
    if(m_bottomPosAnimator.isAnimating(curTime)){
       // qDebug()<<"m_bottomPosAnimator"<<rand();
        return true;
    }
    if(m_dragging==DragDone && m_dragEndAnimator.isAnimating(curTime)){
        return true;
    }
    if(m_autoScroller.isAnimating(curTime) && m_autoScrolling){
       // qDebug()<<"m_bottomPosAnimator"<<rand();
        return true;
    }
    foreach(const Item& item, m_items){
        if(item.posAnimator.isAnimating(curTime))
            return true;
        if(curTime<item.appearTime+50)
            return true;
    }


    return false;
}

STDynamicListView::Item *STDynamicListView::itemWithId(quint64 id){
    for(QLinkedList<Item>::iterator it=m_items.begin();it!=m_items.end();it++){
        Item& item=*it;
        if(item.id==id)return &item;
    }
    return NULL;
}

quint64 STDynamicListView::idForView(QGraphicsItem *view) const{
    for(QLinkedList<Item>::const_iterator it=m_items.begin();it!=m_items.end();it++){
        const Item& item=*it;
        if(item.view==view)return item.id;
    }
    return 0;
}

void STDynamicListView::initateBulkLoad(){


    QList<QPair<quint64, double> > items;
    setItems(items);

    m_topPosAnimator.setPosAnimated(m_topPosAnimator.finalPos()+(double)m_size.height(),
                                    300, currentTime());
    m_bottomPosAnimator.setPosAnimated(m_bottomPosAnimator.finalPos()+(double)m_size.height(),
                                    300, currentTime());


}

static bool setItemItemsSorter(const QPair<quint64, double>& a,
                               const QPair<quint64, double>& b){
    return a.first>b.first;
}

void STDynamicListView::setItems(QList<QPair<quint64, double> > items){
    // remove all items
    for(QLinkedList<Item>::iterator it=m_items.begin();it!=m_items.end();it++){
        Item& item=*it;
        if(item.view)
            delete item.view;
    }
    m_items.clear();;
    m_itemsIndex.clear();;

    double h=0.;

    QPair<quint64, double> item;
    qSort(items.begin(),items.end(),setItemItemsSorter); // desc
    foreach(item, items){
        Item newItem;
        newItem.id=item.first;
        newItem.view=NULL;
        newItem.beingRemoved=false;
        newItem.wasPurged=false;
        newItem.height=item.second;
        newItem.appearTime=0;
        newItem.posAnimator.setPos(h);

        m_items.append(newItem);
        m_itemsIndex.insert(newItem.id ,m_items.end()-1);

        h+=item.second;
    }
    m_topPosAnimator.setPos(0);
    m_bottomPosAnimator.setPos(h);
    abortAutoScroll();

    relayout();
}

void STDynamicListView::insertItem(quint64 id, double height){
    removeItem(id); // duplicate item is not allowed currently
    Item item;
    item.id=id;
    item.view=NULL;
    item.beingRemoved=false;
    item.wasPurged=false;
    item.height=height;
    item.appearTime=0;

    QLinkedList<Item>::iterator it=posToInsertItem(id);
    quint64 curTime=1;
    if(it==m_items.begin()){
        double curTopPos=itemTopPos();
        double pos=curTopPos-height;
        item.posAnimator.setPos(pos);
        m_topPosAnimator.setPosAnimated(pos, 300,curTime);
    }else if(it==m_items.end()){
        item.posAnimator.setPos(m_bottomPosAnimator.finalPos());
        m_bottomPosAnimator.setPosAnimated(m_bottomPosAnimator.finalPos()+height, 300,
                                           curTime);
    }else{
        Item& i1=*(it-1);
        Item& i2=*(it);
        item.posAnimator.setPos(qMax(i1.posAnimator.finalPos()+i1.height,
                                     i2.posAnimator.finalPos()));//(i.posAnimator.currentPos(curTime));
        m_bottomPosAnimator.setPosAnimated(m_bottomPosAnimator.finalPos()+height, 300,
                                           curTime);
    }
    item.lastVisiblePos=item.posAnimator.finalPos();
    QLinkedList<Item>::iterator outIt=m_items.insert(it, item);
    m_itemsIndex.insert(id ,outIt);
    //relayout();
    startAnimation();
}

QLinkedList<STDynamicListView::Item>::iterator STDynamicListView::posToInsertItem(quint64 id){
    if(m_itemsIndex.empty())return m_items.begin();

    QMap<quint64, QLinkedList<Item>::iterator>::iterator it;
    it=m_itemsIndex.lowerBound(id);
    if(it==m_itemsIndex.end()){
        return m_items.begin();
    }

    QLinkedList<Item>::iterator it2=it.value();
    it2++;
    return it2;

    /*
    if(id>m_items.front().id)
        return m_items.begin();
    if(id<m_items.back().id)
        return m_items.end();
    for(QLinkedList<Item>::iterator it=m_items.begin();it!=m_items.end();it++){
        Item& item=*it;
        if(id>item.id)return it;
    }

    Q_ASSERT(false);*/
}

void STDynamicListView::removeItem(quint64 id){
    QMap<quint64, QLinkedList<Item>::iterator>::iterator indIt=m_itemsIndex.find(id);
    if(indIt!=m_itemsIndex.end()){
        QLinkedList<Item>::iterator it=indIt.value();
        Item& item=*it;
        Q_ASSERT(!item.beingRemoved);
        if(item.id==id){
            item.beingRemoved=true;
            m_itemsIndex.erase(indIt);
            if(item.id==m_dragId && m_dragging){
                m_dragging=NoDrag;
                ungrabMouse();
            }
            startAnimation();
            return;
        }
    }

}

int STDynamicListView::itemPreferredWidth() const{
    return (int)m_scrollBar->pos().x();
}

QGraphicsItem *STDynamicListView::createView(quint64 id, double height, STClipView *parent){
    QSize size(itemPreferredWidth(), (int)height);
    QImage img(size, QImage::Format_RGB32);
    img.fill(0);

    QPainter painter(&img);
    QLinearGradient grad(0.f, 0.f, 0.f, (float)height);
    grad.setColorAt(0.f, QColor(75,75,75));
    grad.setColorAt(1.f, QColor(65,65,65));
    painter.fillRect(QRect(0, 0, m_size.width(), m_size.height()), QBrush(grad));

    QString text=QString("ID = %1").arg(id);
    STFont::defaultFont()->drawString(img, QPointF(4, (height-12.f)/2.f),
                                      QColor(240,240,240), text);


    QGraphicsPixmapItem *item=new QGraphicsPixmapItem(QPixmap::fromImage(img), parent);

    return item;
}

void STDynamicListView::setMaxItemCount(int i){
    m_maxItemCount=i;
}

void STDynamicListView::scrollChanged(){
    if(m_ignoreValueChange)
        return;
    relayout();
}

void STDynamicListView::wheelEvent(QGraphicsSceneWheelEvent *event){
    m_scrollBar->setValue(m_scrollBar->value()-event->delta());
}

void STDynamicListView::autoScrollTo(double pos){
    if(!m_autoScrolling)
        m_autoScroller.setPos(m_scrollBar->value());
    m_pinned=true;
    m_autoScroller.setPosAnimated(pos, 300, currentTime());
    m_autoScrolling=true;
    startAnimation();
}

void STDynamicListView::autoScrollToRow(quint64 row){
    QLinkedList<Item>::iterator it;
    if(!m_itemsIndex.contains(row))return; // row not found
    it=m_itemsIndex[row];
    Item& item=*it;
    quint64 curTime=currentTime();

    double topOffset=m_topPosAnimator.currentPos(curTime);
    double curScroll=m_scrollBar->value();
    if(m_autoScrolling)
        curScroll=m_autoScroller.finalPos();
    double newScroll=curScroll;

    double itemPos=item.posAnimator.currentPos(curTime)-topOffset;
    double listHeight=(double)m_size.height();

    if(itemPos+item.height>newScroll+listHeight)
        newScroll=itemPos+item.height-listHeight;

    if(itemPos<newScroll)
        newScroll=itemPos;


    if(newScroll!=curScroll){
        autoScrollTo(newScroll);
    }
}

void STDynamicListView::abortAutoScroll(){
    m_autoScroller.setPos(m_scrollBar->value());
    m_autoScrolling=false;
}

void STDynamicListView::setFollowMode(FollowMode md){
    if(md==m_followMode)
        return;
    abortAutoScroll();;
    m_followMode=md;
}

int STDynamicListView::itemCount() const{
    return m_itemsIndex.count();
}

void STDynamicListView::itemWasPurged(quint64 , double ){

}

void STDynamicListView::itemBeingPurged(quint64 , double ){

}

void STDynamicListView::itemMoved(quint64 oldId, quint64 newId){

}

void STDynamicListView::initiateDragItem(quint64 rowId, double initialShift){
    if(m_dragging==Dragging){
        abortDrag();
    }

    QMap<quint64, QLinkedList<Item>::iterator>::iterator indIt=m_itemsIndex.find(rowId);
    if(indIt!=m_itemsIndex.end()){
        QLinkedList<Item>::iterator it=indIt.value();
        Item& item=*it;
        Q_ASSERT(!item.beingRemoved);
        Q_ASSERT(item.id==rowId);

        QPointF itemPos(0., item.visualPos);
        itemPos=this->mapToScene(itemPos);
        QPoint scrPos=this->scene()->views()[0]->mapToGlobal(itemPos.toPoint());
        QPoint cur=QCursor::pos();
        m_dragPoint=(double)(cur.y()-scrPos.y())-initialShift;
        m_draggedItemPos=item.visualPos+initialShift;
        m_dragId=rowId;
        m_dragOldFinalPos=itemTopPos();
        m_dragging=Dragging;
        grabMouse();
        startAnimation();
    }

}

void STDynamicListView::mouseMoveEvent(QGraphicsSceneMouseEvent *event){
    if(m_dragging){
        m_draggedItemPos=(double)event->pos().y();
        m_draggedItemPos-=m_dragPoint;

        relayout();
    }
}

void STDynamicListView::mouseReleaseEvent(QGraphicsSceneMouseEvent *event){
    if(m_dragging){
        abortDrag();
    }
}

void STDynamicListView::abortDrag(){
    if(m_dragging){

        QMap<quint64, QLinkedList<Item>::iterator>::iterator indIt=m_itemsIndex.find(m_dragId);
        if(indIt!=m_itemsIndex.end()){
            QLinkedList<Item>::iterator it=indIt.value();
            if(it==m_items.begin()){
                // work-around for bug that after releasing the highest item,
                // topItemPos becomes the released pos
                Item& item=*it;
                item.posAnimator.setPosAnimated(m_dragOldFinalPos, 300, currentTime());
            }
        }

        m_dragging=DragDone;
        m_dragEndAnimator.setPos(1.);
        m_dragEndAnimator.setPosAnimated(0., 300, currentTime());
        ungrabMouse();;
        startAnimation();
    }
}
