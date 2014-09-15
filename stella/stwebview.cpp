#include "stwebview.h"
#include "stwebview_p.h"
#include "stmainview.h"
#include <QDebug>

STWebView::STWebView(QWidget *parent) :
    QWidget(parent),m_size(0,0),d_ptr(new STWebViewPrivate(this))
{
}

STWebView::~STWebView(){
    delete d_ptr;
}

void STWebView::setUrl(const QUrl &url){
    Q_D(STWebView);
    d->setUrl(url);
}

void STWebView::resizeEvent(QResizeEvent *){
    Q_D(STWebView);
    d->resized();
}

#if STWEBVIEW_QTWEBKIT

/* QGraphicsWebView port */

#include <QVBoxLayout>

STWebViewPrivate::STWebViewPrivate(STWebView *q):
    QObject(q), q_ptr(q){
    m_webFrame=new QWebView();

    QVBoxLayout *lay=new QVBoxLayout(q);
    lay->setMargin(0);
    lay->addWidget(m_webFrame);
    q->setLayout(lay);
    m_webFrame->setMouseTracking(true);

    m_webFrame->show();
}


void STWebViewPrivate::setUrl(QUrl u){
    m_webFrame->setUrl(u);
}

void STWebViewPrivate::resized(){

}

#elif STWEBVIEW_MAC_WEBVIEW

#include<QGraphicsView>
#include <QGraphicsScene>
#include <math.h>
#include "stmacwebview.h"
#include <QVBoxLayout>

/* Mac WebView port */

STWebViewPrivate::STWebViewPrivate(STWebView *q):
    QObject(q), m_webFrame(NULL),m_container(NULL),q_ptr(q){
    realizeView();
}

STWebViewPrivate::~STWebViewPrivate(){
    //Q_Q(STWebView);
    if(m_container){
        delete m_container;
    }
}

void STWebViewPrivate::realizeView(){
    Q_Q(STWebView);
    if(m_webFrame)return;

    m_webFrame=STMacWebViewCreate();
    m_container=new QMacCocoaViewContainer(NULL,q);
    m_container->setWindowFlags(m_container->windowFlags()|Qt::WindowStaysOnTopHint);
    m_container->setAttribute(Qt::WA_NativeWindow);
    m_container->setCocoaView(m_webFrame);
    STMacWebViewRelease(m_webFrame);
    m_container->setMouseTracking(true);

    QVBoxLayout *lay=new QVBoxLayout(q);
    lay->setMargin(0);
    lay->addWidget(m_container);
    q->setLayout(lay);

    m_container->show();

    if(!m_reqUrl.isEmpty())
        setUrl(m_reqUrl);

}

void STWebViewPrivate::setUrl(QUrl u){

    realizeView();
    if(!m_webFrame){
        m_reqUrl=u;
        return;
    }

    QString str=u.toString();
    QByteArray utf=str.toUtf8();
    STMacWebViewSetUrl(m_webFrame, utf.constData());

}


void STWebViewPrivate::resized(){

}

#elif STWEBVIEW_IE_COMPONENT


#include <QGraphicsView>
#include <QGraphicsScene>
#include <math.h>
#include <QVBoxLayout>
#include <windows.h>

/* IWebBrowser2 port */

STWebViewPrivate::STWebViewPrivate(STWebView *q):
    QObject(q), m_webFrame(NULL),q_ptr(q){
    realizeView();
}

STWebViewPrivate::~STWebViewPrivate(){
    //Q_Q(STWebView);
    if(m_webFrame){
        delete m_webFrame;
    }
}

void STWebViewPrivate::realizeView(){
    Q_Q(STWebView);
    if(m_webFrame)return;

    // disable compatibility mode
    HKEY key;
    RegCreateKeyA(HKEY_CURRENT_USER,
                  "SOFTWARE\\Microsoft\\Internet Explorer\\Main\\FeatureControl\\FEATURE_BROWSER_EMULATION",
                  &key);
    DWORD vl=12000;
    RegSetValueExA(key, "Stella.exe",
                   0, REG_DWORD, (const BYTE *)&vl, sizeof(DWORD));

    m_webFrame=new QAxWidget("8856F961-340A-11D0-A96B-00C04FD705A2",q);
     m_webFrame->setMouseTracking(true);

    QVBoxLayout *lay=new QVBoxLayout(q);
    lay->setMargin(0);
    lay->addWidget(m_webFrame);
    q->setLayout(lay);

    m_webFrame->show();

    if(!m_reqUrl.isEmpty())
        setUrl(m_reqUrl);

}

void STWebViewPrivate::setUrl(QUrl u){

    realizeView();
    if(!m_webFrame){
        m_reqUrl=u;
        return;
    }

    QString str=u.toString();
    m_webFrame->dynamicCall("Navigate(const QString&,QVariant&,QVariant&,QVariant&,QVariant&)",
                            str);

}


void STWebViewPrivate::resized(){

}

#endif
