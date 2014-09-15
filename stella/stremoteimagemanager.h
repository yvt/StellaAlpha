#ifndef STREMOTEIMAGEMANAGER_H
#define STREMOTEIMAGEMANAGER_H

#include <QObject>
#include <QObject>
#include "stobjectmanager.h"
#include <QMutex>
#include <QLinkedList>
#include <QPixmap>
#include <QUrl>
#include <QNetworkReply>

class STRemoteImageManager : public QObject
{
    Q_OBJECT



public:
    explicit STRemoteImageManager();
    virtual ~STRemoteImageManager();

    struct Ticket;

    typedef QMap<quint64, Ticket *> TicketPtrMap;
    typedef QLinkedList<Ticket *> TicketPtrList;

    struct Ticket{
        STRemoteImageManager *manager;
        quint64 hash;
        QImage image;
        QPixmap pixmap;
        QUrl url;
        QSet<QString> lastRedirectTargets;
        qint64 failedTime;

        QMutex mutex;
        unsigned int refCount;
        TicketPtrList::iterator unusedListIterator;
        QNetworkReply *reply;

        void retain();
        void release();
        Ticket():mutex(QMutex::Recursive),refCount(0){}
    };



    class TicketRef{
        Ticket *ticket;
    public:
        TicketRef():ticket(NULL){}
        TicketRef(Ticket *t):ticket(t){
            if(ticket)
            ticket->retain();
        }
        TicketRef(const TicketRef& t):ticket(t.ticket){
            if(ticket)
            ticket->retain();
        }
        ~TicketRef(){
            if(ticket)
            ticket->release();
        }
        void operator =(const TicketRef& t){
            if(t.ticket==ticket)return;
            if(ticket) ticket->release();;
            ticket=t.ticket;
            if(ticket) ticket->retain();
        }
        Ticket *operator ->() const{
            Q_ASSERT(ticket!=NULL);
            return ticket;
        }
        Ticket& operator *() const{
            return *ticket;
        }
        operator bool() const{
            return ticket!=NULL;
        }
        bool operator !() const{
            return ticket==NULL;
        }
    };
    friend struct Ticket;

    quint64 hashForUrl(QUrl);
    TicketRef requestUrl(QUrl);

    void purgeOldUnusedTickets();

private:

    QMutex *g_ticketsMutex;
    TicketPtrMap g_tickets;
    TicketPtrList g_unusedTickets;
    QNetworkAccessManager *g_netManager;

protected:
    int m_unusedTicketsLimit;

signals:
    void iconReady(quint64 hash);
private slots:
    void processRequests();
    bool doRequest(quint64, QUrl);
    void readyRead();
    void slotError();
};

#endif // STREMOTEIMAGEMANAGER_H
