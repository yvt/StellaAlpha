#include "sttabview.h"
#include <QImage>
#include <QLinearGradient>
#include <QPainter>
#include <QBrush>
#include "stbuttonview.h"

STTabView::STTabView(int tabCount, QGraphicsItem *parent) :
    QGraphicsObject(parent)
{
    setCacheMode(DeviceCoordinateCache);

    m_currentTab=0;

    for(int i=0;i<(int)tabCount;i++){
        STButtonView *view=new STButtonView(this);
        view->setChecked(i==(int)m_currentTab);
        m_tabs.push_back(view);

        connect(view, SIGNAL(activated()),
                this, SLOT(tabClicked()));
    }
/*
    m_tabs[TimelineTab]->setText(":/stella/res/HomeIcon.png");
    m_tabs[MentionsTab]->setText(":/stella/res/MentionIcon.png");
    m_tabs[MessagesTab]->setText(":/stella/res/MessageIcon.png");
    m_tabs[SearchTab]->setText(":/stella/res/SearchIcon.png");
    m_tabs[ProfileTab]->setText(":/stella/res/UserIcon.png");
*/
    setFlag(ItemIsFocusable);

}

void STTabView::setTabText(int tabIndex, const QString &text){
    m_tabs[tabIndex]->setText(text);
}

void STTabView::setTabTooltip(int tabIndex, const QString &text){
    m_tabs[tabIndex]->setToolTip(text);
}
void STTabView::relayout(){
    this->update();
    int cols = 5;
    int col = 0;

    cols = m_tabs.count();
    for(int i=0;i<m_tabs.count();i++){
        STButtonView *view=m_tabs[i];
        int colSpan = 1;
        if((i == 1  && m_tabs.count()==3) ||
                (i==0 && m_tabs.count()==4)){
           // colSpan = 6 - m_tabs.count();
        }
        QRect rt(col*(m_size.width()+1)/cols-1, 0,
                 (col+colSpan)*(m_size.width()+1)/cols-col*(m_size.width()+1)/cols+1,
                 m_size.height()+1);
        view->setSize(rt.size());
        view->setPos(rt.x(), rt.y());
        col+=colSpan;
    }
}

void STTabView::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    QLinearGradient grad(0.f, 0.f, 0.f, m_size.height());
    grad.setColorAt(0.f, QColor(110, 110, 110));
    grad.setColorAt(1.f, QColor(100, 100, 100));
    //grad.setColorAt(1.f, QColor(0,0,0));
    painter->fillRect(QRect(0, 1, m_size.width(), m_size.height()-1), QBrush(grad));
    painter->fillRect(QRect(0, 0, m_size.width(), 1),
                      QColor(60,60,60));

}

QRectF STTabView::boundingRect() const{
    return QRectF(0,0,m_size.width(),m_size.height());
}

void STTabView::tabClicked() {
    for(int i=0;i<(int)m_tabs.count();i++){
        STButtonView *view=m_tabs[i];
        if(sender()==view){
            m_currentTab=(int)i;
        }
    }

    for(int i=0;i<(int)m_tabs.count();i++){
        STButtonView *view=m_tabs[i];
        view->setChecked(i==(int)m_currentTab);
    }

    emit currentTabChanged();
}

void STTabView::setCurrentTab(int t){
    if(t==m_currentTab) return;
    m_currentTab=t;
    for(int i=0;i<(int)m_tabs.count();i++){
        STButtonView *view=m_tabs[i];
        view->setChecked(i==(int)m_currentTab);
    }
    emit currentTabChanged();
}
