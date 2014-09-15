#ifndef STUSERSTREAMER_H
#define STUSERSTREAMER_H

#include <QObject>
#include "ststreamer.h"
#include "stobjectmanager.h"
#include <QSet>

class STAccount;
class KQOAuthStreamer;
class STUserStreamer : public STStreamer
{
    Q_OBJECT

    QSet<quint64> m_friends;

    bool isStatusOfFollowingUser(const STObjectManager::StatusRef&);
    bool isMention(const STObjectManager::StatusRef&);

public:
    explicit STUserStreamer(STAccount *);
    
    bool following(quint64);
    void unfollowedUser(quint64);

signals:
    void statusReceived(STObjectManager::StatusRef);
    void homeNewStatusReceived(STObjectManager::StatusRef);
    void mentionsNewStatusReceived(STObjectManager::StatusRef);

    void accountBlockedUser(STObjectManager::UserRef target, STObjectManager::UserRef thisUser);
    void accountUnblockedUser(STObjectManager::UserRef target, STObjectManager::UserRef thisUser);
    void accountFollowedUser(STObjectManager::UserRef target, STObjectManager::UserRef thisUser);
    void accountUnfollowedUser(quint64); // actually not provided by User Streams
    void listCreated(STObjectManager::ListRef, STObjectManager::UserRef thisUser);
    void listDestroyed(STObjectManager::ListRef, STObjectManager::UserRef thisUser);
    void listUpdated(STObjectManager::ListRef, STObjectManager::UserRef thisUser);
    void listMemberAdded(STObjectManager::ListRef, STObjectManager::UserRef target, STObjectManager::UserRef thisUser);
    void listMemberRemoved(STObjectManager::ListRef, STObjectManager::UserRef target, STObjectManager::UserRef thisUser);
    void accountSubscribedList(STObjectManager::ListRef, STObjectManager::UserRef thisUser);
    void accountUnsubscribedList(STObjectManager::ListRef, STObjectManager::UserRef thisUser);
    void accountFavoritedStatus(STObjectManager::StatusRef, STObjectManager::UserRef thisUser);
    void accountUnfavoritedStatus(STObjectManager::StatusRef, STObjectManager::UserRef thisUser);
    void accountUpdatedProfile(STObjectManager::UserRef thisUser);

    void statusFavorited(STObjectManager::StatusRef, STObjectManager::UserRef);
    void statusUnfavorited(STObjectManager::StatusRef, STObjectManager::UserRef);
    void statusRetweeted(STObjectManager::StatusRef retweet); // retweet->retweetedStatus points the retweeted status
    void userSubscribedToList(STObjectManager::ListRef, STObjectManager::UserRef);
    void userUnsubscribedFromList(STObjectManager::ListRef, STObjectManager::UserRef);
    void accountAddedToList(STObjectManager::ListRef, STObjectManager::UserRef accountUser);
    void accountRemovedFromList(STObjectManager::ListRef, STObjectManager::UserRef accountUser);
    void userFollowedAccount(STObjectManager::UserRef byUser, STObjectManager::UserRef thisUser);

    void streamingJamming(int queueFullPercent);

    void statusRemoved(quint64);



protected slots:
    virtual void lineReceived(const QByteArray&);
};

#endif // STUSERSTREAMER_H
