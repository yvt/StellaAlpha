#ifndef STTIMELINE_H
#define STTIMELINE_H

#include <QObject>
#include <QUrl>
#include "stobjectmanager.h"
#include <QMap>

class STTimelineFetcher;
class STAccount;
class STTimeline : public QObject
{
    Q_OBJECT



public:


    struct FetchRequest{
        quint64 requestId;
        quint64 minId;
        quint64 maxId;
    };

    struct Row{
        quint64 id;
        STObjectManager::StatusRef status;

        bool fetching;

        bool isFetchMore() const{return !status;}
        Row(){
            fetching=false;
        }
    };

    explicit STTimeline(QString identifier,QObject *parent = 0);
    virtual ~STTimeline();

    bool loadFromSTONVariant(QVariant);
    QVariant toSTONVariant();

    static STTimeline *createTimeline(QString identifier, QObject *parent=0);
    static STAccount *accountForIdentifier(const QString&);

    QString identifier() const{return m_identifier;}

    bool fetchSupported();
    virtual bool streamingSupported();

    QUrl fetchEndpoint() const{return m_fetchEndpoint;}
    void setFetchEndpoint(const QUrl& u){m_fetchEndpoint=u;}

    void start();
    void stop();

    bool isRunning() const{return m_running;}
    virtual bool stoppingStreamingSupported();

    Row *row(quint64);
    void fetchMore(quint64);

    QList<quint64> allRowIds();

    void prefetchFinalize();

    bool hasStatusIncludingRetweet(quint64);

protected:

    void requestFetch(const FetchRequest&);
    void fetchAbort(quint64);

    virtual void startStream();
    virtual void stopStream();

    void purgeRows();

private:

    STAccount *m_account;

    QString m_identifier;
    QUrl m_fetchEndpoint;

    QMap<quint64, Row> m_rows;
    bool m_running;

    QList<STObjectManager::StatusRef> m_prefetchRows;
    bool m_prefetching;
    quint64 m_prefetchId;

    QSet<quint64> m_containingStatuses; // for retweet, not retweet status but retweet"ed" status.

    bool m_shouldBulkLoad;

    STTimelineFetcher *m_fetcher;

    bool insertingStatus(STObjectManager::StatusRef);
    void deletingStatus(STObjectManager::StatusRef);


signals:
    
    void initateBulkLoad();
    void rowAdded(quint64);
    void rowRemoved(quint64);
    void fetchRowUpdated(quint64);

public slots:
    void fetchFailed(QString errorMsg, quint64 requestId);
    void fetchCompleted(const QList<STObjectManager::StatusRef>&, quint64 requestId);
    void streamReceived(const STObjectManager::StatusRef&);
    void streamRemoved(quint64);

    void streamStarted();
    void streamStopped(QString errorMsg);
    void streamFailedToStart(QString);
};

#endif // STTIMELINE_H
