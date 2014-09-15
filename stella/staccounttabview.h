#ifndef STACCOUNTTABVIEW_H
#define STACCOUNTTABVIEW_H

#include <QGraphicsObject>
#include <QVector>
#include "stviewanimator.h"

class STBaseTabPageView;
class STPane;
class STAccount;

class STAccountTabView : public QGraphicsObject
{
    Q_OBJECT

    quint64 m_accountId;
    STAccount *m_account;

    QVector<STBaseTabPageView *> m_tabPages;
    int m_currentTab;

    QSize m_size;

    STViewAnimator m_animator;

    void relayout();

public:
    explicit STAccountTabView(QGraphicsItem *parent ,quint64 accountId);
    
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *widget);
    virtual QRectF boundingRect() const;

    void setSize(const QSize&);

    STPane *currentPane();
    STBaseTabPageView *currentTabView();

    int currentTab() const{return m_currentTab;}
    void setCurrentTab(int);
    void setCurrentTabAnimated(int);

signals:

    void headerViewContentsChanged(int opening);
    
public slots:
    
    void paneHeaderViewContentsChanged(int opening);
};

#endif // STACCOUNTTABVIEW_H
