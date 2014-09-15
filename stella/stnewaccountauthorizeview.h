#ifndef STNEWACCOUNTAUTHORIZEVIEW_H
#define STNEWACCOUNTAUTHORIZEVIEW_H

#include <QGraphicsObject>
#include "stsimpleanimator.h"
#include <QGraphicsPixmapItem>
#include <QGraphicsProxyWidget>
#include "stobjectmanager.h"

class STAuthorizeCancelButton;
class STAuthorizeView;
class STMainView;
class KQOAuthManager;
class KQOAuthRequest;
class STStripeAnimationView;

class STNewAccountAuthorizeView : public QGraphicsObject
{
    Q_OBJECT
    QSize m_size;
    STSimpleAnimator *m_fadeAnimation;
    STSimpleAnimator *m_cancelAnimation;
    QGraphicsPixmapItem *m_window;
    QGraphicsProxyWidget *m_cancelButtonProxy;
    STAuthorizeCancelButton *m_cancelButton;
    STStripeAnimationView *m_animView;

    STAuthorizeView *m_authView;

    KQOAuthManager *m_manager;
    KQOAuthRequest *m_request;
    QString m_temporaryTokenString;
    QString m_accessToken;
    QString m_accessTokenSecret;

    STObjectManager::UserRef m_receivedUser;

    STMainView *mainView();
    void processAddedAccount();

public:
    explicit STNewAccountAuthorizeView(KQOAuthManager *manager, QString token, QGraphicsItem *parent = 0);
    
    void setSize(QSize);
    void relayout();

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual QRectF boundingRect() const;

    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);

    bool wasCancelled();

signals:
    
private slots:
    void fadeAnimated(float);
    void cancelled();
    void close();

    void authorizationReceived(QString,QString);
    void accessTokenReceived(QString, QString);
    void requestReadyEx(QByteArray, int);
};

#endif // STNEWACCOUNTAUTHORIZEVIEW_H
