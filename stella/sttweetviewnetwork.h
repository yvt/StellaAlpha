#ifndef STTWEETVIEWNETWORK_H
#define STTWEETVIEWNETWORK_H

#include <QObject>
#include "stobjectmanager.h"

class STAccount;
class KQOAuthManager;
class KQOAuthRequest;

class STTweetViewNetwork : public QObject
{
    Q_OBJECT
    KQOAuthManager *m_manager;
    KQOAuthRequest *m_request;
public:
    explicit STTweetViewNetwork(QObject *parent = 0);
    
    bool isSending();
    void sendTweet(const QString& text,
                   STAccount *account,
                   STObjectManager::StatusRef inReplyTo);


signals:
    void tweetSent();
    void tweetErrorSending(const QString& desc);
public slots:
    void endRequest(const QByteArray&, int code);
};

#endif // STTWEETVIEWNETWORK_H
