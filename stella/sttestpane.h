#ifndef STTESTPANE_H
#define STTESTPANE_H

#include "stpane.h"
class STDynamicListView;
class STStaticListView;
class STTestPane : public STPane
{
    Q_OBJECT

    STStaticListView *m_lv;
public:
    explicit STTestPane(QGraphicsItem *parent);
    virtual void relayout();
    virtual STHeaderView::Contents headerViewContents();
    virtual void backButtonActivated();
    virtual void timerEvent(QTimerEvent *);
signals:
    
public slots:
    
};

#endif // STTESTPANE_H
