#ifndef STTABVIEW_H
#define STTABVIEW_H

#include <QGraphicsObject>
#include <QList>

class STButtonView;

class STTabView : public QGraphicsObject
{
    Q_OBJECT
public:


    explicit STTabView(int tabCount, QGraphicsItem *parent = 0);
    
    void relayout();
    void setSize(const QSize& sz){
        prepareGeometryChange();
        m_size=sz;
        relayout();
    }

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual QRectF boundingRect() const;

    int currentTab() const{return m_currentTab;}
    void setTabText(int tabIndex, const QString&);
    void setTabTooltip(int tabIndex,const QString&);
    void setCurrentTab(int);

private:

    int m_currentTab;
    QSize m_size;
    QList<STButtonView *> m_tabs;


signals:
    
    void currentTabChanged();

private slots:
    void tabClicked();
};

#endif // STTABVIEW_H
