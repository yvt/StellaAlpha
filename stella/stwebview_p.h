#ifndef STWEBVIEW_P_H
#define STWEBVIEW_P_H

#include <QObject>
#include "QtConfig"

#define STWEBVIEW_QTWEBKIT       1

#if QT_MAC_USE_COCOA
#undef STWEBVIEW_QTWEBKIT
#define STWEBVIEW_MAC_WEBVIEW       1
#elif ST_USE_ACTIVEQT
#undef STWEBVIEW_QTWEBKIT
#define STWEBVIEW_IE_COMPONENT  1
#endif

#include "ststdafx.h"
#include "stwebview.h"
#include <QUrl>


#if STWEBVIEW_QTWEBKIT

#include <QtWebKit>

class STWebViewPrivate:public QObject{
    Q_OBJECT
    QWebView *m_webFrame;
public:
    STWebViewPrivate(STWebView *);

    void setUrl(QUrl);
    void resized();
protected:
    STWebView *q_ptr;
private:
    Q_DECLARE_PUBLIC(STWebView)
};

#elif STWEBVIEW_MAC_WEBVIEW

#include <QMacCocoaViewContainer>

class STWebViewPrivate:public QObject{
    Q_OBJECT
    void *m_webFrame;
    QMacCocoaViewContainer *m_container;
    QUrl m_reqUrl;
public:
    STWebViewPrivate(STWebView *);
    virtual ~STWebViewPrivate();

    void setUrl(QUrl);

    void realizeView();
    void resized();
protected:
    STWebView *q_ptr;
private:
    Q_DECLARE_PUBLIC(STWebView)
};


#elif STWEBVIEW_IE_COMPONENT

#include <QAxWidget>

class STWebViewPrivate:public QObject{
    Q_OBJECT
    QAxWidget *m_webFrame;
    QUrl m_reqUrl;
public:
    STWebViewPrivate(STWebView *);
    virtual ~STWebViewPrivate();

    void setUrl(QUrl);

    void realizeView();
    void resized();
protected:
    STWebView *q_ptr;
private:
    Q_DECLARE_PUBLIC(STWebView)
};

#endif

#endif // STWEBVIEW_P_H
