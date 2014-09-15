#ifndef STBASETABPAGEVIEW_H
#define STBASETABPAGEVIEW_H

#include <QGraphicsObject>
#include <QList>
#include "stviewanimator.h"
#include "stobjectmanager.h"

class STPane;
class STAccount;
class STBaseTabPageView : public QGraphicsObject
{
    Q_OBJECT    

    QSize m_size;
    QList<STPane *> m_paneStack;

    STViewAnimator m_animator;
    quint64 m_accountId;

public:
    explicit STBaseTabPageView(quint64 accountId, QGraphicsItem *parent);
    
    void setSize(const QSize& size);
    QSize size() const{return m_size;}
    virtual void relayout();

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual QRectF boundingRect() const;

    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);

    void pushPane(STPane *);
    void popPane();

    void pushPaneAnimated(STPane *);
    void popPaneAnimated();

    void swapPane(STPane *);

    bool canPopPane();

    STPane *currentPane();
    STPane *previousPaneFor(STPane *);

    STAccount *account();

    void linkActivate(STObjectManager::EntityRange *,
                      STObjectManager::LinkActivationInfo);

signals:
    

    void headerViewContentsChanged(int opening);

public slots:
    void paneHeaderViewContentsChanged(int opening);
};

#endif // STBASETABPAGEVIEW_H
