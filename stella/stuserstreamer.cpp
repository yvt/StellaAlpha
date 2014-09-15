#include "stuserstreamer.h"
#include <QtKOAuth>
#include "staccount.h"
#include <QDebug>
#include <QVariantMap>
#include <QVariantList>
#include "stjsonparser.h"

STUserStreamer::STUserStreamer(STAccount *account) :
    STStreamer(account, QUrl("https://userstream.twitter.com/1.1/user.json?stall_warnings=true&include_my_retweet=1"))
{

}

bool STUserStreamer::following(quint64 uid){
    return m_friends.contains(uid);
}

bool STUserStreamer::isStatusOfFollowingUser(const STObjectManager::StatusRef &s){
    // warning: user stream doesn't return s->following.
    // it becomes always false (null).
    return following(s->user->id);
}

bool STUserStreamer::isMention(const STObjectManager::StatusRef &s){
    STObjectManager::StatusRef stat=s;
    QMutexLocker lock(&stat->mutex);
    if(stat->retweetedStatus)
        stat=stat->retweetedStatus;
    QVariant entitiesVar=stat->data["entities"];
    if(entitiesVar.isNull())
        return false;
    QVariantMap entities=entitiesVar.toMap();

    QVariant userMentionsVar=entities["user_mentions"];
    if(userMentionsVar.isNull())
        return false;
    QVariantList userMentions=userMentionsVar.toList();

    foreach(const QVariant& userMentionVar, userMentions){
        QVariantMap userMention=userMentionVar.toMap();
        quint64 targetId=userMention["id"].toULongLong();
        if(targetId==account()->userId())
            return true;
    }

    return false;
}

