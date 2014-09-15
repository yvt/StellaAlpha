#include "sttestpane.h"
#include "stdynamiclistview.h"
#include "ststaticlistview.h"
#include "sttimelinepane.h"

STTestPane::STTestPane(QGraphicsItem *parent) :
    STPane(parent)
{
    m_lv=new STStaticListView(this);
    //m_lv->appendItem(1000);
}

void STTestPane::relayout(){
    m_lv->setSize(size());
}

STHeaderView::Contents STTestPane::headerViewContents(){
    STHeaderView::Contents ct=STPane::headerViewContents();
    ct.title="List View Test";
    ct.hasBackButton=true;
    return ct;
}

void STTestPane::backButtonActivated(){/*
    static quint64 k=1;
    for(int i=0;i<1000;i++)
    m_lv->insertItem(k++,24.);*/
    //startTimer(1000)
   // m_lv->insertItem(0, (qint64)rand()*((qint64)rand()>>16))

    if(isPaneActive()){
        STTimelinePane *p=new STTimelinePane();
        p->setTimelineIdentifier("570915827.sample");
        pushPaneAnimated(p);
    }

;}

void STTestPane::timerEvent(QTimerEvent *){

    //m_lv->removeItem(rand()%10);
    //m_lv->removeItem(rand()%m_lv->itemCount());
    /*

    static quint64 k=1;
    for(int i=0;i<1;i++)
        m_lv->insertItem(k++,20.);*/
    /*
    for(int i=0;i<10;i++){
        m_lv->insertItem(i,20.);
       // m_lv->removeItem(1+(rand()%100));
    }*/
    //k+=rand()>>1;
}
