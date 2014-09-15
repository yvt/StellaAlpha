#ifndef STUSERLISTVIEW_H
#define STUSERLISTVIEW_H

#include "ststdafx.h"
#include "ststaticlistview.h"
#include "stobjectmanager.h"

class STUserListModel;
class STUserListFetchMoreView;

class STUserListView : public STStaticListView
{
    Q_OBJECT

    STUserListModel *m_model;
public:
    explicit STUserListView(STUserListModel *,QGraphicsItem *parent = 0);
    
protected:
    virtual QGraphicsItem *createView(qint64 index, STClipView *parent);

    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);

signals:
    
    void fetchingUpdated(bool);
    void linkActivated(STObjectManager::EntityRange *,
                       STObjectManager::LinkActivationInfo);

private slots:

    void itemLinkActivated(STObjectManager::EntityRange *,
                           STObjectManager::LinkActivationInfo);

public slots:
    
    void someUsersLoaded();

    void userIdsPrepended(quint64 count);
    void userIdsAppended(qint64 count);
    void userIdRemoved(qint64 index, qint64 rowId, quint64 userId);

    void userIdFetchDone();
    void userIdFetchFailed();

    void startUserIdFetch();

};

#endif // STUSERLISTVIEW_H
