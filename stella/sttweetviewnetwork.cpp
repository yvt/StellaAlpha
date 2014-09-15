#include "sttweetviewnetwork.h"
#include <QtKOAuth>
#include "staccount.h"
#include "stjsonparser.h"
#include <QDebug>
#include <QVariantMap>
#include "stnetworkreplyerrorparser.h"

STTweetViewNetwork::STTweetViewNetwork(QObject *parent) :
    QObject(parent)
{
    m_manager=new KQOAuthManager(this);
    m_request=NULL;

    connect(m_manager, SIGNAL(requestReadyEx(QByteArray,int)),
            this, SLOT(endRequest(QByteArray,int)));
}

bool STTweetViewNetwork::isSending(){
    return m_request;
}

void STTweetViewNetwork::sendTweet(const QString &text, STAccount *account, STObjectManager::StatusRef inReplyTo){
    Q_ASSERT(!isSending());

    KQOAuthParameters param;
    param.insert("status", text);
    if(inReplyTo)
        param.insert("in_reply_to_status_id", QString::number(inReplyTo->id));


    QUrl url("https://api.twitter.com/1.1/statuses/update.json");

    m_request=new KQOAuthRequest(this);
    m_request->initRequest(KQOAuthRequest::AuthorizedRequest, url);
    m_request->setAdditionalParameters(param);
    m_request->setHttpMethod(KQOAuthRequest::POST);
    m_request->setConsumerKey(account->consumerKey());
    m_request->setConsumerSecretKey(account->consumerSecret());
    m_request->setToken(account->accessToken());
    m_request->setTokenSecret(account->accessTokenSecret());

    m_manager->executeRequest(m_request);

    Q_ASSERT(isSending());
}

void STTweetViewNetwork::endRequest(const QByteArray &res, int code){
    m_request->deleteLater();
    m_request=NULL;
    QVariant var=STJsonParser().parse(res);

    Q_ASSERT(!isSending());



    if(var.isNull()){

        if(code){
            qWarning()<<"STTweetViewNetwork::endRequest: code="<<code;
            emit tweetErrorSending(STNetworkReplyErrorParser::parseError(code));
            return;
        }

        qWarning()<<"STTweetViewNetwork::endRequest: returned object is null or unparsable.";
        emit tweetErrorSending(tr("Invalid response"));
        return;
    }
    if(!var.canConvert(QVariant::Map)){
        qWarning()<<"STTweetViewNetwork::endRequest: returned object is not a dictionary.";
        emit tweetErrorSending(tr("Invalid response"));
        return;
    }

    if(STObjectManager::sharedManager()->status(var, false)){
        emit tweetSent();
        return;
    }

    QVariantMap map=var.toMap();

    if(map.contains("errors")){
        QVariantList lst=map["errors"].toList();
        if(!lst.isEmpty()){
            map=lst[0].toMap();
            if(map.contains("message")){
                qWarning()<<"STTweetViewNetwork::endRequest: errors[0].message = "<<map["message"];
                if(map.contains("code")){
                    qWarning()<<"STTweetViewNetwork::endRequest: errors[0].code = "<<map["code"];
                    int code=map["code"].toInt();
                    if(code==170){
                        // tweet empty
                        emit tweetErrorSending(tr("Tweet is empty."));
                        return;
                    }else if(code==187){
                        // duplicate
                        emit tweetErrorSending(tr("Tweet is a duplicate."));
                        return;
                    }else if(code==88){
                        // rate limit
                        emit tweetErrorSending(tr("Rate limit exceeded."));
                        return;
                    }else if(code==135){
                        // unauthorized
                        emit tweetErrorSending(tr("Login failed."));
                        return;
                    }
                }

                emit tweetErrorSending(map["message"].toString());
                return;
            }
        }
    }

    qWarning()<<"STTweetViewNetwork::endRequest: unknown error: "<<var;
    emit tweetErrorSending(tr("Invalid response"));
    return;

}
