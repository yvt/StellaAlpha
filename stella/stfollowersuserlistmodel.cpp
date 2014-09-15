#include "stfollowersuserlistmodel.h"
#include "staccount.h"
#include "stuserstreamer.h"

STFollowersUserListModel::STFollowersUserListModel(STAccount *account, quint64 userId, QObject *parent) :
    STUserListModel(account, parent)
{
    QString urlStr="https://api.twitter.com/1.1/followers/ids.json?user_id=%1";
    setFetchEndpointUrl(QUrl(urlStr.arg(userId)));
    if(account->userId()==userId){
        STUserStreamer *streamer=account->userStreamer();
        connect(streamer, SIGNAL(userFollowedAccount(STObjectManager::UserRef,STObjectManager::UserRef)),
                this, SLOT(userFollowedAccount(STObjectManager::UserRef,STObjectManager::UserRef)));
    }
}

void STFollowersUserListModel::userFollowedAccount(STObjectManager::UserRef followedBy, STObjectManager::UserRef){
    this->streamUserAdded(followedBy);
}
