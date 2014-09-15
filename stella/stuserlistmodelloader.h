#ifndef STUSERLISTMODELLOADER_H
#define STUSERLISTMODELLOADER_H

#include "ststdafx.h"
#include <QObject>
#include "stobjectmanager.h"

class KQOAuthManager;
class KQOAuthRequest;
class STUserListModelLoader : public QObject
{
    Q_OBJECT

    struct Request{
        KQOAuthManager *manager;
        KQOAuthRequest *request;
        int pageId;
    };

    QMap<const KQOAuthManager *, Request> m_requests;

public:
    explicit STUserListModelLoader(QObject *parent = 0);
    virtual ~STUserListModelLoader();
signals:
    void loaded(int pageId, STObjectManager::UserRefList);
    void loadFailed(QString, int pageId);
public slots:
    void doRequest(QSet<quint64>, STAccount *, int pageId);

private slots:
    void requestDone(QByteArray, int);
};

#endif // STUSERLISTMODELLOADER_H
