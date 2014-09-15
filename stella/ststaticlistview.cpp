#include "ststdafx.h"
#include "ststaticlistview.h"
#include "stscrollbarview.h"
#include "stclipview.h"
#include <QVarLengthArray>
#include <QLinkedList>
#include "stfont.h"
#include <QDebug>
#include <QGraphicsSceneWheelEvent>

STStaticListView::STStaticListView(QGraphicsItem *parent) :
    QGraphicsObject(parent), m_size(0,0)
{
    m_clipView=new STClipView(this);
    m_scrollBar=new STScrollBarView(this);
    m_itemHeight=20;
    m_timerId=0;
    m_animatedFirstIndex=-1;
    m_animatedItemsPos.setPos(0.);
    m_itemsCount=0;
    m_topClipPos.setPos(0);
    m_bottomClipPos.setPos(0);
    m_topItemPos=0;

    setFlag(ItemHasNoContents);

    connect(m_scrollBar, SIGNAL(valueChanged()),
            this, SLOT(relayout()));
}

void STStaticListView::setSize(const QSize &sz){
    if(sz==m_size)
        return;
    prepareGeometryChange();
    m_size=sz;
    relayout();
}

void STStaticListView::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){

}

QRectF STStaticListView::boundingRect() const{
    return QRectF(0,0,m_size.width(),m_size.height());
}

static quint64 currentTime(){
    return QDateTime::currentMSecsSinceEpoch();
}

