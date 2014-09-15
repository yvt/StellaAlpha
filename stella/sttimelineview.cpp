#include "sttimelineview.h"
#include "sttimeline.h"
#include <QPainter>
#include <QLinearGradient>
#include <QBrush>
#include "sttimelineitemview.h"
#include <QDebug>
#include "sttimelinefetchmoreview.h"
#include "staccount.h"

STTimelineView::STTimelineView(QString idt, QGraphicsItem *parent) :
    STDynamicListView(parent)
{
    m_timeline=STTimeline::createTimeline(idt, this);
    Q_ASSERT(m_timeline);

    m_accountId=m_timeline->accountForIdentifier(m_timeline->identifier())->userId();
    this->setFollowMode(AutoFollow);

    connect(m_timeline, SIGNAL(rowAdded(quint64)),
            this, SLOT(timelineAddRow(quint64)));
    connect(m_timeline, SIGNAL(rowRemoved(quint64)),
            this, SLOT(timelineRemoveRow(quint64)));
    connect(m_timeline, SIGNAL(initateBulkLoad()),
            this, SLOT(initateBulkLoad()));

    m_loadTimer.setInterval(10);
    m_loadTimer.setSingleShot(true);
    connect(&m_loadTimer, SIGNAL(timeout()),
            this, SLOT(loadTimeline()));
    m_loadTimer.start();


}

QVariant STTimelineView::itemChange(GraphicsItemChange change, const QVariant &value){
    if(change==ItemVisibleHasChanged){
        setFollowMode(isVisible()?AutoFollow:NeverFollow);
    }
    return STDynamicListView::itemChange(change, value);
}

void STTimelineView::loadTimeline(){
    QList<quint64> keys=m_timeline->allRowIds();
    QList<QPair<quint64, double> > items;
    items.reserve(keys.count());
    foreach(quint64 id, keys){
        items.append(QPair<quint64, double>(id, heightForRow(id)));

    }
    this->setItems(items);

    m_timeline->start();
}

STTimelineView::~STTimelineView(){
    m_timeline->stop();
}

STFont::Layout STTimelineView::layoutRow(STTimeline::Row *row){
    QVariant txt=row->status->data["text"];
    QString text=txt.toString();
    return STFont::defaultFont()->layoutString(text, itemPreferredWidth()-4);
}

double STTimelineView::heightForRow(quint64 id){
    STTimeline::Row *row=m_timeline->row(id);
    if(!row){
        qDebug()<<"STTimelineView::heightForRow: row "<<id<<"not found.";
        return 0.;
    }
    if(!row->status){
        return STTimelineFetchMoreView::defaultSize(itemPreferredWidth()).height();
    }
    return STTimelineItemView::estimateSize(itemPreferredWidth(), row->status).height();
}

void STTimelineView::timelineAddRow(quint64 id){
    this->insertItem(id, heightForRow(id));
}

void STTimelineView::timelineRemoveRow(quint64 id){
    this->removeItem(id);
}

QString STTimelineView::timelineIdentifier() const{
    return m_timeline->identifier();
}

QGraphicsItem *STTimelineView::createView(quint64 id, double height, STClipView *parent){

    STTimeline::Row *row=m_timeline->row(id);
    if(!row)return NULL;
    if(!row->status){
        return new STTimelineFetchMoreView(QSize(this->itemPreferredWidth(), (int)height),
                                           m_timeline, id, parent);
    }
    Q_ASSERT(row->status);
    STTimelineItemView *view=new STTimelineItemView(QSize(this->itemPreferredWidth(), (int)height),
                                  row->status,m_accountId, parent);
    connect(view, SIGNAL(linkActivated(STObjectManager::EntityRange*,
                                       STObjectManager::LinkActivationInfo)),
            this, SLOT(linkActivatedSlot(STObjectManager::EntityRange*,
                                         STObjectManager::LinkActivationInfo)));
    return view;
}

void STTimelineView::linkActivatedSlot(STObjectManager::EntityRange *er,
                                       STObjectManager::LinkActivationInfo i){
    emit linkActivated(er, i);
}
