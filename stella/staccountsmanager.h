#ifndef STACCOUNTSMANAGER_H
#define STACCOUNTSMANAGER_H

#include <QObject>

class STAccount;
typedef QList<STAccount *> STAccountList;

class STAccountsManager : public QObject
{
    Q_OBJECT
public:
    explicit STAccountsManager(QObject *parent = 0);
    static STAccountsManager *sharedManager();

    const STAccountList& accounts() const;
    STAccount *account(quint64) const;

    static QString consumerKey();
    static QString consumerSecret();

    static QString accountsPath();
    static QString accountPathFor(quint64 acId);
    static QString accountsOrderFilePath();

    void addAccount(STAccount *);
    void reorder(int oldIndex, int newIndex);

    void saveAllAccountsInfo();
    void saveAllAccountsTimeline();
    void saveAccountsOrder();

signals:
    void accountBeingAdded(quint64);
    void accountWasAdded(quint64);
    void accountBeingRemoved(quint64);
    void accountWasRemoved(quint64);
public slots:

};

#endif // STACCOUNTSMANAGER_H