void STUserStreamer::lineReceived(const QByteArray &data){
    STJsonParser parser;
   // qDebug()<<data;
    if(data.isEmpty())
        return;
    if(data.at(0)!='{') // not array nor hash
        return;

    //if(qrand()&15)return;
    QVariant var=parser.parse(data);
    if(var.isNull())
        return;

//qDebug()<<data;

    STObjectManager::StatusRef ref=STObjectManager::sharedManager()->status(var, false, account()->userId());
    if(ref){
        emit statusReceived(ref);
        if(isStatusOfFollowingUser(ref) || ref->user->id==account()->userId())
            emit homeNewStatusReceived(ref);
        if(isMention(ref))
            emit mentionsNewStatusReceived(ref);
        if(ref->retweetedStatus){
            // maybe retweet?
            if(ref->retweetedStatus->user->id==account()->userId()){
                emit statusRetweeted(ref);
            }
        }
    }else{
        QVariantMap map=var.toMap();
        if(map.contains("delete")){
            map=map["delete"].toMap();
            if(map.contains("status")){
                map=map["status"].toMap();
                STObjectManager::StatusRef st=STObjectManager::sharedManager()->status(map, true);
                if(st){
                    if(st->retweetedStatus){
                        st->retweetedStatus->setRetweetStatusId(st->user->id, 0);
                    }
                }
                emit statusRemoved(map["id"].toULongLong());
            }
        }else if(map.contains("friends")){

            QVariantList lst=map["friends"].toList();
            foreach(const QVariant& var, lst){
                quint64 id=var.toULongLong();
                if(id){
                    m_friends.insert(id);
                }
            }
        }else if(map.contains("warning")){
            map=map["warning"].toMap();
            if(map.contains("code")){
                QString code=map["code"].toString();
                if(code=="FALLING_BEHIND"){
                    int percentFull=map["percent_full"].toInt();
                    emit streamingJamming(percentFull);
                }
            }
        }else if(map.contains("event")){
            QString event=map["event"].toString();
            if(event=="follow"){
                STObjectManager::UserRef source=STObjectManager::sharedManager()->user(map["source"]);
                STObjectManager::UserRef target=STObjectManager::sharedManager()->user(map["target"]);
                if(!source){qWarning()<<"STUserStreamer: follow event without source"; return;}
                if(!target){qWarning()<<"STUserStreamer: follow event without target"; return;}
                if(source->id==account()->userId()){
                    // user followed someone.
                    m_friends.insert(target->id);
                    emit accountFollowedUser(target, source);
                }else{
                    emit userFollowedAccount(source, target);
                }
            }else if(event=="block"){
                STObjectManager::UserRef source=STObjectManager::sharedManager()->user(map["source"]);
                STObjectManager::UserRef target=STObjectManager::sharedManager()->user(map["target"]);
                if(!source){qWarning()<<"STUserStreamer: block event without source"; return;}
                if(!target){qWarning()<<"STUserStreamer: block event without target"; return;}
                emit accountBlockedUser(target, source);
            }else if(event=="unblock"){
                STObjectManager::UserRef source=STObjectManager::sharedManager()->user(map["source"]);
                STObjectManager::UserRef target=STObjectManager::sharedManager()->user(map["target"]);
                if(!source){qWarning()<<"STUserStreamer: unblock event without source"; return;}
                if(!target){qWarning()<<"STUserStreamer: unblock event without target"; return;}
                emit accountUnblockedUser(target, source);
            }else if(event=="favorite"){
                STObjectManager::UserRef source=STObjectManager::sharedManager()->user(map["source"]);
                STObjectManager::StatusRef target=STObjectManager::sharedManager()->status(map["target_object"], true, account()->userId());
                if(!source){qWarning()<<"STUserStreamer: favorite event without source"; return;}
                if(!target){qWarning()<<"STUserStreamer: favorite event without target"; return;}
                if(source->id==account()->userId()){
                    target->setFavorited(account()->userId(), true);
                    emit accountFavoritedStatus(target, source);
                }else
                    emit statusFavorited(target, source);
            }else if(event=="unfavorite"){
                STObjectManager::UserRef source=STObjectManager::sharedManager()->user(map["source"]);
                STObjectManager::StatusRef target=STObjectManager::sharedManager()->status(map["target_object"], true, account()->userId());
                if(!source){qWarning()<<"STUserStreamer: unfavorite event without source"; return;}
                if(!target){qWarning()<<"STUserStreamer: unfavorite event without target"; return;}
                if(source->id==account()->userId()){
                    target->setFavorited(account()->userId(), false);
                    emit accountUnfavoritedStatus(target, source);
                }else
                    emit statusUnfavorited(target, source);
            }else if(event=="list_created"){
                STObjectManager::UserRef source=STObjectManager::sharedManager()->user(map["source"]);
                STObjectManager::ListRef target=STObjectManager::sharedManager()->list(map["target_object"]);
                if(!source){qWarning()<<"STUserStreamer: list_created event without source"; return;}
                if(!target){qWarning()<<"STUserStreamer: list_created event without target"; return;}
                emit listCreated(target, source);
            }else if(event=="list_destroyed"){
                STObjectManager::UserRef source=STObjectManager::sharedManager()->user(map["source"]);
                STObjectManager::ListRef target=STObjectManager::sharedManager()->list(map["target_object"]);
                if(!source){qWarning()<<"STUserStreamer: list_destroyed event without source"; return;}
                if(!target){qWarning()<<"STUserStreamer: list_destroyed event without target"; return;}
                emit listDestroyed(target, source);
            }else if(event=="list_updated"){
                STObjectManager::UserRef source=STObjectManager::sharedManager()->user(map["source"]);
                STObjectManager::ListRef target=STObjectManager::sharedManager()->list(map["target_object"]);
                if(!source){qWarning()<<"STUserStreamer: list_updated event without source"; return;}
                if(!target){qWarning()<<"STUserStreamer: list_updated event without target"; return;}
                emit listUpdated(target, source);
            }else if(event=="list_member_added"){
                STObjectManager::UserRef source=STObjectManager::sharedManager()->user(map["source"]);
                STObjectManager::UserRef target=STObjectManager::sharedManager()->user(map["target"]);
                STObjectManager::ListRef list=STObjectManager::sharedManager()->list(map["target_object"]);
                if(!source){qWarning()<<"STUserStreamer: list_member_added event without source"; return;}
                if(!target){qWarning()<<"STUserStreamer: list_member_added event without target"; return;}
                if(!list){qWarning()<<"STUserStreamer: list_member_added event without list"; return;}

                if(target->id==account()->userId())
                    emit accountAddedToList(list, target);
                else
                    emit listMemberAdded(list, target, source);
            }else if(event=="list_member_removed"){
                STObjectManager::UserRef source=STObjectManager::sharedManager()->user(map["source"]);
                STObjectManager::UserRef target=STObjectManager::sharedManager()->user(map["target"]);
                STObjectManager::ListRef list=STObjectManager::sharedManager()->list(map["target_object"]);
                if(!source){qWarning()<<"STUserStreamer: list_member_removed event without source"; return;}
                if(!target){qWarning()<<"STUserStreamer: list_member_removed event without target"; return;}
                if(!list){qWarning()<<"STUserStreamer: list_member_removed event without list"; return;}
                if(target->id==account()->userId())
                    emit accountRemovedFromList(list, target);
                else
                    emit listMemberRemoved(list, target, source);
            }else if(event=="list_user_subscribed"){
                STObjectManager::UserRef source=STObjectManager::sharedManager()->user(map["source"]);
                STObjectManager::UserRef target=STObjectManager::sharedManager()->user(map["target"]);
                STObjectManager::ListRef list=STObjectManager::sharedManager()->list(map["target_object"]);
                if(!source){qWarning()<<"STUserStreamer: list_user_subscribed event without source"; return;}
                if(!target){qWarning()<<"STUserStreamer: list_user_subscribed event without target"; return;}
                if(!list){qWarning()<<"STUserStreamer: list_user_subscribed event without list"; return;}
                if(target->id==account()->userId())
                    emit userSubscribedToList(list, source);
                else
                    emit accountSubscribedList(list, source);
            }else if(event=="list_user_unsubscribed"){
                STObjectManager::UserRef source=STObjectManager::sharedManager()->user(map["source"]);
                STObjectManager::UserRef target=STObjectManager::sharedManager()->user(map["target"]);
                STObjectManager::ListRef list=STObjectManager::sharedManager()->list(map["target_object"]);
                if(!source){qWarning()<<"STUserStreamer: list_user_unsubscribed event without source"; return;}
                if(!target){qWarning()<<"STUserStreamer: list_user_unsubscribed event without target"; return;}
                if(!list){qWarning()<<"STUserStreamer: list_user_unsubscribed event without list"; return;}
                if(target->id==account()->userId())
                    emit userUnsubscribedFromList(list, source);
                else
                    emit accountUnsubscribedList(list, source);
            }else if(event=="user_update"){
                STObjectManager::UserRef source=STObjectManager::sharedManager()->user(map["source"]);
                if(!source){qWarning()<<"STUserStreamer: user_update event without source"; return;}
                emit accountUpdatedProfile(source);
            }else{
                qWarning()<<"unsupported event: "<<event;
            }
        }
    }
}

void STUserStreamer::unfollowedUser(quint64 uid){
    m_friends.remove(uid);
    emit accountUnfollowedUser(uid);
}

