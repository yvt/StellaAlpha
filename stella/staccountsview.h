#ifndef STACCOUNTSVIEW_H
#define STACCOUNTSVIEW_H

#include "ststdafx.h"
#include "stdynamiclistview.h"

class STAccount;

class STAccountsView : public STDynamicListView
{
    Q_OBJECT

    QMap<quint64, quint64> m_rowIdForAccountId;
    QMap<quint64, quint64> m_accountIdForRowId;

    quint64 m_selectedAccountId;

    quint64 minRowId();
public:
    explicit STAccountsView(QGraphicsItem *parent = 0);
    
    virtual void relayoutScrollBar();

    bool validateStructure();

protected:
    virtual QGraphicsItem *createView(quint64 id, double height, STClipView *parent);
    virtual void itemMoved(quint64 oldId, quint64 newId);
    QSize itemSizeForAccount(STAccount *);
signals:
    void accountSelected(quint64);

    void selectedAccountChanged(quint64); // for buttons
public slots:
    void loadView();
    void accountWasAdded(quint64);
    void accountBeingRemoved(quint64);

    void setSelectedAccount(quint64);

private slots:
    void accountScreenNameChanged();
    void accountButtonTriggered();
    void dragStarting(quint64, QPointF);
};

#endif // STACCOUNTSVIEW_H
