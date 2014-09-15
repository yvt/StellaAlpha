#include "ststdafx.h"
#include "stnewaccountauthorizeview.h"
#include <QRadialGradient>
#include <QPainter>
#include "stmath.h"
#include <QPixmapCache>
#include "stfont.h"
#include "stauthorizecancelbutton.h"
#include "stauthorizeview.h"
#include "stmainview.h"
#include "ststripeanimationview.h"
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QtKOAuth>
#include "stobjectmanager.h"
#include "staccount.h"
#include "stjsonparser.h"
#include "staccountsmanager.h"
#include "ststandardview.h"

STNewAccountAuthorizeView::STNewAccountAuthorizeView(KQOAuthManager *manager, QString token,QGraphicsItem *parent) :
    QGraphicsObject(parent),m_size(0,0),m_manager(manager),m_temporaryTokenString(token)
{
    setCacheMode(DeviceCoordinateCache);
    //setFlag(ItemDoesntPropagateOpacityToChildren);
    setAcceptsHoverEvents(true);
    setFlag(ItemIsFocusable);


    QImage pix;
    pix.load(":/stella/res/AuthorizeWindow.png");

    // draw title bar and so on
    QString title=tr("Login");
    STFont *font=STFont::defaultBoldFont();

    font->drawString(pix, QPointF(14, 18-1), QColor(0,0,0,96), title);
    font->drawString(pix, QPointF(14, 18), QColor(230,230,230), title);

    m_window=new QGraphicsPixmapItem(QPixmap::fromImage(pix), this);
    m_window->setOffset(-pix.width()/2,-pix.height()/2);

    m_cancelButton=new STAuthorizeCancelButton();
    m_cancelButtonProxy=new QGraphicsProxyWidget(m_window);
    m_cancelButtonProxy->setWidget(m_cancelButton);
    m_cancelButtonProxy->setPos(QPointF(239,11)+m_window->offset());

    m_animView=new STStripeAnimationView(m_window);
    m_animView->setVisible(false);
    m_animView->setPos(10+m_window->offset().x(), 36+413-20+m_window->offset().y());
    m_animView->setSize(QSize(320, 20));

    connect(m_cancelButton, SIGNAL(clicked()),
            this, SLOT(cancelled()));
    //m_cancelButtonProxy->setCacheMode(DeviceCoordinateCache);

    m_authView=NULL;
    m_request=NULL;

    m_fadeAnimation=new STSimpleAnimator(this);
    m_cancelAnimation=new STSimpleAnimator(this);
    connect(m_fadeAnimation, SIGNAL(valueChanged(float)),
            this, SLOT(fadeAnimated(float)));
    connect(m_cancelAnimation, SIGNAL(valueChanged(float)),
            this, SLOT(fadeAnimated(float)));
    m_cancelAnimation->setValue(1.f);
    m_fadeAnimation->setValue(0);

    connect(m_manager, SIGNAL(authorizationReceived(QString,QString)),
            this, SLOT(authorizationReceived(QString,QString)));
    connect(m_manager, SIGNAL(accessTokenReceived(QString,QString)),
            this, SLOT(accessTokenReceived(QString,QString)));


}

STMainView *STNewAccountAuthorizeView::mainView(){
    QGraphicsView *vw=this->scene()->views()[0];
    STMainView *vw2=dynamic_cast<STMainView *>(vw);
    Q_ASSERT(vw2);
    return vw2;
}

void STNewAccountAuthorizeView::fadeAnimated(float v){
    setOpacity(m_fadeAnimation->value()*m_cancelAnimation->value());
    this->setFocus();
    relayout();

    if(m_cancelAnimation->value()==0.f){
        // canceled or done.
        processAddedAccount();
        this->deleteLater();
    }else if(!wasCancelled()){
        if(m_fadeAnimation->value()==1.f){
            if(!m_authView){
                // authoization start!!
                m_authView=new STAuthorizeView();

                QRectF rt(10,36,320,413);
                rt.translate(m_window->offset());
                rt.translate(m_window->pos());

                mainView()->startAuthorizeView(m_authView, (m_cancelButton->pos()+m_window->pos()).toPoint(),
                                               rt.toRect());
                connect(m_authView, SIGNAL(cancelled()),
                        this, SLOT(cancelled()));

                QUrl userAuthURL("https://api.twitter.com/oauth/authorize");
                QPair<QString, QString> tokenParam = qMakePair(QString("oauth_token"), m_temporaryTokenString);
                QUrl openWebPageUrl(userAuthURL.toString(), QUrl::StrictMode);
                openWebPageUrl.addQueryItem(tokenParam.first, tokenParam.second);
                openWebPageUrl.addQueryItem("force_login", "true");
                m_authView->setUrl(openWebPageUrl);
            }
        }
    }
}


void STNewAccountAuthorizeView::setSize(QSize sz){
    if(sz==m_size)return;
    m_size=sz;
    relayout();
}

void STNewAccountAuthorizeView::mousePressEvent(QGraphicsSceneMouseEvent *){

}

