#include "ststandardview.h"
#include <QBrush>
#include <QPen>
#include <QDateTime>
#include <QDebug>
#include <math.h>
#include <QPainter>
#include "stheaderview.h"
#include "stnewsview.h"
#include "sttabview.h"
#include "stsimpleeditview.h"
#include "sttweetview.h"
#include "staccounttabview.h"
#include "stpane.h"
#include "staccountsmanager.h"
#include "stbasetabpageview.h"
#include "staccountsview.h"
#include <QLinearGradient>
#include <QBrush>
#include <QBrush>
#include "stresizeview.h"
#include "stnewaccountview.h"
#include <QGraphicsRectItem>
#include "stnewaccountauthorizeview.h"
#include "staccount.h"
#include "stbuttonview.h"
#include "stlogoview.h"


static const int g_userBarWidth=40;
static const int g_headerHeight=32;
static const int g_sizeBarHeight=10;

STStandardView::STStandardView(QGraphicsItem *parent) :
    QGraphicsObject(parent)
{
    //this->setFlag(ItemHasNoContents);
    setCacheMode(DeviceCoordinateCache);
    this->setFlag(ItemIsFocusable);
    this->setFocus();

    //m_accountTabView=new STAccountTabView(this, 570915827);
    m_viewWrapper=new QGraphicsRectItem(this);
    m_viewWrapper->setFlag(ItemHasNoContents);
    m_viewWrapper->setFlag(ItemClipsChildrenToShape);
    m_viewWrapper->setAcceptedMouseButtons(Qt::NoButton);
    m_newAccountView=new STNewAccountView(m_viewWrapper);
    m_logoView=new STLogoView(m_viewWrapper);
    m_tweetView=new STTweetView(this); // must be below the tab
    m_headerView=new STHeaderView(this);
    m_headerView->setZValue(25.f);
    m_tabView=new STTabView(3, this);
    m_newsView=new STNewsView(this);
    m_newsView->setZValue(10.f);
    m_accountsView=new STAccountsView(this);
    m_accountsView->setZValue(-5.f);
    m_addAccountButton=new STButtonView(this);
    m_addAccountButton->setZValue(-4.f); // just above accounts view

    m_currentAccountId=0;

    QBrush borderColor=QColor(60,60,60);
    QPen pen(QColor(0,0,0,0));
    m_topBorder=new QGraphicsRectItem(this);
    m_topBorder->setBrush(borderColor);
    m_topBorder->setPen(pen);
    m_topBorder->setZValue(20.f);
    m_bottomBorder=new QGraphicsRectItem(this);
    m_bottomBorder->setBrush(borderColor);
    m_bottomBorder->setPen(pen);
    m_bottomBorder->setZValue(20.f);
    m_rightBorder=new QGraphicsRectItem(this);
    m_rightBorder->setBrush(borderColor);
    m_rightBorder->setPen(pen);
    m_rightBorder->setZValue(20.f);
    m_leftBorder=new QGraphicsRectItem(this);
    m_leftBorder->setBrush(borderColor);
    m_leftBorder->setPen(pen);
    m_leftBorder->setZValue(20.f);

    m_resizeView=new STResizeView(this);
    m_resizeView->setZValue(100.f);

    m_tabView->setTabText(0, ":/stella/res/HomeIcon.png");
    m_tabView->setTabText(1, ":/stella/res/MentionIcon.png");
    //m_tabView->setTabText(2, ":/stella/res/MessageIcon.png");
    //m_tabView->setTabText(3, ":/stella/res/SearchIcon.png");
    m_tabView->setTabText(2, ":/stella/res/UserIcon.png");

    m_tabView->setTabTooltip(0, tr("Home Timeline"));
    m_tabView->setTabTooltip(1, tr("Mentions"));
    //m_tabView->setTabTooltip(2, tr("Messages"));
    //m_tabView->setTabTooltip(3, tr("Search"));
    m_tabView->setTabTooltip(2, tr("Your Profile"));

    m_tabView->setCurrentTab(0);

    connect(m_newsView, SIGNAL(currentHeightChanged()),
            this, SLOT(relayout()));

    connect(m_tweetView, SIGNAL(currentHeightChanged()),
            this, SLOT(relayout()));

    connect(m_tweetView, SIGNAL(linkActivated(STObjectManager::EntityRange*,
                                              STObjectManager::LinkActivationInfo)),
            this, SLOT(linkActivated(STObjectManager::EntityRange*,
                                     STObjectManager::LinkActivationInfo)));

    connect(m_tabView, SIGNAL(currentTabChanged()),
            this, SLOT(currentTabChanged()));

    m_newAccountOpacityAnimator=new STSimpleAnimator(this);
    connect(m_newAccountOpacityAnimator, SIGNAL(valueChanged(float)),
            this, SLOT(setTweetTabViewOpacity(float)));

    m_accountAnimator=new STViewAnimator(this);

    connect(STAccountsManager::sharedManager(), SIGNAL(accountWasAdded(quint64)),
            this, SLOT(accountWasAdded(quint64)));
    connect(STAccountsManager::sharedManager(), SIGNAL(accountBeingRemoved(quint64)),
            this, SLOT(accountBeingRemoved(quint64)));
    STAccountList lst=STAccountsManager::sharedManager()->accounts();
    foreach(STAccount *ac, lst){
        createAccountTabView(ac->userId());
    }
    m_newAccountView->setVisible(false);

    // TODO: restore current account id frm the previous session
    m_currentAccountId=lst.isEmpty()?0:lst[0]->userId();

    m_accountsView->setSelectedAccount(m_currentAccountId);
    m_accountAnimator->activateView(m_logoView);
    //

    m_newAccountOpacityAnimator->setValue(0.f);
    m_addAccountButton->setChecked(m_currentAccountId==0);
    m_addAccountButton->setText(":/stella/res/AddAccountIcon.png");
    connect(m_addAccountButton, SIGNAL(activated()),
            this, SLOT(addAccountClicked()));
    m_tweetView->setAccount(STAccountsManager::sharedManager()->account(m_currentAccountId));

    connect(m_newAccountView, SIGNAL(headerViewContentsChanged(int)),
            this, SLOT(paneHeaderViewContentsChanged(int)));

    connect(m_headerView, SIGNAL(backButtonActivated()),
            this, SLOT(backButtonActivated()));

    connect(m_accountsView, SIGNAL(accountSelected(quint64)),
            this, SLOT(setCurrentAccount(quint64)));

    m_headerView->setContents(currentPane()->headerViewContents());

    //m_tweetView->setAccount(STAccountsManager::sharedManager()->account(570915827));

}

