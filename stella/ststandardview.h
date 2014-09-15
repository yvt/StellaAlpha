#ifndef SHSTANDARDVIEW_H
#define SHSTANDARDVIEW_H

#include "ststdafx.h"
#include <QGraphicsObject>
#include <QGraphicsPixmapItem>
#include <QTimer>
#include <QPropertyAnimation>
#include "stobjectmanager.h"
#include <QGraphicsRectItem>
#include "stviewanimator.h"
#include "stsimpleanimator.h"

class STHeaderView;
class STNewsView;
class STTabView;
class STTweetView;
class STAccountTabView;
class STPane;
class STAccountsView;
class STResizeView;
class STNewAccountView;
class STNewAccountAuthorizeView;
class STButtonView;
class STLogoView;

class STStandardView : public QGraphicsObject
{
    Q_OBJECT

    QSize m_size;

    STAccountsView *m_accountsView;

    STHeaderView *m_headerView;
    STButtonView *m_addAccountButton;
    STNewsView *m_newsView;
    STTabView *m_tabView;
    STTweetView *m_tweetView;
    QMap<quint64, STAccountTabView *> m_accountTabViews;
    STNewAccountView *m_newAccountView;
    STLogoView *m_logoView;
    QGraphicsRectItem *m_viewWrapper;

    STViewAnimator *m_accountAnimator;
    STSimpleAnimator *m_newAccountOpacityAnimator; // needed to hide tweet/tab view without current account
    STSimpleAnimator *m_startupAnimator; // fades sidebar button and news view

    STResizeView *m_resizeView;

    QGraphicsRectItem *m_topBorder;
    QGraphicsRectItem *m_bottomBorder;
    QGraphicsRectItem *m_rightBorder;
    QGraphicsRectItem *m_leftBorder;

    quint64 m_currentAccountId;

    STNewAccountAuthorizeView *m_authView;

    STAccountTabView *createAccountTabView(quint64);

public:
    explicit STStandardView(QGraphicsItem *parent = 0);
    
    void setSize(const QSize& size){
        this->prepareGeometryChange();
        m_size=size;relayout();
    }

    virtual void keyPressEvent(QKeyEvent *event);

    virtual QRectF boundingRect() const;
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    STAccountTabView *currentAccountTabView();
    STPane *currentPane();

    QGraphicsObject *currentView(); // acount tab view, or new account view

    STTweetView *tweetView() const{return m_tweetView;}

    void setAuthView(STNewAccountAuthorizeView *);
    STNewAccountAuthorizeView *authView() const{return m_authView;}

    void initializeInitialView();  // called by STMainView while startup
    void startOperation(); // called by STMainView when startup is ready

signals:
    
public slots:
    void relayout();
    void currentTabChanged();
    void paneHeaderViewContentsChanged(int opening);
    void backButtonActivated();
    void linkActivated(STObjectManager::EntityRange *,
                       STObjectManager::LinkActivationInfo);

    void setCurrentAccount(quint64);
private slots:
    void setTweetTabViewOpacity(float);
    void startupAnimation(float);
    void authViewClosed();


    void accountWasAdded(quint64);
    void accountBeingRemoved(quint64);

    void addAccountClicked();
};

#endif // SHSTANDARDVIEW_H
