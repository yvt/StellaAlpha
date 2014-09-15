#ifndef STTIMELINEFETCHER_H
#define STTIMELINEFETCHER_H

#include <QObject>
#include "sttimeline.h"
#include <QList>

class KQOAuthManager;
class KQOAuthRequest;

class STTimelineFetcher : public QObject
{
    Q_OBJECT

    struct Request{
        STTimeline::FetchRequest req;
        KQOAuthManager *manager;
        KQOAuthRequest *request;
        STAccount *account;
    };

    QList<Request> m_requests;
    int indexForObject(KQOAuthManager *);

public:
    explicit STTimelineFetcher();
    virtual ~STTimelineFetcher();


signals:
    void fetchFailed(QString errorMsg, quint64 requestId);
    void fetchCompleted(const QList<STObjectManager::StatusRef>&, quint64 requestId);
public slots:
    
    void requestFetch(STTimeline::FetchRequest, QUrl url, STAccount *);
    void abortFetch(quint64);

    void requestDone(QByteArray, int code);
};

#endif // STTIMELINEFETCHER_H
