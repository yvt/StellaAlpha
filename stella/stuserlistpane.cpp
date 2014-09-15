#include "stuserlistpane.h"
#include "stuserlistview.h"
#include "stfollowersuserlistmodel.h"
#include "stfriendsuserlistmodel.h"

STUserListPane::STUserListPane(STUserListModel *model, QGraphicsItem *parent) :
    STPane(parent),m_model(model)
{
    m_view=new STUserListView(model, contentsView());
    connect(m_view, SIGNAL(linkActivated(STObjectManager::EntityRange*,
                                         STObjectManager::LinkActivationInfo)),
            this, SLOT(linkActivate(STObjectManager::EntityRange*,
                                    STObjectManager::LinkActivationInfo)));
}

void STUserListPane::relayoutContents(){
    m_view->setSize(size());
}

STHeaderView::Contents STUserListPane::headerViewContents(){
    STHeaderView::Contents contents=STPane::headerViewContents();
    if(dynamic_cast<STFollowersUserListModel *>(m_model)){
        contents.title=tr("Followers");
    }else if(dynamic_cast<STFriendsUserListModel *>(m_model)){
        contents.title=tr("Following Users");
    }
    return contents;
}
