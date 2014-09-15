#include "stfriendsuserlistmodel.h"
#include "staccount.h"
#include "stuserstreamer.h"

STFriendsUserListModel::STFriendsUserListModel(STAccount *account, quint64 userId, QObject *parent) :
    STUserListModel(account, parent)
{
    QString urlStr="https://api.twitter.com/1.1/friends/ids.json?user_id=%1";
    setFetchEndpointUrl(QUrl(urlStr.arg(userId)));
    if(account->userId()==userId){
        STUserStreamer *streamer=account->userStreamer();
        connect(streamer, SIGNAL(accountUnfollowedUser(quint64)),
                this, SLOT(streamUserIdRemoved(quint64)));
    }
}