void STStaticListView::relayout(){
    quint64 curTime=currentTime();

    // layout views.
    if(m_scrollBar->style()==STScrollBarView::StellaLight){
        m_scrollBar->setPos((float)(m_size.width()-15)+.01f, -1);
        m_scrollBar->setSize(QSize(16, m_size.height()+2));
    }else{
        m_scrollBar->setPos((float)(m_size.width()-10)-.2f, 1);
        m_scrollBar->setSize(QSize(9, m_size.height()-2));
    }

    // calculate new scroll bar range.
    qint64 listHeight=m_size.height();
    qint64 topClipPos=(qint64)m_topClipPos.currentPos(curTime);
    qint64 bottomClipPos=(qint64)m_bottomClipPos.currentPos(curTime);
    qint64 scrollMax=qMax(bottomClipPos-topClipPos-listHeight, (qint64)0);
    m_scrollBar->blockSignals(true);
    m_scrollBar->setRange(0, (double)scrollMax);
    m_scrollBar->setLargeIncrement(listHeight);
    m_scrollBar->blockSignals(false);

    qint64 scrollPos=(qint64)m_scrollBar->value();
    scrollPos+=topClipPos;
    scrollPos-=m_topItemPos;

    qint64 bottomPos=scrollPos+listHeight;

    // create/destroy items properly.
    qint64 animatedItemsShift=(qint64)m_animatedItemsPos.currentPos(curTime);

    qint64 oldVisibleStartIndex=m_loadedFirstIndex;
    qint64 oldVisibleEndIndex=m_loadedFirstIndex+(qint64)m_loadedItems.count();
    qint64 newVisibleStartIndex;
    qint64 newVisibleEndIndex;
    if(m_animatedFirstIndex!=-1 && animatedItemsShift!=0){
        qint64 animatedItemPos=m_animatedFirstIndex*(qint64)m_itemHeight;
        if(scrollPos<=0){
            newVisibleStartIndex=0;
        }else if(scrollPos<=animatedItemPos){
            newVisibleStartIndex=scrollPos/(qint64)m_itemHeight;
        }else{
            if(animatedItemsShift<0){
                newVisibleStartIndex=(scrollPos-animatedItemPos-animatedItemsShift)/(qint64)m_itemHeight;
                newVisibleStartIndex+=m_animatedFirstIndex;
            }else{
                newVisibleStartIndex=m_animatedFirstIndex;
            }
        }

        if(bottomPos<=0){
            newVisibleEndIndex=0;
        }else{
            qint64 animatedItemShiftedPos=animatedItemPos+animatedItemsShift;
            if(bottomPos<=animatedItemShiftedPos){
                newVisibleEndIndex=(bottomPos+m_itemHeight-1)/(qint64)m_itemHeight;
            }else{
                newVisibleEndIndex=(bottomPos-animatedItemShiftedPos+m_itemHeight-1)/(qint64)m_itemHeight;
                newVisibleEndIndex+=m_animatedFirstIndex;
            }
        }
    }else{
        if(scrollPos<m_itemHeight){
            newVisibleStartIndex=0;
        }else{
            newVisibleStartIndex=scrollPos/(qint64)m_itemHeight;
        }
        if(bottomPos<=0){
            newVisibleEndIndex=0;
        }else{
            newVisibleEndIndex=(bottomPos+m_itemHeight-1)/(qint64)m_itemHeight;
        }
    }
    if(newVisibleStartIndex<0)
        newVisibleStartIndex=0;
    if(newVisibleEndIndex>m_itemsCount){
        newVisibleEndIndex=m_itemsCount;
    }
    if(newVisibleEndIndex<=newVisibleStartIndex){
        // no item is visible.
        foreach(QGraphicsItem *items, m_loadedItems){
            delete items;
        }
        m_loadedItems.clear();
    }else{
        if(newVisibleStartIndex>=oldVisibleEndIndex ||
                newVisibleEndIndex<=oldVisibleStartIndex){
            foreach(QGraphicsItem *items, m_loadedItems){
                delete items;
            }
            m_loadedItems.clear();
        }
        if(m_loadedItems.isEmpty()){
            m_loadedItems.reserve(newVisibleEndIndex-newVisibleStartIndex);
            for(qint64 i=newVisibleStartIndex;i<newVisibleEndIndex;i++){
                QGraphicsItem *item=createView(i, m_clipView);
                m_loadedItems.push_back(item);
            }
        }else{
            m_loadedItems.reserve(newVisibleEndIndex-newVisibleStartIndex);
            while(oldVisibleStartIndex<newVisibleStartIndex){
                delete m_loadedItems.takeFirst();
                oldVisibleStartIndex++;
            }
            while(oldVisibleEndIndex>newVisibleEndIndex){
                delete m_loadedItems.takeLast();
                oldVisibleEndIndex--;
            }
            while(newVisibleStartIndex<oldVisibleStartIndex){
                oldVisibleStartIndex--;
                QGraphicsItem *item=createView(oldVisibleStartIndex, m_clipView);
                m_loadedItems.prepend(item);
            }
            while(newVisibleEndIndex>oldVisibleEndIndex){
                QGraphicsItem *item=createView(oldVisibleEndIndex, m_clipView);
                m_loadedItems.append(item);
                oldVisibleEndIndex++;
            }
        }
    }

    m_loadedFirstIndex=newVisibleStartIndex;


    // layout items.
    qint64 index=m_loadedFirstIndex;
    scrollPos=(qint64)m_scrollBar->value()+topClipPos;
    qint64 pos=index*(qint64)m_itemHeight+m_topItemPos-scrollPos;
    foreach(QGraphicsItem *item, m_loadedItems){
        if(index==qMax(m_animatedFirstIndex,m_loadedFirstIndex) &&
                animatedItemsShift!=0){
            pos+=animatedItemsShift;
        }

        item->setPos(0, pos);
        item->setZValue((index>=m_animatedFirstIndex)?2.f:1.f);
        pos+=m_itemHeight;
        index++;
    }

    foreach(const RemovedItem& item, m_itemsBeingRemoved){
        QGraphicsItem *view=item.view;
        view->setPos(0, item.originalPos-scrollPos);
        view->setZValue(0.f);
    }

    // purge old removed items
    if(!m_itemsBeingRemoved.isEmpty()){
        bool innerAnimationRunning=m_animatedItemsPos.isAnimating(curTime);
        QVarLengthArray<QLinkedList<RemovedItem>::Iterator, 64> trash;
        for(QLinkedList<RemovedItem>::Iterator it=m_itemsBeingRemoved.begin();
            it!=m_itemsBeingRemoved.end();it++){
            RemovedItem& item=*it;
            if(item.lowestItem){
                if(bottomClipPos<=item.originalPos+4){
                    trash.append(it);
                }
            }else{
                if(!innerAnimationRunning){
                    trash.append(it);
                }
            }
        }
        for(int i=0;i<trash.count();i++){
            QLinkedList<RemovedItem>::Iterator it=trash[i];
            RemovedItem& item=*it;
            delete item.view;
            m_itemsBeingRemoved.erase(it);
        }
    }

    // set clip view
    qint64 clipHeight=bottomClipPos-topClipPos;
    if(clipHeight<listHeight){
        m_clipView->setFlag(ItemClipsChildrenToShape);
        m_clipView->setRect(QRectF(0,0,m_size.width(), clipHeight));
    }else{
        m_clipView->setFlag(ItemClipsChildrenToShape, false);
    }

}

