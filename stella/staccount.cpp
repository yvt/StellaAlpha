#include "ststdafx.h"
#include "staccount.h"

#include "stuserstreamer.h"
#include "stfilteredstreamer.h"
#include "sthometimeline.h"
#include "stmentionstimeline.h"
#include "staccountsmanager.h"
#include "stobjectnotation.h"
#include <QFile>
#include <QDebug>
#include <QDir>
#include <QThread>
#include <QApplication>

STAccount::STAccount(quint64 userId, QString accessToken,
                     QString accessTokenSecret, QObject *parent) :
    QObject(parent), m_userId(userId),
    m_accessToken(accessToken), m_accessTokenSecret(accessTokenSecret)
{
    Q_ASSERT(thread()==QApplication::instance()->thread());
    m_userStreamer=new STUserStreamer(this);
    m_userStreamer->startStreaming();

    m_filteredStreamer=new STFilteredStreamer(this);

    m_homeTimeline=NULL;
    m_mentionsTimeline=NULL;

}

QString STAccount::accountPath(){
    return STAccountsManager::accountPathFor(m_userId);
}

QString STAccount::accountInfoPath(){
    return accountPath()+"/Info.ston";
}

QString STAccount::homeTimelinePath(){
    return accountPath()+"/HomeTimeline.ston";
}

QString STAccount::mentionsTimelinePath(){
    return accountPath()+"/MentionsTimeline.ston";
}

STHomeTimeline *STAccount::homeTimeline(){
    if(!m_homeTimeline){
        Q_ASSERT(thread()==QThread::currentThread());
        m_homeTimeline=new STHomeTimeline(QString("%1.home").arg(m_userId), this);

        QVariant var=STObjectNotation::parse(homeTimelinePath());
        if(!var.isNull())m_homeTimeline->loadFromSTONVariant(var);
    }
    return m_homeTimeline;
}

STMentionsTimeline *STAccount::mentionsTimeline(){

    if(!m_mentionsTimeline){
        Q_ASSERT(thread()==QThread::currentThread());
        m_mentionsTimeline=new STMentionsTimeline(QString("%1.mentions").arg(m_userId), this);

        QVariant var=STObjectNotation::parse(mentionsTimelinePath());
        if(!var.isNull())m_mentionsTimeline->loadFromSTONVariant(var);
    }
    return m_mentionsTimeline;
}

STAccount::~STAccount(){
    if(m_homeTimeline)
    delete m_homeTimeline;
    if(m_mentionsTimeline)
    delete m_mentionsTimeline;

    m_userStreamer->blockSignals(true);
    m_filteredStreamer->blockSignals(true);
    m_userStreamer->stopStreaming();
    m_filteredStreamer->stopStreaming();
    m_userStreamer->metaObject()->invokeMethod(m_userStreamer, "deleteLater", Qt::BlockingQueuedConnection);
    m_filteredStreamer->metaObject()->invokeMethod(m_filteredStreamer, "deleteLater", Qt::BlockingQueuedConnection);
}


void STAccount::reportError(const QString &msg){
    emit errorReported(msg, this);
}

QString STAccount::consumerKey(){
    return STAccountsManager::sharedManager()->consumerKey();
}

QString STAccount::consumerSecret(){
    return STAccountsManager::sharedManager()->consumerSecret();
}

void STAccount::setUser(STObjectManager::UserRef us){
    m_user=us;

    QString newProfileImageUrl=m_user->data["profile_image_url"].toString();
    QString newScreenName=m_user->data["screen_name"].toString();
    if(newProfileImageUrl!=m_profileImageUrl && !m_profileImageUrl.isEmpty())
        emit iconChanged();
    if(newScreenName!=m_screenName && !m_screenName.isEmpty())
        emit screenNameChanged();
    m_profileImageUrl=newProfileImageUrl;
    m_screenName=newScreenName;
}

QVariant STAccount::toSTONVariant(){
    QVariantMap map;
    map.insert("access_token_secret", m_accessTokenSecret);
    map.insert("access_token", m_accessToken);
    map.insert("user_id", m_userId);
    map.insert("user_object", m_user->toRefVariant());
    return map;
}

void STAccount::saveUserInfo(){
    QDir().mkpath(STAccountsManager::accountPathFor(m_userId));

    QVariant var=toSTONVariant();
    QString path=STAccountsManager::accountPathFor(m_userId)+"/Info.ston";
    if(!STObjectNotation::serialize(var, path)){
        qWarning()<<"STAccount::saveUserInfo: failed to open "<<path;
    }
}

STAccount *STAccount::accountWithInfo(QVariant info){
    QVariantMap map=info.toMap();
    if(!map.contains("access_token_secret")) return NULL;
    if(!map.contains("access_token")) return NULL;
    if(!map.contains("user_id")) return NULL;
    if(!map.contains("user_object")) return NULL;

    STObjectManager::UserRef user=map["user_object"].value<STObjectManager::UserRef>();
    if(!user)return NULL;

    STAccount *ac=new STAccount(map["user_id"].toULongLong(),
                                map["access_token"].toString(),
                                map["access_token_secret"].toString());
    ac->setUser(user);

    return ac;
}

void STAccount::saveTimelines(){
    if(m_homeTimeline)
    if(!STObjectNotation::serialize(m_homeTimeline->toSTONVariant(), homeTimelinePath()))
        qWarning()<<"STAccount::saveTimelines failed to write "<<homeTimelinePath();
    if(m_mentionsTimeline)
    if(!STObjectNotation::serialize(m_mentionsTimeline->toSTONVariant(), mentionsTimelinePath()))
        qWarning()<<"STAccount::saveTimelines failed to write "<<mentionsTimelinePath();


}
