#ifndef STACTIONMANAGER_H
#define STACTIONMANAGER_H

#include <QObject>
#include "stobjectmanager.h"

class STActionManager : public QObject
{
    Q_OBJECT
    Q_ENUMS(TweetAction)
public:
    explicit STActionManager(QObject *parent = 0);
    static STActionManager *sharedManager();

    enum StatusAction{
        Favorite,
        Unfavorite,
        Retweet,
        DeleteStatus
    };

    QString consumerKey();
    QString consumerSecret();

public slots:
    void statusAction(quint64 statusId, quint64 accountId,
                      STActionManager::StatusAction);

signals:
    void statusActionFailed(quint64 statusId, quint64 accountId,
                            STActionManager::StatusAction);
    void statusActionDone(STObjectManager::StatusRef, quint64 accountId,
                          STActionManager::StatusAction);
private slots:
    void statusRequestDone(QByteArray, int);
    void accountWillRemoved(quint64);
};

#endif // STACTIONMANAGER_H
