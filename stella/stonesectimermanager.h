#ifndef STONESECTIMERMANAGER_H
#define STONESECTIMERMANAGER_H

#include <QObject>

class STOneSecTimerManager : public QObject
{
    Q_OBJECT
public:
    explicit STOneSecTimerManager(QObject *parent = 0);
    static STOneSecTimerManager *sharedManager();
    void timerActivated();
    void timerDeactivated();
private:
    void startOneSecTimer();
    void stopOneSecTimer();
signals:
    
    void timeout();
private slots:
    void oneSecTimeout();
};

#endif // STONESECTIMERMANAGER_H
