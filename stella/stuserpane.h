#ifndef STUSERPANE_H
#define STUSERPANE_H

#include "stscrolledpane.h"
#include "stobjectmanager.h"
#include <QGraphicsPixmapItem>
#include <QVariantMap>
#include "stviewanimator.h"
#include <QAction>

class STClearButtonView;
class STUserIconView;
class STStripeAnimationView;
class KQOAuthManager;
class KQOAuthRequest;
class STAccount;
class STTabView;
class STUserProfileView;
class STTimelinePane;
class STSimpleAnimator;

class STUserPane : public STPane
{
    Q_OBJECT

    STUserIconView*m_iconView;
    quint64 m_userId;
    STAccount *m_account;
    int m_lastTab;
    qint64 m_requestedTime;

    STStripeAnimationView *m_animView;
    QGraphicsPixmapItem *m_statusView;
    QGraphicsPixmapItem *m_nameView;

    QGraphicsPixmapItem *m_protectedIcon;
    QGraphicsPixmapItem *m_verifiedIcon;
    QGraphicsPixmapItem *m_translatorIcon;
    STClearButtonView *m_optionsButton;


    STTabView *m_tabView;

    STViewAnimator m_animator;
    STUserProfileView *m_profileTab;
    STTimelinePane *m_tweetsTab;
    STTimelinePane *m_mentionsTab;
    STTimelinePane *m_favoritesTab;

    STObjectManager::UserRef m_user;
    QVariantMap m_relationship;
    QString m_errorMessage;

    KQOAuthManager *m_manager;
    KQOAuthRequest *m_request;

    QAction *m_copyNameAction;
    QAction *m_copyScreenNameAction;

    STSimpleAnimator *m_displayAnimator;
    STSimpleAnimator *m_animViewAnimator;

    void updateStatus();
    void updateNameView();
    void updateIcons();

    QUrl largeAvatarUrl();

public:
    explicit STUserPane(quint64 userId, STAccount *acc, QGraphicsItem *parent = 0);

    virtual STHeaderView::Contents headerViewContents();
    virtual void relayoutContents();

    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);

    bool isRequesting();
    void startRequest();

    STAccount *account() const{return m_account;}
signals:
    
private slots:
    void requestReady(const QByteArray&, int code);
    void tabChanged();

    void copyName();
    void copyScreenName();

    void animated(float);
    void setAnimViewOpacity(float);

    void showTweets();
    void showFavorites();

    void showLargeAvatar();

    void showOptions();
public slots:
    void setUser(STObjectManager::UserRef, bool animated=false);
    void setRelationship(QVariantMap);
    void showError(QString);
};

#endif // STUSERPANE_H