void STNewAccountAuthorizeView::relayout(){
    if(m_fadeAnimation->finalValue()<.5f){
        m_fadeAnimation->setValueAnimated(1.f, 500);
    }


    QPixmap pix=m_window->pixmap();
    QRect finalRect((m_size.width()-pix.width())/2,
                    (m_size.height()-pix.height())/2,
                    pix.width(),pix.height());
    float per=m_fadeAnimation->value();
    per=STDampedSpringStep(per, 3.f);

    float startPos=m_size.height();//m_size.height()+pix.height()/2;
    float endPos=finalRect.center().y();
    startPos+=(endPos-startPos)*.5f;

    m_window->setPos(finalRect.center().x(),
                     floorf(startPos*(1.f-per)+endPos*per));

}

void STNewAccountAuthorizeView::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *){
    QSizeF sz(m_size.width(),m_size.height());
    float radius=sqrtf(sz.width()*sz.width()+sz.height()*sz.height())*.5f;
    QRadialGradient grad(sz.width()*.5f, sz.height()*.5f, radius);
    grad.setColorAt(0.f, QColor(0,0,0,100));
    grad.setColorAt(1.f, QColor(0,0,0,140));
    painter->fillRect(boundingRect(), grad);

    QPixmap dither;
    static QPixmapCache::Key ditherKey;
    if(!QPixmapCache::find(ditherKey, &dither)){
        dither.load(":/stella/res/DitherNoise.png");
        ditherKey=QPixmapCache::insert(dither);
    }
    painter->drawTiledPixmap(boundingRect(), dither);
}

QRectF STNewAccountAuthorizeView::boundingRect() const{
    return QRectF(0,0,m_size.width(),m_size.height());
}
void STNewAccountAuthorizeView::close(){
    m_cancelAnimation->setValueAnimated(0, 200);
}

void STNewAccountAuthorizeView::cancelled(){
    if(m_authView){
        mainView()->stopAuthorizeView(m_authView);
        m_authView->deleteLater();;
        m_authView=NULL;
    }
    if(m_manager){
        m_manager->abortCurrentRequest();;
        m_manager->deleteLater();
        m_manager=NULL;
    }
    if(m_request){
        m_request->deleteLater();;
        m_request=NULL;
    }
    close();
}
bool STNewAccountAuthorizeView::wasCancelled(){
    return m_cancelAnimation->finalValue()<.2f;
}

void STNewAccountAuthorizeView::authorizationReceived(QString, QString){
    if(m_manager->lastError()!=KQOAuthManager::NoError){
        // error!
        // TODO: show error
        cancelled();
        return;
    }

    if(m_authView){
        mainView()->stopAuthorizeView(m_authView);
        m_authView->deleteLater();;
        m_authView=NULL;
    }

    m_animView->setVisible(true);

    m_manager->getUserAccessTokens(QUrl("https://api.twitter.com/oauth/access_token"));

}

void STNewAccountAuthorizeView::accessTokenReceived(QString token, QString tokenSecret){
    m_accessToken=token;
    m_accessTokenSecret=tokenSecret;

    if(m_manager->lastError()!=KQOAuthManager::NoError){
        // error!
        // TODO: show error
        cancelled();
        return;
    }

    // start authenticated user query.
    m_manager->abortCurrentRequest();;
    m_manager->deleteLater();;
    if(m_request){
        m_request->deleteLater();
    }
    m_manager=new KQOAuthManager(this);
    m_request=new KQOAuthRequest(this);

    connect(m_manager, SIGNAL(requestReadyEx(QByteArray,int)),
            this, SLOT(requestReadyEx(QByteArray,int)));

    m_request->initRequest(KQOAuthRequest::AuthorizedRequest, QUrl("https://api.twitter.com/1.1/account/verify_credentials.json"));
    m_request->setConsumerKey(STAccountsManager::sharedManager()->consumerKey());
    m_request->setConsumerSecretKey(STAccountsManager::sharedManager()->consumerSecret());
    m_request->setToken(token);
    m_request->setTokenSecret(tokenSecret);
    m_request->setHttpMethod(KQOAuthRequest::GET);

    m_manager->executeRequest(m_request);
}

void STNewAccountAuthorizeView::requestReadyEx(QByteArray data, int code){
    if(code){
        // error!
        // TODO: show error
        cancelled();
        return;
    }

    if(m_request){
        // must be authenticated user query...

        QVariant var=STJsonParser().parse(data);
        if(var.isNull()){
            // error!
            // TODO: show error
            cancelled();
            return;
        }

        STObjectManager::UserRef user=STObjectManager::sharedManager()->user(var);
        if(!user){
            // error!
            // TODO: show error
            cancelled();
            return;
        }

        m_receivedUser=user;
        cancelled(); // close (maybe incorrect name, but cancelled() does cleanup)


    }
}


void STNewAccountAuthorizeView::processAddedAccount(){
    if(!m_receivedUser)return;
    STObjectManager::UserRef user=m_receivedUser;

    // already exists?
    STAccount *account=STAccountsManager::sharedManager()->account(user->id);

    if(account){
        // update settings.
        account->setAccessToken(m_accessToken);
        account->setAccessTokenSecret(m_accessTokenSecret);
        account->setUser(user);
    }else{
        // create account.
        account=new STAccount(user->id, m_accessToken, m_accessTokenSecret,
                              STAccountsManager::sharedManager());
        account->setUser(user);
        STAccountsManager::sharedManager()->addAccount(account);
    }

    // select authorized account.
    QGraphicsItem *item=this;
    STStandardView *vw=NULL;
    while(!vw){
        item=item->parentItem();
        vw=dynamic_cast<STStandardView *>(item);
    }
    vw->setCurrentAccount(account->userId());
}
