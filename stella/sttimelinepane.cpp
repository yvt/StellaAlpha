#include "sttimelinepane.h"
#include "sttimelineview.h"
#include <QStringList>
#include <QDebug>

STTimelinePane::STTimelinePane(QGraphicsItem *parent) :
    STPane(parent)
{
    m_timelineView=NULL;
    m_animator.setVisibleOnlyWhenParentIsVisible(true);
}
STTimelinePane::~STTimelinePane(){
}

STTimelineView *STTimelinePane::createView(QString idt){
    STTimelineView *newView=new STTimelineView(idt, this);
    connect(newView, SIGNAL(linkActivated(STObjectManager::EntityRange*,
                                          STObjectManager::LinkActivationInfo)),
            this, SLOT(linkActivate(STObjectManager::EntityRange*,
                                    STObjectManager::LinkActivationInfo)));
    return newView;
}

void STTimelinePane::relayout(){
    if(m_timelineView){
        m_timelineView->setPos(m_animator.viewXPos(m_timelineView), 0.f);
        m_timelineView->setSize(this->size());
        m_timelineView->setVisible(m_animator.isViewVisible(m_timelineView));
    }
}

void STTimelinePane::setTimelineIdentifier(QString idt){
    if(idt==timelineIdentifier())return;

    STTimelineView *newView=createView(idt);

    m_animator.activateView(newView);
    if(m_timelineView)
        delete m_timelineView;
    m_timelineView=newView;

    emit headerViewContentsChanged(0);
}

void STTimelinePane::setTimelineIdentifierAnimated(QString idt){
    if(idt==timelineIdentifier())return;

    STTimelineView *newView=createView(idt);
    m_animator.activateViewAnimated(newView, 1, 200, true);
    m_timelineView=newView;

    emit headerViewContentsChanged(1);
}

QString STTimelinePane::timelineIdentifier(){
    if(m_timelineView)
        return m_timelineView->timelineIdentifier();
    return QString();
}

STHeaderView::Contents STTimelinePane::headerViewContents(){
    STHeaderView::Contents ct=STPane::headerViewContents();
    QStringList lst=timelineIdentifier().split(".");
    if(lst[1]=="sample"){
        ct.title=tr("Public Timeline");
    }else if(lst[1]=="home"){
        ct.title=tr("Home Timeline");
    }else if(lst[1]=="mentions"){
        ct.title=tr("Mentions");
    }else if(lst[1]=="user"){
        ct.title=tr("Tweets"); // TODO: show user name
    }else if(lst[1]=="search"){
        lst.pop_front(); lst.pop_front();
        ct.title=lst.join(".");
    }

    if(!m_title.isEmpty())
        ct.title=m_title;

    return ct;
}

void STTimelinePane::setTitle(QString t){
    m_title=t;
    emit headerViewContentsChanged(0);
}

QVariant STTimelinePane::itemChange(GraphicsItemChange change, const QVariant &value){
    if(change==ItemVisibleHasChanged)
        relayout();
    return STPane::itemChange(change, value);
}
