#ifndef STFRIENDSUSERLISTMODEL_H
#define STFRIENDSUSERLISTMODEL_H

#include "stuserlistmodel.h"

class STFriendsUserListModel : public STUserListModel
{
    Q_OBJECT
public:
    explicit STFriendsUserListModel(STAccount *account, quint64 userId, QObject *parent = 0);
    
signals:
    
public slots:
    
};

#endif // STFRIENDSUSERLISTMODEL_H
