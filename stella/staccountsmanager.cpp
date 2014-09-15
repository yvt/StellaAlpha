#include "ststdafx.h"
#include "staccountsmanager.h"
#include "staccount.h"
#include <QList>
#include <QSet>
#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QFileInfoList>
#include "stobjectnotation.h"
#include <QDebug>
#include "stjsonparser.h"
#include <QThread>
#include <QApplication>

static STAccountList g_accounts;
static QMap<quint64, STAccount *> g_accountMap;

static STAccountsManager * volatile g_sharedManager;


STAccountsManager::STAccountsManager(QObject *parent) :
    QObject(parent)
{
    // load accounts
    g_sharedManager=this;
    Q_ASSERT(thread()==QApplication::instance()->thread());
    QMap<quint64, STAccount *> g_acc;
    QDir dir(accountsPath());
    QFileInfo fi;
    QStringList lst;
    lst.append("*.account");
    foreach(fi, dir.entryInfoList(lst, QDir::Dirs)){

        QString infoPath=fi.filePath()+"/Info.ston";
        if(!fi.dir().exists(infoPath)){
            continue;
        }
        QVariant var=STObjectNotation::parse(infoPath);
        if(var.isNull()){
            qWarning()<<"STAccountsManager: failed to parse "<<infoPath;
            continue;
        }

        STAccount *ac=STAccount::accountWithInfo(var);
        if(!ac){
            qWarning()<<"STAccountsManager: invalid user info: "<<infoPath;
            continue;
        }

        quint64 userIdFromDirName=fi.baseName().toULongLong();
        if(userIdFromDirName!=ac->userId()){
            delete ac;
            qWarning()<<"STAccountsManager: unmatched user id: "<<infoPath;
            continue;
        }

        g_acc.insert(ac->userId(), ac);
    }

    // load user order
    QList<quint64> ids;
    {
        QFile fl(accountsOrderFilePath());
        if(fl.open(QFile::ReadOnly)){
            QByteArray dat=fl.readAll();
            fl.close();
            QVariantList lst=STJsonParser().parse(dat).toList();
            foreach(QVariant var, lst){
                if(var.canConvert(QVariant::ULongLong)){
                    ids.append(var.toULongLong());
                }
            }
        }
    }
    g_accounts.clear();
    foreach(quint64 id, ids){
        if(!g_acc.contains(id))
            continue;
        STAccount *ac=g_acc.take(id);
        g_accounts.append(ac);
        g_accountMap.insert(id, ac);
    }
    foreach(quint64 id, g_acc.keys()){
        if(g_accountMap.contains(id))
            continue;
        STAccount *ac=g_acc[id];
        g_accounts.append(ac);
        g_accountMap.insert(id, ac);
    }
}

STAccountsManager *STAccountsManager::sharedManager(){
    static QMutex m;
    QMutexLocker locker(&m);
    if(!g_sharedManager){
        g_sharedManager=new STAccountsManager();
    }
    return g_sharedManager;
}

const STAccountList& STAccountsManager::accounts() const{
    return g_accounts;
}

void STAccountsManager::reorder(int oldIndex, int newIndex){
    if(oldIndex==newIndex)return;
    Q_ASSERT(oldIndex<g_accounts.count());
    Q_ASSERT(newIndex<g_accounts.count());

    if(newIndex>oldIndex){
        STAccount *ac=g_accounts[oldIndex];
        g_accounts.insert(newIndex+1, ac);
        g_accounts.removeAt(oldIndex);
    }else{
        STAccount *ac=g_accounts.takeAt(oldIndex);
        g_accounts.insert(newIndex, ac);
    }
}

STAccount *STAccountsManager::account(quint64 id) const{
    QMap<quint64, STAccount *>::iterator it=g_accountMap.find(id);
    if(it==g_accountMap.end())return NULL;
    return it.value();
}


QString STAccountsManager::consumerKey(){
    static QString str="2NjamBTylJEjGANFg7jJpg";
    return str;
}

QString STAccountsManager::consumerSecret(){
    static QString str="27LoQOEWZ2oUabDpoUTPvFITn8VePvD9hosu91RHs";
    return str;
}

QString STAccountsManager::accountsPath(){
    QString dir=QDesktopServices::storageLocation(QDesktopServices::DataLocation);
    return dir+("/Accounts");
}

QString STAccountsManager::accountsOrderFilePath(){
    static QString tmpl="%1/AccountsOrder.js";
    return tmpl.arg(accountsPath());
}

QString STAccountsManager::accountPathFor(quint64 acId){
    static QString tmpl="%1/%2.account";
    return tmpl.arg(accountsPath(), QString::number(acId));
}

void STAccountsManager::addAccount(STAccount *ac){
    if(!ac)return;
    Q_ASSERT(!account(ac->userId()));

    emit accountBeingAdded(ac->userId());

    g_accounts.push_back(ac);
    g_accountMap.insert(ac->userId(), ac);

    emit accountWasAdded(ac->userId());
    ac->saveUserInfo();
    saveAccountsOrder();
}

void STAccountsManager::saveAccountsOrder(){
    QString str;
    str="/* Stella Alpha Accounts Order File */\n";
    str="/* Each line with user ID represents one account. */\n\n";
    str+="[\n";
    foreach(STAccount *ac, g_accounts){
        static QString s="%1, /* @%2 */\n";
        str+=s.arg(QString::number(ac->userId()), ac->userObject()->data["screen_name"].toString());
    }
    str+="]\n";

    QFile fl(accountsOrderFilePath());
    if(!fl.open(QFile::WriteOnly|QFile::Truncate)){
        qWarning()<<"STAccountsManager::saveAccountsOrder: failed to open "<<accountsOrderFilePath();
        return;
    }
    fl.write(str.toLatin1());
    fl.close();
}


void STAccountsManager::saveAllAccountsInfo(){
    saveAccountsOrder();
    foreach(STAccount *ac, g_accounts){
        ac->saveUserInfo();
    }
}

void STAccountsManager::saveAllAccountsTimeline(){
    foreach(STAccount *ac, g_accounts){
        ac->saveTimelines();
    }
}
