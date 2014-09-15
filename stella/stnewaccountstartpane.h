#ifndef STNEWACCOUNTSTARTPANE_H
#define STNEWACCOUNTSTARTPANE_H

#include "ststdafx.h"
#include "stpane.h"

class STButtonView;
class STLinkedLabelView;
class STStripeAnimationView;

class KQOAuthManager;
class KQOAuthRequest;

class STNewAccountStartPane : public STPane
{
    Q_OBJECT
    STLinkedLabelView *m_titleLabel;
    STLinkedLabelView *m_textLabel;
    STButtonView *m_loginButton;
    int m_oldWidth;

    STStripeAnimationView *m_animView;

    KQOAuthManager *m_manager;
    KQOAuthRequest *m_request;
public:

    explicit STNewAccountStartPane(QGraphicsItem *parent = 0);
    
    virtual void relayoutContents();

    virtual STHeaderView::Contents headerViewContents();
signals:
    
private slots:
    void temporaryTokenReceived(QString, QString);
    void requestDone(QByteArray, int code);
public slots:
    void startAuthorization();
};

#endif // STNEWACCOUNTSTARTPANE_H
