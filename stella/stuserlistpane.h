#ifndef STUSERLISTPANE_H
#define STUSERLISTPANE_H

#include "ststdafx.h"
#include "stpane.h"

class STUserListView;
class STUserListModel;

class STUserListPane : public STPane
{
    Q_OBJECT

    STUserListView *m_view;
    STUserListModel *m_model;
public:
    explicit STUserListPane(STUserListModel *model, QGraphicsItem *parent = 0);
    
    virtual void relayoutContents();

    virtual STHeaderView::Contents headerViewContents();

signals:
    
public slots:
    
};

#endif // STUSERLISTPANE_H