void STStandardView::initializeInitialView(){
    // this function is used to prerender the pane's contents so that
    // startup animation becomes smoother.
    relayout();


    int contWidth=m_size.width()-g_userBarWidth-1;
    currentView()->setPos(0, 0);
    if(currentAccountTabView()){
        currentAccountTabView()->setSize(QSize(contWidth, 400));

    }else if(m_newAccountView){
        m_newAccountView->setSize(QSize(contWidth, 400));
    }

    currentView()->setOpacity(1);
    currentView()->setVisible(true);

}

void STStandardView::startOperation(){
    relayout();
    m_accountAnimator->activateViewAnimated(currentView(), 1, 200);
    m_startupAnimator=new STSimpleAnimator(this);
    connect(m_startupAnimator, SIGNAL(valueChanged(float)),
            this, SLOT(startupAnimation(float)));
    m_startupAnimator->setValue(0);
    m_startupAnimator->setValueAnimated(1, 200);
     m_newAccountOpacityAnimator->setValueAnimated(m_currentAccountId?1.f:0.f, 200);
    m_accountsView->loadView();;

}

void STStandardView::startupAnimation(float val){
    m_newsView->setOpacity(val);
    m_addAccountButton->setOpacity(val);
}

void STStandardView::setTweetTabViewOpacity(float op){
    m_tweetView->setOpacity(op);
    m_tabView->setOpacity(op);

    bool tweetable=op>.99f;
    bool visible=op>.01f;
    m_tweetView->setVisible(visible);
    m_tabView->setVisible(visible);
    m_tweetView->setFlag(ItemIsFocusable, tweetable);

    m_authView=NULL;

}


