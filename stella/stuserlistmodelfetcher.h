#ifndef STUSERLISTMODELFETCHER_H
#define STUSERLISTMODELFETCHER_H

#include "ststdafx.h"
#include <QObject>
#include <QThread>
#include "stobjectmanager.h"

class STAccount;
class KQOAuthManager;
class KQOAuthRequest;

class STUserListModelFetcher : public QObject
{
    Q_OBJECT

    KQOAuthManager *m_manager;
    KQOAuthRequest *m_request;
    QString m_nextCursor;
    QMutex m_mutex;
public:
    explicit STUserListModelFetcher();
    virtual ~STUserListModelFetcher();

    bool hasMore();
signals:
    void userIdsFetched(QList<quint64>);
    void usersFetched(QList<STObjectManager::UserRef>);
    void fetchFailed(QString);
public slots:
    void doRequest(QUrl, STAccount *);

private slots:
    void requestDone(QByteArray, int);

private:
    void parseReturnedIds(QVariantList);
    void parseReturnedUsers(QVariantList);
};

#endif // STUSERLISTMODELFETCHER_H