qint64 STStaticListView::insertItem(qint64 index, qint64 count){
    if(count==0)
        return index;
    if(index<=0){
        // prepending
        m_itemsCount+=count;
        m_loadedFirstIndex+=count;
        m_topItemPos-=count*(qint64)m_itemHeight;
        if(m_itemsCount==count){
            // initial items
            m_topClipPos.setPos(m_topItemPos-(qint64)m_size.height());
        }
        m_topClipPos.setPosAnimated(m_topItemPos, 200, currentTime());
        startAnimation();
        return 0;
    }else if(index>=m_itemsCount){
        // appending
        m_itemsCount+=count;
        m_bottomClipPos.setPosAnimated(m_bottomClipPos.finalPos()+(double)(count*(qint64)m_itemHeight),
                                       200, currentTime());
        startAnimation();
        return m_itemsCount-count;
    }else{
        // insertig
        completeInnerAnimation();
        m_itemsCount+=count;
        if(index<=m_loadedFirstIndex){
            m_loadedFirstIndex+=count;
        }
        m_bottomClipPos.setPosAnimated(m_bottomClipPos.finalPos()+(double)(count*(qint64)m_itemHeight),
                                       200, currentTime());
        m_animatedFirstIndex=index+count;
        m_animatedItemsPos.setPos(-(count*(qint64)m_itemHeight));
        m_animatedItemsPos.setPosAnimated(0, 200, currentTime());
        startAnimation();
        return index;
    }

}

qint64 STStaticListView::appendItem(qint64 count){
    return insertItem(m_itemsCount,  count);
}

qint64 STStaticListView::prependItem(qint64 count){
    return insertItem(0, count);
}

void STStaticListView::markItemsToDelete(qint64 startIndex, qint64 endIndex, bool lowest){
    qint64 loadedCount=m_loadedItems.count();
    if(loadedCount==0){
        // nothing to do
    }else{
        qint64 loadedStartIndex=m_loadedFirstIndex;
        qint64 loadedEndIndex=loadedStartIndex+loadedCount;
        if(endIndex<=loadedStartIndex){
            m_loadedFirstIndex-=endIndex-startIndex;
        }else if(startIndex>=loadedEndIndex){
            // nothing to do
        }else{
            if(startIndex<=loadedStartIndex){
                if(endIndex<loadedEndIndex){
                    //      XXXXXXXXXXXXX
                    //          [               ]
                    qint64 removeCount=endIndex-loadedStartIndex;
                    qint64 pos=m_topItemPos;
                    pos+=loadedStartIndex*(qint64)m_itemHeight;
                    while(removeCount--){
                        QGraphicsItem *item=m_loadedItems.takeFirst();


                        RemovedItem rItem;
                        rItem.view=item;
                        rItem.originalPos=pos;
                        rItem.lowestItem=lowest;
                        m_itemsBeingRemoved.push_back(rItem);
                        pos+=m_itemHeight;
                    }
                    m_loadedFirstIndex=startIndex;
                }else{
                    //    XXXXXXXXXXXXXXXXXXXX
                    //        [          ]
                    qint64 pos=m_topItemPos;
                    pos+=loadedStartIndex*(qint64)m_itemHeight;
                    foreach(QGraphicsItem *item, m_loadedItems){


                        RemovedItem rItem;
                        rItem.view=item;
                        rItem.originalPos=pos;
                        rItem.lowestItem=lowest;
                        m_itemsBeingRemoved.push_back(rItem);
                        pos+=m_itemHeight;
                    }
                    m_loadedItems.clear();
                }
            }else{
                if(endIndex>=loadedEndIndex){
                    //       XXXXXXXXXXXXXX
                    //    [             ]
                    qint64 removeCount=loadedEndIndex-startIndex;
                    qint64 pos=m_topItemPos;
                    pos+=loadedEndIndex*(qint64)m_itemHeight;
                    while(removeCount--){
                        pos-=m_itemHeight;
                        QGraphicsItem *item=m_loadedItems.takeLast();

                        RemovedItem rItem;
                        rItem.view=item;
                        rItem.originalPos=pos;
                        rItem.lowestItem=lowest;
                        m_itemsBeingRemoved.push_back(rItem);
                    }
                }else{
                    //       XXXXXXXXXXXXX
                    //    [                   ]
                    // slow removal. change approach.
                    qint64 pos=m_topItemPos;
                    pos+=startIndex*(qint64)m_itemHeight;
                    for(qint64 i=startIndex;i<endIndex;i++){
                        QGraphicsItem *item=m_loadedItems[i-loadedStartIndex];


                        RemovedItem rItem;
                        rItem.view=item;
                        rItem.originalPos=pos;
                        rItem.lowestItem=lowest;
                        m_itemsBeingRemoved.push_back(rItem);
                        pos+=m_itemHeight;
                    }
                    // remove later
                    m_loadedItems.erase(m_loadedItems.begin()+(startIndex-loadedStartIndex),
                                        m_loadedItems.begin()+(endIndex-loadedStartIndex));
                }
            }
        }
    }
}