void STStandardView::relayout(){

    const int newsDefaultHeight=20;
    const int tabHeight=30;


    QRect contentsRect(g_userBarWidth, 1, m_size.width()-g_userBarWidth-1, m_size.height()-g_sizeBarHeight-1);
    QRect headerRect(0,0,m_size.width(), g_headerHeight+1);

    m_newsView->setSize(QSize(contentsRect.width(), newsDefaultHeight));
    m_newsView->setPos(contentsRect.left(), contentsRect.bottom()+1-m_newsView->currentHeight());

    int shiftByNews=m_newsView->currentHeight()-newsDefaultHeight;

    m_headerView->setPos(headerRect.left(), headerRect.top());
    m_headerView->setSize(QSize(headerRect.width(), headerRect.height()));
    m_headerView->setUserBarWidth(g_userBarWidth);

    m_tabView->setPos(contentsRect.left(), contentsRect.bottom()+1-newsDefaultHeight-tabHeight-shiftByNews);
    m_tabView->setSize(QSize(contentsRect.width(), tabHeight));

    int tweetViewHeight=m_tweetView->currentHeight();
    m_tweetView->setPos(contentsRect.left(), m_tabView->pos().y()-(float)tweetViewHeight);
    m_tweetView->setWidth(contentsRect.width());

    QGraphicsObject *view=currentView();
    m_accountAnimator->setWidth(contentsRect.width());
    QSize viewSize=QSize(contentsRect.width(), qMax(contentsRect.height()-g_headerHeight-
                                                    tabHeight-newsDefaultHeight-
                                                    m_tweetView->currentHeight(), 10));
    QSize newsViewSize=QSize(viewSize.width(),qMax(contentsRect.height()-g_headerHeight-
                                                  tabHeight+10, 10));

    m_viewWrapper->setPos(contentsRect.left(), g_headerHeight-shiftByNews+contentsRect.top());
    m_viewWrapper->setRect(QRect(QPoint(0,0),newsViewSize));

    view->setPos(m_accountAnimator->viewXPos(view), 0.f);
    foreach(STAccountTabView *acTabView, m_accountTabViews.values()){
        acTabView->setSize(viewSize);
    }


    if(m_newAccountView){
        m_newAccountView->setSize(newsViewSize);
    }

    if(m_logoView){
        m_logoView->setSize(QSize(contentsRect.width(), contentsRect.height()-g_headerHeight));
    }


    // sidebar (accounts)
    QRect sideRect(0,g_headerHeight+1, g_userBarWidth, m_size.height()-g_headerHeight-1-g_sizeBarHeight);
    QRect accountsRect(sideRect.left(), sideRect.top(), sideRect.width(), sideRect.height());
    m_accountsView->setPos(accountsRect.left(), accountsRect.top());
    m_accountsView->setSize(QSize(accountsRect.width(), accountsRect.height()-32));
    m_addAccountButton->setSize(QSize(m_accountsView->itemPreferredWidth()+1, 34));
    m_addAccountButton->setPos(accountsRect.left(), accountsRect.top()+accountsRect.height()-33);

    // borders
    m_topBorder->setRect(contentsRect.left(),contentsRect.top()-1,
                         contentsRect.width(), 1);
    m_bottomBorder->setRect(contentsRect.left(),contentsRect.bottom()+1,
                         contentsRect.width(), 1);
    m_rightBorder->setRect(contentsRect.right()+1,contentsRect.top()-1,
                         1,contentsRect.height()+2);
    m_leftBorder->setRect(contentsRect.left()-1, contentsRect.top()-1,
                          1,contentsRect.height()+2);

    m_resizeView->setPos(0,m_size.height()-g_sizeBarHeight);
    m_resizeView->setSize(QSize(m_size.width(), g_sizeBarHeight));

    if(m_authView)
        m_authView->setSize(m_size);

}



