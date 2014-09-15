#ifndef STSCROLLEDPANE_H
#define STSCROLLEDPANE_H

#include "stpane.h"
#include "stscrollbarview.h"
#include <QGraphicsRectItem>

class STScrolledPane : public STPane
{
    Q_OBJECT

    STScrollBarView *m_scrollbar;
    QGraphicsRectItem *m_contentsView;

public:
    explicit STScrolledPane(QGraphicsItem *parent = 0);
    
    virtual void relayout();
    virtual void relayoutContents();

    virtual void setContentsHeight(float);
    virtual int contentsWidth();

    virtual QGraphicsItem *contentsView() const{return m_contentsView;}

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual void wheelEvent(QGraphicsSceneWheelEvent *event);
signals:
    
private slots:
    void scrollChanged();
};

#endif // STSCROLLEDPANE_H