void STStaticListView::removeItem(qint64 startIndex, qint64 count){
    qint64 endIndex=count+startIndex;
    if(startIndex<0)startIndex=0;
    if(endIndex>m_itemsCount)endIndex=m_itemsCount;
    if(startIndex>=endIndex)
        return;
    count=endIndex-startIndex;

    completeInnerAnimation();

    if(endIndex==m_itemsCount){
        // pop
        markItemsToDelete(startIndex, endIndex, true);
        m_itemsCount-=count;
        m_bottomClipPos.setPosAnimated(m_bottomClipPos.finalPos()-(double)(count*(qint64)m_itemHeight),
                                       200, currentTime());
        startAnimation();
    }else{
        markItemsToDelete(startIndex, endIndex, false);
        m_itemsCount-=count;
        m_animatedFirstIndex=startIndex;
        m_animatedItemsPos.setPos(count*(qint64)m_itemHeight);
        m_animatedItemsPos.setPosAnimated(0, 200, currentTime());
        m_bottomClipPos.setPosAnimated(m_bottomClipPos.finalPos()-(double)(count*(qint64)m_itemHeight),
                                       200, currentTime());
        startAnimation();
    }

}

void STStaticListView::completeInnerAnimation(){
    foreach(const RemovedItem& item, m_itemsBeingRemoved){
        delete item.view;
    }
    m_itemsBeingRemoved.clear();

    m_animatedFirstIndex=-1;
    m_animatedItemsPos.setPos(0.);

    relayout();
}

void STStaticListView::setItemHeight(int newItemHeight){
    if(m_itemHeight==newItemHeight)
        return;
    m_itemHeight=newItemHeight;
    relayout();
}


bool STStaticListView::shouldAnimate(){
    quint64 curTime=currentTime();
    return m_topClipPos.isAnimating(curTime)||m_bottomClipPos.isAnimating(curTime)||
            m_animatedItemsPos.isAnimating(curTime);
}

void STStaticListView::startAnimation(){
    if(m_timerId)
        return;
    m_timerId=startTimer(15);
}

void STStaticListView::timerEvent(QTimerEvent *){
    bool shouldAnim=shouldAnimate();
    relayout();
    if(!shouldAnim){
        killTimer(m_timerId);
        m_timerId=0;
    }
}

int STStaticListView::itemPreferredWidth() const{
    return (int)m_scrollBar->pos().x();
}

QGraphicsItem *STStaticListView::createView(qint64 id, STClipView *parent){
    QSize size(itemPreferredWidth(),itemHeight());
    QImage img(size, QImage::Format_RGB32);
    img.fill(0);

    QPainter painter(&img);
    QLinearGradient grad(0.f, 0.f, 0.f, itemHeight());
    grad.setColorAt(0.f, QColor(75,75,75));
    grad.setColorAt(1.f, QColor(65,65,65));
    painter.fillRect(QRect(0, 0, m_size.width(), m_size.height()), QBrush(grad));

    QString text=QString("ID = %1").arg(id);
    STFont::defaultFont()->drawString(img, QPointF(4, (itemHeight()-12.f)/2.f),
                                      QColor(240,240,240), text);


    QGraphicsPixmapItem *item=new QGraphicsPixmapItem(QPixmap::fromImage(img), parent);

   // qDebug()<<"Created: "<<id;

    return item;
}

void STStaticListView::wheelEvent(QGraphicsSceneWheelEvent *event){
    m_scrollBar->setValue(m_scrollBar->value()-event->delta());
}