QRectF STStandardView::boundingRect() const{
    return QRectF(0.f, 0.f, (float)m_size.width(), (float)m_size.height());
}
void STStandardView::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *){
    //pt->drawEllipse(0,0,m_size.width(),m_size.height());
    QRect sideRect(0,g_headerHeight+1, g_userBarWidth, m_size.height()-g_headerHeight-1-g_sizeBarHeight);
    QLinearGradient grad(0,0,g_userBarWidth,0);
   // grad.setColorAt(0, QColor(190,190,190));
    //grad.setColorAt(1, QColor(170,170,170));

#if 0
    grad.setColorAt(0, QColor(80,80,80));
    grad.setColorAt(0.1f, QColor(90,90,90));
    grad.setColorAt(0.2f, QColor(95,95,95));
    grad.setColorAt(0.5f, QColor(96,96,96));
    grad.setColorAt(0.7f, QColor(95,95,95));
    grad.setColorAt(0.8f, QColor(90,90,90));
    grad.setColorAt(1, QColor(80,80,80)); // dark sidebar
#else
    grad.setColorAt(0, QColor(70,70,70));
    grad.setColorAt(0.1f, QColor(80,80,80));
    grad.setColorAt(0.2f, QColor(85,85,85));
    grad.setColorAt(0.5f, QColor(86,86,86));
    grad.setColorAt(0.7f, QColor(85,85,85));
    grad.setColorAt(0.8f, QColor(80,80,80));
    grad.setColorAt(1, QColor(70,70,70)); // dark sidebar, more darkness
#endif

    painter->fillRect(sideRect, grad);

    QColor borderColor(60,60,60);
    QColor brightBorderColor(106,106,106);
    brightBorderColor=borderColor; // dark sidebar
    //painter->fillRect(0,0,m_size.width(), 1, borderColor);
    painter->fillRect(0,m_size.height()-g_sizeBarHeight,sideRect.width(), 1, brightBorderColor);
    painter->fillRect(sideRect.width(),m_size.height()-g_sizeBarHeight,m_size.width()- sideRect.width(), 1, borderColor);

    painter->fillRect(0,0,1, m_size.height(), borderColor);
}

void STStandardView::currentTabChanged(){
    if(!currentAccountTabView()){
        qWarning()<<"STStandardView::currentTabChanged: currentAccountTabView()==NULL";
        return;
    }
    currentAccountTabView()->setCurrentTabAnimated((int)m_tabView->currentTab());
}

void STStandardView::paneHeaderViewContentsChanged(int opening){
    if(currentView()!=sender())
        return;
    STHeaderView::Contents newContents=(currentPane()->headerViewContents());
    if(opening==1)
        m_headerView->setContentsAnimated(newContents, STHeaderView::SlideToLeft);
    else if(opening==-1)
        m_headerView->setContentsAnimated(newContents, STHeaderView::SlideToRight);
    else if(opening==0)
        m_headerView->setContents(newContents);
}


STPane *STStandardView::currentPane(){
    if(!currentAccountTabView())
        return m_newAccountView->currentPane();
    return currentAccountTabView()->currentPane();
}

void STStandardView::backButtonActivated(){
    if(currentPane()->headerViewContents().hasBackButton==false)
        return;
    currentPane()->backButtonActivated();

}

void STStandardView::keyPressEvent(QKeyEvent *event){
    //qDebug()<<"key="<<event->key() << ", mod=" << event->modifiers();
    if(event->key()==Qt::Key_N){
        if(event->modifiers()&Qt::ControlModifier){
            m_tweetView->setFocus();
            event->accept();
            return;
        }
    }
    event->ignore();
}

