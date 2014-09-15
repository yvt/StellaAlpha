#ifndef STACCOUNT_H
#define STACCOUNT_H

#include "ststdafx.h"
#include <QObject>
#include "stobjectmanager.h"

class STUserStreamer;
class STFilteredStreamer;
class STHomeTimeline;
class STMentionsTimeline;

class STAccount : public QObject
{
    Q_OBJECT

    quint64 m_userId;
    QString m_accessToken;
    QString m_accessTokenSecret;

    STUserStreamer *m_userStreamer;
    STFilteredStreamer *m_filteredStreamer;

    STHomeTimeline *m_homeTimeline;
    STMentionsTimeline *m_mentionsTimeline;

    STObjectManager::UserRef m_user;

    QString m_profileImageUrl;  // to trace the change
    QString m_screenName;       // to trace the change

public:
    explicit STAccount(quint64 userId, QString accessToken,
                       QString accessTokenSecret, QObject *parent = 0);
    static STAccount *accountWithInfo(QVariant);
    virtual ~STAccount();

    STHomeTimeline *homeTimeline();
    STMentionsTimeline *mentionsTimeline();
    
    quint64 userId() const{return m_userId;}
    QString accessToken() const{return m_accessToken;}
    QString accessTokenSecret() const{return m_accessTokenSecret;}
    QString consumerKey();
    QString consumerSecret();

    void setAccessToken(QString s){m_accessToken=s;}
    void setAccessTokenSecret(QString s){m_accessTokenSecret=s;}

    void setUser(STObjectManager::UserRef us);

    STUserStreamer *userStreamer(){return m_userStreamer;}
    STFilteredStreamer *filteredStreamer() {return m_filteredStreamer;}

    STObjectManager::UserRef userObject() {return m_user;}

    void saveUserInfo();
    QVariant toSTONVariant();

    void saveTimelines();

    QString accountPath();
    QString accountInfoPath();
    QString homeTimelinePath();
    QString mentionsTimelinePath();

signals:
    
    void errorReported(const QString&, STAccount *account);

    void iconChanged();
    void screenNameChanged();

public slots:
    
    void reportError(const QString&);
};

#endif // STACCOUNT_H
