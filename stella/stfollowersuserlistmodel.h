#ifndef STFOLLOWERSUSERLISTMODEL_H
#define STFOLLOWERSUSERLISTMODEL_H

#include "stuserlistmodel.h"

class STFollowersUserListModel : public STUserListModel
{
    Q_OBJECT
public:
    explicit STFollowersUserListModel(STAccount *account, quint64 userId, QObject *parent = 0);
    
signals:
    
private slots:
    void userFollowedAccount(STObjectManager::UserRef followedBy,
                             STObjectManager::UserRef thisUser);
};

#endif // STFOLLOWERSUSERLISTMODEL_H
