#ifndef STSTATUSPANE_H
#define STSTATUSPANE_H

#include "stpane.h"
#include "stobjectmanager.h"

class STUserIconView;
class STStatusUserNameView;
class STStatusContentsView;

class STStatusPane : public STPane
{
    Q_OBJECT

    STAccount *m_account;

    quint64 m_statusId;
    STObjectManager::StatusRef m_status;

    STStatusUserNameView *m_nameView;
    STStatusContentsView *m_contentsView;
    STUserIconView*m_iconView;

public:
    explicit STStatusPane(quint64 statusId, STAccount *acc, QGraphicsItem *parent = 0);
    void setStatus(STObjectManager::StatusRef, bool animated=false);

    virtual void relayoutContents();;

    virtual STHeaderView::Contents headerViewContents();

    STAccount *account() const{return m_account;}

signals:
    
public slots:
    void showUser();
};

#endif // STSTATUSPANE_H
