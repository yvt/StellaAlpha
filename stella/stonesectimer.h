#ifndef STONESECTIMER_H
#define STONESECTIMER_H

#include <QObject>
#include <QLinkedList>

/* this timer is used to synchronize the time label like "1s ago", "2s ago", ... */
class STOneSecTimer : public QObject
{
    Q_OBJECT
    bool m_active;
public:
    explicit STOneSecTimer(QObject *parent = 0);
    virtual ~STOneSecTimer();

    void setActive(bool);
signals:
    void timeout();
private slots:
    void timeoutReceived();
};

#endif // STONESECTIMER_H