void STStandardView::linkActivated(STObjectManager::EntityRange *er,
                                   STObjectManager::LinkActivationInfo i){
    if(currentAccountTabView())
        currentAccountTabView()->currentTabView()->linkActivate(er, i);
    else
        m_newAccountView->linkActivate(er, i);
}

STAccountTabView *STStandardView::currentAccountTabView(){
    if(m_currentAccountId && m_accountTabViews.contains(m_currentAccountId))
        return m_accountTabViews[m_currentAccountId];
    else
        return NULL;
}

QGraphicsObject *STStandardView::currentView(){
    return (QGraphicsObject *)currentAccountTabView()?:(QGraphicsObject *)m_newAccountView;
}

void STStandardView::setAuthView(STNewAccountAuthorizeView *av){
    if(m_authView==av)
        return;
    if(m_authView)
        disconnect(this, SLOT(authViewClosed()));
    m_authView=av;
    if(av){
        connect(av, SIGNAL(destroyed()),
                this, SLOT(authViewClosed()));
    }
    relayout();

}

void STStandardView::authViewClosed(){
    if(sender()==m_authView){
        setAuthView(NULL);
    }
}

void STStandardView::setCurrentAccount(quint64 acId){
    if(acId==m_currentAccountId)return;
    m_currentAccountId=acId;

    m_accountAnimator->activateViewAnimated(currentView(), 1, 200);
    m_newAccountOpacityAnimator->setValueAnimated(acId?1.f:0.f, 200);
    m_accountsView->setSelectedAccount(acId);
    m_addAccountButton->setChecked(acId==0);

    STHeaderView::Contents newContents=(currentPane()->headerViewContents());
    m_headerView->setContentsAnimated(newContents, STHeaderView::SlideToLeft);
    if(currentAccountTabView()){
        m_tabView->setCurrentTab(currentAccountTabView()->currentTab());
    }
    m_tweetView->setAccount(STAccountsManager::sharedManager()->account(acId));
}

void STStandardView::addAccountClicked(){
    setCurrentAccount(0);
}

STAccountTabView *STStandardView::createAccountTabView(quint64 acId){
    STAccount *ac=STAccountsManager::sharedManager()->account(acId);
    Q_ASSERT(ac!=NULL);
    if(m_accountTabViews.contains(ac->userId())){
        //oooooops??????
        return NULL;
    }
    STAccountTabView *view=new STAccountTabView(m_viewWrapper, ac->userId());
    // at least width should be set because
    // if width isn't set, timeline item's height is calculated
    // with 0px width, causing very tall items when it's exposed.
    view->setSize(QSize(m_newAccountView->size().width(), 10));
    view->setVisible(false);
    m_accountTabViews.insert(ac->userId(), view);

    connect(view, SIGNAL(headerViewContentsChanged(int)),
            this, SLOT(paneHeaderViewContentsChanged(int)));
}

void STStandardView::accountWasAdded(quint64 acId){
    createAccountTabView(acId);
}

void STStandardView::accountBeingRemoved(quint64 acId){

    // TODO: !!!: this account removal procedure is INVALID!!!
    //            remove active must first report removal to STStandardView,
    //            and then the account should be actually removed when
    //            the animation is done.
    Q_ASSERT(false);
  /*  if(acId==m_currentAccountId){
        // select alternative account
        STAccountList lst=STAccountsManager::sharedManager()->accounts();
        int i;
        for(int i=0;i<lst.count();i++){
            if(lst[i]->userId()==acId)
                break;
        }
        Q_ASSERT(i<lst.count());
        i++;
        if(i>=lst.count())i=lst.count()-2;
        if(i>=0){
            setCurrentAccount(lst[i]->userId());
        }else{
            setCurrentAccount(0);
        }
    }*/

}
