#include "stnewaccountstartpane.h"
#include "stbuttonview.h"
#include "stlinkedlabelview.h"
#include "stnewaccountauthorizeview.h"
#include "ststandardview.h"
#include <QtKOAuth>
#include "staccountsmanager.h"
#include "ststripeanimationview.h"
#include <QDebug>
#include "stnetworkreplyerrorparser.h"

STNewAccountStartPane::STNewAccountStartPane(QGraphicsItem *parent) :
    STPane(parent)
{
    m_titleLabel=new STLinkedLabelView(this);
    m_titleLabel->setBgColor(QColor(70,70,70));

    m_textLabel=new STLinkedLabelView(this);
    m_textLabel->setBgColor(QColor(70,70,70));

    m_loginButton=new STButtonView(this);
    m_loginButton->setText(tr("Sign in with Twitter"));
    connect(m_loginButton, SIGNAL(activated()),
            this, SLOT(startAuthorization()));

    m_animView=new STStripeAnimationView(this);
    m_animView->setVisible(false);

    m_oldWidth=0;

    m_manager=NULL;
    m_request=NULL;

}

void STNewAccountStartPane::relayoutContents(){
    int cy=20;
    int width=contentsWidth();
    if(width<=0)return;
    if(m_oldWidth!=width){
        m_oldWidth=width;

        float maxWidth=width;

        m_titleLabel->setContents(STFont::defaultBoldFont(),
                                  tr("Account Login"), maxWidth-40.f);

        m_textLabel->setContents(STFont::defaultFont(),
                                 tr("Twitter requires you to authorize Stella Alpha to access your account. You'll be taken to Twitter's authorization page."), maxWidth-40.f);

    }

    float titleWidth=m_titleLabel->boundingRect().right();
    m_titleLabel->setPos(((float)width-titleWidth)*.5f, cy);
    cy+=m_titleLabel->boundingRect().bottom();
    cy+=5;
    m_textLabel->setPos(20, cy);
    cy+=m_textLabel->boundingRect().bottom();
    cy+=13;

    m_loginButton->setPos(20, cy);
    m_loginButton->setSize(QSize(contentsWidth()-40, 25));
    m_animView->setPos(21,cy+1);
    m_animView->setSize(m_loginButton->size()-QSize(2,2));
}

STHeaderView::Contents STNewAccountStartPane::headerViewContents(){
    STHeaderView::Contents cont=STPane::headerViewContents();
    cont.title=(tr("New Account"));
    return cont;
}

void STNewAccountStartPane::startAuthorization(){
    if(!isPaneActive())return;/*
    STNewAccountAuthorizePane *p=new STNewAccountAuthorizePane();
    pushPaneAnimated(p);*/

    if(m_manager){
        return;
    }

    m_animView->setVisible(true);

    m_manager=new KQOAuthManager(this);
    m_request=new KQOAuthRequest(this);
    connect(m_manager, SIGNAL(temporaryTokenReceived(QString,QString)),
            this, SLOT(temporaryTokenReceived(QString,QString)));
    connect(m_manager, SIGNAL(requestReadyEx(QByteArray,int)),
            this, SLOT(requestDone(QByteArray,int)));
    m_request->initRequest(KQOAuthRequest::TemporaryCredentials,
                           QUrl("https://api.twitter.com/oauth/request_token"));
    m_request->setConsumerKey(STAccountsManager::sharedManager()->consumerKey());
    m_request->setConsumerSecretKey(STAccountsManager::sharedManager()->consumerSecret());

    m_manager->setHandleUserAuthorization(true);
    m_manager->executeRequest(m_request);

    if(m_manager->lastError()!=KQOAuthManager::NoError){
        m_manager->abortCurrentRequest();
        m_manager->deleteLater();;
        m_request->deleteLater();;
        m_animView->setVisible(false);
        qWarning()<<"STNewAccountStartPane::startAuthorization: failed to initate authorization.";
    }

}

void STNewAccountStartPane::requestDone(QByteArray, int code){
    // this is used to receive errors.
    if(!code)return;

    // TODO :show error!
    qWarning()<<"STNewAccountStartPane::requestDone: error: "<<STNetworkReplyErrorParser::parseError(code);

    m_request->deleteLater();
    m_animView->setVisible(false);
    m_manager->deleteLater();
    m_manager=NULL;
    m_loginButton->update();
}


void STNewAccountStartPane::temporaryTokenReceived(QString token, QString){
    m_animView->setVisible(false);
    m_request->deleteLater();

    Q_ASSERT(m_manager==sender());

    if(m_manager->lastError()==KQOAuthManager::NoError){

        QGraphicsItem *par=this;
        STStandardView *v=NULL;
        while(!v){
            par=par->parentItem();
            v=dynamic_cast<STStandardView *>(par);
        }



        if(v->authView()){
            m_manager->deleteLater();
            m_manager=NULL;
            return; // already visible. why??
        }

        STNewAccountAuthorizeView *p=new STNewAccountAuthorizeView(m_manager, token, v);
        p->setZValue(10000.f);
        v->setAuthView(p);
        m_manager->setParent(p);

        m_manager=NULL;

    }else{
        QString reason;
        switch(m_manager->lastError()){
        case KQOAuthManager::NetworkError:
            reason="Network error.";
            break;
        case KQOAuthManager::RequestEndpointError:
            reason="Invalid endpoint.";
            break;
        case KQOAuthManager::RequestValidationError:
            reason="Invalid request.";
            break;
        case KQOAuthManager::RequestUnauthorized:
            reason="Authorization error.";
            break;
        case KQOAuthManager::RequestError:
            reason="Request error.";
            break;
        case KQOAuthManager::ManagerError:
            reason="Internal error.";
            break;
        case KQOAuthManager::NoError:
            break;
        }

        // TODO :show error!

        m_manager->deleteLater();
        m_manager=NULL;
    }

}
