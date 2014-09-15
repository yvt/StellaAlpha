#include "stsimplestreamer.h"
#include "stjsonparser.h"
#include <QDebug>
#include <QVariantMap>
#include "staccount.h"

STSimpleStreamer::STSimpleStreamer(STAccount *ac, QUrl ur) :
    STStreamer(ac,ur)
{
}

void STSimpleStreamer::lineReceived(const QByteArray &data){
    STJsonParser parser;
    if(data.isEmpty())
        return;
    if(data.at(0)!='{') // not array nor hash
        return;

    //if(qrand()&15)return;
    QVariant var=parser.parse(data);
    if(var.isNull())
        return;

    STObjectManager::StatusRef ref=STObjectManager::sharedManager()->status(var, false,
                                                                                            account()->userId());
    if(ref){
        if(isStatusFiltered((ref))){
            return;
        }
        emit statusReceived(ref);
    }else{
        QVariantMap map=var.toMap();
        if(map.contains("delete")){
            map=map["delete"].toMap();
            if(map.contains("status")){
                map=map["status"].toMap();
                emit statusRemoved(map["id"].toULongLong());
            }
        }
    }
}
