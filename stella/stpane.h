#ifndef STPANE_H
#define STPANE_H

#include <QGraphicsObject>
#include "stheaderview.h"
#include "stobjectmanager.h"

class STBaseTabPageView;

class STPane : public QGraphicsObject
{
    Q_OBJECT

    QSize m_size;


public:
    explicit STPane(QGraphicsItem *parent = 0);
    void setSize(const QSize&);
    virtual void relayout();
    virtual void relayoutContents();

    virtual void setContentsHeight(float);
    virtual int contentsWidth();
    
    virtual QGraphicsItem *contentsView() const{return const_cast<STPane *>(this);}

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual QRectF boundingRect() const;
    virtual STHeaderView::Contents headerViewContents();

    QSize size() const{return m_size;}

    STBaseTabPageView *controller();
    STPane *previousPane();

protected slots:


    void pushPaneAnimated(STPane *);
    void pushPane(STPane *);
    void popPaneAnimated();
    void popPane();
    void swapPane(STPane *);
    void linkActivate(STObjectManager::EntityRange *,
                      STObjectManager::LinkActivationInfo);

protected:

    bool canPopPane();
    bool isPaneActive();

    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);

signals:
    
    void headerViewContentsChanged(int opening);

public slots:

    virtual void backButtonActivated();
};

#endif // STPANE_H
