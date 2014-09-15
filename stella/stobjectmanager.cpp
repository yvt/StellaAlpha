#include "stobjectmanager.h"
#include <QMutexLocker>
#include "stmath.h"
#include <QDebug>
#include "stfont.h"
#include <QJson/Serializer>
#include "staccountsmanager.h"

STObjectManager::STObjectManager(QObject *parent) :
    QObject(parent)
{
    qRegisterMetaType<STObjectManager::ObjectRef>("STObjectManager::ObjectRef");
    qRegisterMetaType<STObjectManager::StatusRef>("STObjectManager::StatusRef");
    qRegisterMetaType<STObjectManager::MessageRef>("STObjectManager::MessageRef");
    qRegisterMetaType<STObjectManager::UserRef>("STObjectManager::UserRef");
    qRegisterMetaType<STObjectManager::UserRefList>("STObjectManager::UserRefList");
    qRegisterMetaType<STObjectManager::ListRef>("STObjectManager::ListRef");
}

STObjectManager *STObjectManager::sharedManager(){
    static STObjectManager *manager=NULL;
    if(!manager){
        manager=new STObjectManager();
    }
    return manager;
}

STObjectManager::Object::Object(quint64 i, ObjectPool *pl):
    id(i),pool(pl){
    refCount=0;
    loadedTime=QDateTime::currentMSecsSinceEpoch();
}

STObjectManager::Object::~Object(){
}

QVariant STObjectManager::Object::toSTONVariant(){
    QVariantMap map;
    static QString dataString=STGoThroughCommonStringPool("data");
    static QString idString=STGoThroughCommonStringPool("id");
    static QString loadedAtString=STGoThroughCommonStringPool("loaded_at");
    map.insert(dataString, data);
    map.insert(idString, id);
    map.insert(loadedAtString, loadedTime);
    return map;
}


QVariant STObjectManager::Object::toRefVariant(){
    Q_ASSERT(false);
    return QVariant();
}

void STObjectManager::Object::retain(){
    QMutexLocker locker(&(pool->mutex()));
    refCount++;
    if(refCount==1){
        pool->objects.insert(id, this);
    }
}

void STObjectManager::Object::release(){
    QMutexLocker locker(&(pool->mutex()));

    Q_ASSERT(refCount>0);
    refCount--;
    if(refCount==0){
        pool->objects.remove(id);
        delete this;
    }
}

STObjectManager::Object *STObjectManager::ObjectPool::object(quint64 id) const{
    QMap<quint64, Object *>::const_iterator it=objects.find(id);
    if(it==objects.end())return NULL;
    return it.value();
}


static STObjectManager::ObjectPool g_statusPool;

static STObjectManager::EntityRange parseEntityCharRange(const QVariantMap& mp){
    STObjectManager::EntityRange range;
    QVariantList indices=mp["indices"].toList();
    if(indices.count()<2){
        // strange entity.
        range.charIndexStart=0;
        range.charIndexEnd=0;
        return range;
    }
    range.charIndexStart=indices[0].toInt();
    range.charIndexEnd=indices[1].toInt();
    return range;
}

static QVariantList entityRangesToSTONVariant(QList<STObjectManager::EntityRange> ranges){
    QVariantList lst;

    static QString charIndexStartString=STGoThroughCommonStringPool("char_index_start");
    static QString charIndexEndString=STGoThroughCommonStringPool("char_index_end");
    static QString entityString=STGoThroughCommonStringPool("entity");
    static QString entityTypeString=STGoThroughCommonStringPool("entity_type");
    static QString displayTextString=STGoThroughCommonStringPool("display_text");
    lst.reserve(ranges.count());
    foreach(const STObjectManager::EntityRange& range, ranges){
        QVariantMap mp;
        mp.insert(charIndexStartString, range.charIndexStart);
        mp.insert(charIndexEndString, range.charIndexEnd);
        mp.insert(entityString, range.entity);
        mp.insert(entityTypeString, range.entityType);
        mp.insert(displayTextString, range.displayText);
        lst.append(mp);
    }
    return lst;
}

static QList<STObjectManager::EntityRange> entityRangesFromSTONVariant(QVariant var){
    static QString charIndexStartString=STGoThroughCommonStringPool("char_index_start");
    static QString charIndexEndString=STGoThroughCommonStringPool("char_index_end");
    static QString entityString=STGoThroughCommonStringPool("entity");
    static QString entityTypeString=STGoThroughCommonStringPool("entity_type");
    static QString displayTextString=STGoThroughCommonStringPool("display_text");

    QList<STObjectManager::EntityRange> ranges;
    if(var.isNull()) return QList<STObjectManager::EntityRange>();
    QVariantList lst=var.toList();
    ranges.reserve(lst.count());
    foreach(QVariant var, lst){
        QVariantMap mp=var.toMap();
        if(!mp.contains(charIndexStartString)) continue;
        if(!mp.contains(charIndexEndString)) continue;
        if(!mp.contains(entityString)) continue;
        if(!mp.contains(entityTypeString)) continue;
        if(!mp.contains(displayTextString)) continue;

        STObjectManager::EntityRange er;
        er.charIndexStart=mp[charIndexStartString].toInt();
        er.charIndexEnd=mp[charIndexEndString].toInt();
        er.entity=mp[entityString];
        er.entityType=mp[entityTypeString].toString();
        er.displayText=mp[displayTextString].toString();

        ranges.append(er);
    }

    return ranges;
}

static void handleMediaLink(STObjectManager::EntityRange& er){
    QVariantMap ent=er.entity.toMap();
    static QString typeString="type";
    static QString mediaUrlString="media_url";
    static QString smallUrlString="small_url";
    static QString urlString="url";
    static QString expandedUrlString="expanded_url";
    static QString stImageEntityType="st_image";
    if(er.entityType=="media"){
        if(ent[typeString]=="photo"){
            QVariantMap ent2;
            ent2.insert(urlString, ent[urlString]);
            ent2.insert(mediaUrlString, ent[mediaUrlString]);
            ent2.insert(smallUrlString, ent[mediaUrlString].toString()+":thumb");
            er.entity=ent2;
            er.entityType=stImageEntityType;
        }else{
            // unsupported, convert to link
            er.entityType="urls";
        }
    }else if(er.entityType=="urls"){
        QString url=ent[urlString].toString();
        if(ent.contains(expandedUrlString))
            url=ent[expandedUrlString].toString();
        QVariantMap ent2;
        if(url.startsWith(QLatin1String("http://twitpic.com/"))||
                url.startsWith(QLatin1String("http://www.twitpic.com/"))){
            static QRegExp cond("http:\\/\\/(www[.]|)twitpic[.]com\\/([a-zA-Z0-9]+)");
            if(cond.exactMatch(url)){
                QString imgId=cond.cap(2);
                ent2.insert(urlString, ent[urlString]);
                ent2.insert(mediaUrlString, "http://twitpic.com/show/large/"+imgId);
                ent2.insert(smallUrlString, "http://twitpic.com/show/mini/"+imgId);
                er.entity=ent2;
                er.entityType=stImageEntityType;
            }
        }else if(url.startsWith(QLatin1String("http://f.hatena.ne.jp"))){
             static QRegExp cond("http:\\/\\/f[.]hatena[.]ne[.]jp\\/([a-zA-Z0-9_]+)\\/(\\d{8})(.*)");
             if(cond.exactMatch(url)){
                 QString userId=cond.cap(1);
                 QString date=cond.cap(2);
                 QString imgId=cond.cap(3);
                 static QString mediaUrlPattern=
                         "http://img.f.hatena.ne.jp/images/fotolife/%1/%2/%3/%4%5.jpg";
                 static QString smallUrlPattern=
                         "http://img.f.hatena.ne.jp/images/fotolife/%1/%2/%3/%4%5_120.jpg";
                 ent2.insert(urlString, ent[urlString]);
                 ent2.insert(mediaUrlString,
                             mediaUrlPattern.arg(userId.left(1), userId,
                                                 date, date, imgId));
                 ent2.insert(smallUrlString,
                             smallUrlPattern.arg(userId.left(1), userId,
                                                 date, date, imgId));
                 er.entity=ent2;
                 er.entityType=stImageEntityType;
             }
         }else if(url.startsWith(QLatin1String("http://moby.to/"))||
             url.startsWith(QLatin1String("http://www.moby.to/"))){
            static QRegExp cond("http:\\/\\/(www[.]|)moby[.]to\\/([a-zA-Z0-9]+)");
            if(cond.exactMatch(url)){
                QString imgId=cond.cap(2);
                ent2.insert(urlString, ent[urlString]);
                ent2.insert(mediaUrlString, "http://moby.to/"+imgId+":view");
                ent2.insert(smallUrlString, "http://moby.to/"+imgId+":square");
                er.entity=ent2;
                er.entityType=stImageEntityType;
            }
        }else if(url.startsWith(QLatin1String("http://yfrog.com/"))||
             url.startsWith(QLatin1String("http://www.yfrog.com/"))){
            static QRegExp cond("http:\\/\\/(www[.]|)yfrog[.]com\\/([a-zA-Z0-9]+)");
            if(cond.exactMatch(url)){
                QString imgId=cond.cap(2);
                ent2.insert(urlString, ent[urlString]);
                ent2.insert(mediaUrlString, "http://yfrog.com/"+imgId+":medium");
                ent2.insert(smallUrlString, "http://yfrog.com/"+imgId+":small");
                er.entity=ent2;
                er.entityType=stImageEntityType;
            }
        }else if(url.startsWith(QLatin1String("http://bctiny.com/"))||
             url.startsWith(QLatin1String("http://www.bctiny.com/"))){
            static QRegExp cond("http:\\/\\/(www[.]|)bctiny[.]com\\/p([a-zA-Z0-9]+)");
            if(cond.exactMatch(url)){
                QString imgIdB36=cond.cap(2);
                quint64 imgId=STDecodeBase36(imgIdB36);
                static QString tmpl="http://images.bcphotoshare.com/storages/%1/%2.jpg";
                ent2.insert(urlString, ent[urlString]);
                ent2.insert(mediaUrlString, tmpl.arg(imgId).arg("large"));
                ent2.insert(smallUrlString, tmpl.arg(imgId).arg("thumbnail"));
                er.entity=ent2;
                er.entityType=stImageEntityType;
            }
        }else if(url.startsWith(QLatin1String("http://img.ly/"))||
                 url.startsWith(QLatin1String("http://www.img.ly/"))){
            static QRegExp cond("http:\\/\\/(www[.]|)img[.]ly\\/([a-zA-Z0-9]+)");
            if(cond.exactMatch(url)){
                QString imgId=cond.cap(2);
                ent2.insert(urlString, ent[urlString]);
                ent2.insert(mediaUrlString, "http://img.ly/show/large/"+imgId);
                ent2.insert(smallUrlString, "http://img.ly/show/thumb/"+imgId);
                er.entity=ent2;
                er.entityType=stImageEntityType;
            }
        }else if(url.startsWith(QLatin1String("http://twitgoo.com/"))||
             url.startsWith(QLatin1String("http://www.twitgoo.com/"))){
            static QRegExp cond("http:\\/\\/(www[.]|)twitgoo[.]com\\/([a-zA-Z0-9]+)");
            if(cond.exactMatch(url)){
                QString imgId=cond.cap(2);
                ent2.insert(urlString, ent[urlString]);
                ent2.insert(mediaUrlString, "http://twitgoo.com/show/img/"+imgId);
                ent2.insert(smallUrlString, "http://twitgoo.com/show/thumb/"+imgId);
                er.entity=ent2;
                er.entityType=stImageEntityType;
            }
        }else if(url.startsWith(QLatin1String("http://youtu.be/"))||
                 url.startsWith(QLatin1String("http://www.youtu.be/"))){
            static QRegExp cond("http:\\/\\/(www[.]|)youtu[.]be\\/([a-zA-Z0-9]+)");
            if(cond.exactMatch(url)){
                QString imgId=cond.cap(2);
                ent2.insert(urlString, ent[urlString]);
                ent2.insert(smallUrlString, "http://i.ytimg.com/vi/"+imgId+"/default.jpg");
                er.entity=ent2;
                er.entityType="st_video";
            }
        }else if(url.startsWith(QLatin1String("http://imgur.com/"))||
                 url.startsWith(QLatin1String("http://www.imgur.com/"))){
            static QRegExp cond("http:\\/\\/(www[.]|)imgur[.]com\\/([a-zA-Z0-9]+)[.]jpg");
            if(cond.exactMatch(url)){
                QString imgId=cond.cap(2);
                ent2.insert(urlString, ent[urlString]);
                ent2.insert(mediaUrlString, "http://i.imgur.com/"+imgId+"l.jpg");
                ent2.insert(smallUrlString, "http://i.imgur.com/"+imgId+"s.jpg");
                er.entity=ent2;
                er.entityType=stImageEntityType;
            }
        }else if(url.startsWith(QLatin1String("http://lockerz.com/s/"))||
                 url.startsWith(QLatin1String("http://plixi.com/p/"))||
                 url.startsWith(QLatin1String("http://tweetphoto.com/"))){
            static QRegExp cond("http:\\/\\/(lockerz[.]com\\/s|plixi[.]com\\/p|tweetphoto[.]com)\\/([a-zA-Z0-9]+)");
            if(cond.exactMatch(url)){
                QString imgId=cond.cap(2);
                static QString tmpl="http://api.plixi.com/api/TPAPI.svc/imagefromurl?size=%1&url=%2";
                ent2.insert(urlString, ent[urlString]);
                ent2.insert(mediaUrlString, tmpl.arg("big", url));
                ent2.insert(smallUrlString, tmpl.arg("thumbnail", url));
                er.entity=ent2;
                er.entityType=stImageEntityType;
            }
        }else if(url.startsWith(QLatin1String("http://instagr.am/p/"))){
            static QRegExp cond("http:\\/\\/(www[.]|)instagr[.]am\\/p\\/([a-zA-Z0-9]+)\\/*");
            if(cond.exactMatch(url)){
                QString imgId=cond.cap(2);
                ent2.insert(urlString, ent[urlString]);
                ent2.insert(mediaUrlString, "http://instagr.am/p/"+imgId+"/media/?size=l");
                ent2.insert(smallUrlString, "http://instagr.am/p/"+imgId+"/media/?size=t");
                er.entity=ent2;
                er.entityType=stImageEntityType;
            }
        }else if(url.startsWith(QLatin1String("http://photozou.jp/photo/show/"))){
            static QRegExp cond("http:\\/\\/photozou[.]jp\\/photo\\/show\\/(\\d+)\\/([a-zA-Z0-9]+)");
            if(cond.exactMatch(url)){
                QString imgId=cond.cap(2);
                ent2.insert(urlString, ent[urlString]);
                ent2.insert(mediaUrlString, "http://photozou.jp/p/img/"+imgId);
                ent2.insert(smallUrlString, "http://photozou.jp/p/thumb/"+imgId);
                er.entity=ent2;
                er.entityType=stImageEntityType;
            }
        }else if(url.startsWith(QLatin1String("http://p.twipple.jp/"))){
            static QRegExp cond("http:\\/\\/p[.]twipple[.]jp\\/([a-zA-Z0-9]+)");
            if(cond.exactMatch(url)){
                QString imgId=cond.cap(1);
                ent2.insert(urlString, ent[urlString]);
                ent2.insert(mediaUrlString, "http://p.twipple.jp/show/large/"+imgId);
                ent2.insert(smallUrlString, "http://p.twipple.jp/show/thumb/"+imgId);
                er.entity=ent2;
                er.entityType=stImageEntityType;
            }
        }
    }
}

static QList<STObjectManager::EntityRange> handleEntities(QVariantMap allEntites, QString inputString, QString& outDisplayString){
    QMap<int, STObjectManager::EntityRange> ranges; // char index is raw text index. key=charIndexStart
    QVariantList entities;

    // hashtag
    entities=allEntites["hashtags"].toList();
    foreach(const QVariant& var, entities){
        QVariantMap entity=var.toMap();
        STObjectManager::EntityRange rng=parseEntityCharRange(entity);
        if(rng.charIndexStart>=rng.charIndexEnd)
            continue;
        static const QString entityType="hashtags";
        rng.entityType=entityType;
        rng.entity=var;
        ranges.insert(rng.charIndexStart, rng);
    }

    // media
    entities=allEntites["media"].toList();
    foreach(const QVariant& var, entities){
        QVariantMap entity=var.toMap();
        STObjectManager::EntityRange rng=parseEntityCharRange(entity);
        if(rng.charIndexStart>=rng.charIndexEnd)
            continue;
        static const QString entityType="media";
        rng.entityType=entityType;
        rng.entity=var;
        rng.displayText=entity["display_url"].toString();
        handleMediaLink(rng);
        ranges.insert(rng.charIndexStart, rng);
    }

    // urls
    entities=allEntites["urls"].toList();
    foreach(const QVariant& var, entities){
        QVariantMap entity=var.toMap();
        STObjectManager::EntityRange rng=parseEntityCharRange(entity);
        if(rng.charIndexStart>=rng.charIndexEnd)
            continue;
        static const QString entityType="urls";
        rng.entityType=entityType;
        rng.entity=var;
        rng.displayText=entity["display_url"].toString();
        handleMediaLink(rng);
        ranges.insert(rng.charIndexStart, rng);
    }

    // user_mentions
    entities=allEntites["user_mentions"].toList();
    foreach(const QVariant& var, entities){
        QVariantMap entity=var.toMap();
        STObjectManager::EntityRange rng=parseEntityCharRange(entity);
        if(rng.charIndexStart>=rng.charIndexEnd)
            continue;
        static const QString entityType="user_mentions";
        rng.entityType=entityType;
        rng.entity=var;
        ranges.insert(rng.charIndexStart, rng);
    }

    QString outputString;
    outputString.reserve(inputString.size());

    //int lastIndex=0;    // input string index UTF-16 w/ HTML entities
    int index=0;        // input string index UTF-16 w/ HTML entities
    int inIndex=0;      // input string index Unicode w/ HTML entities

    const QChar *chars=inputString.constData();
    STObjectManager::EntityRange *curInRange=NULL;
    STObjectManager::EntityRange curOutRange;
    QList<STObjectManager::EntityRange> outRanges;
    outRanges.reserve(ranges.count());

    while(index<inputString.length()){
        bool echo=true;
        if(curInRange){
            if(inIndex==curInRange->charIndexEnd){
                curOutRange.charIndexEnd=outputString.length();
                outRanges.push_back(curOutRange);
                curInRange=NULL;
            }else{
                if(!curInRange->displayText.isEmpty()){
                    echo=false;
                }
            }
        }

        if(!curInRange){
            if(ranges.contains(inIndex)){
                curInRange=&(ranges[inIndex]);
                curOutRange.displayText=curInRange->displayText;
                curOutRange.entity=curInRange->entity;
                curOutRange.entityType=curInRange->entityType;
                curOutRange.charIndexStart=outputString.length();
                if(curInRange->displayText.isEmpty()){
                }else{
                    outputString+=curInRange->displayText;
                    echo=false;
                }

            }
        }

        // what's current char?
        int charLength=1;
        if(index<inputString.length()-1){
            // maybe surrogate pair
            if(STFont::isLeadingSurrogatePair(chars[index].unicode()) &&
                    STFont::isTrailSurrogatePair(chars[index+1].unicode())){
                charLength=2;
            }
        }
        if(echo){
            int coronIndex=-1;
            if(charLength==1&&chars[index]==QChar('&')){
                coronIndex=inputString.indexOf(QChar(';'), index+1);
            }
            if(coronIndex!=-1){
                // isn't it HTML entities?
                QString entityName=inputString.mid(index, coronIndex-index+1);
                QString entityOutput=entityName;
                if(!entityName.compare(QLatin1String("&amp;"), Qt::CaseInsensitive)){
                    outputString.append(QChar('&'));
                }else if(!entityName.compare(QLatin1String("&lt;"), Qt::CaseInsensitive)){
                    outputString.append(QChar('<'));
                }else if(!entityName.compare(QLatin1String("&gt;"), Qt::CaseInsensitive)){
                    outputString.append(QChar('>'));
                }else if(!entityName.compare(QLatin1String("&quot;"), Qt::CaseInsensitive)){
                    outputString.append(QChar('"'));
                }else if(!entityName.compare(QLatin1String("&apos;"), Qt::CaseInsensitive)){
                    outputString.append(QChar('\''));
                }else{
                    goto notEntity;
                }
                index=coronIndex+1;
                inIndex+=entityName.count();
                continue;
            }else{
                notEntity:
                while(charLength--){
                    outputString.append(chars[index]);
                    index++;
                }
            }
        }else{
            index+=charLength;
        }
        inIndex++;
    }

    if(curInRange){
        curOutRange.charIndexEnd=outputString.length();
        outRanges.push_back(curOutRange);
    }

    outDisplayString=outputString;

    return outRanges;
}

void STObjectManager::Status::setFavorited(quint64 accountId, bool favorited){
    if(!accountId)
        return;
    if(favorited){
        if(!isFavoritedBy(accountId)){
            favoritedBy.append(accountId);
        }
    }else{
        for(int i=0;i<favoritedBy.count();i++){
            if(favoritedBy[i]==accountId){
                favoritedBy.remove(i);
                break;
            }
        }
    }
    Q_ASSERT(isFavoritedBy(accountId)==favorited);
}

bool STObjectManager::Status::isFavoritedBy(quint64 accountId) const{
    if(accountId==0)
        return 0;
    for(int i=0;i<favoritedBy.count();i++)
        if(favoritedBy[i]==accountId)
            return true;
    return false;
}

void STObjectManager::Status::setRetweetStatusId(quint64 accountId, quint64 newRetweetStatusId){
    if(!accountId)
        return;
    if(newRetweetStatusId){
        for(int i=0;i<retweets.count();i++){
            if(retweets[i].accountId==accountId){
                retweets[i].retweetStatusId=newRetweetStatusId;
                return;
            }
        }
        RetweetInfo info;
        info.accountId=accountId;
        info.retweetStatusId=newRetweetStatusId;
        retweets.append(info);
    }else{
        for(int i=0;i<retweets.count();i++){
            if(retweets[i].accountId==accountId){
                retweets.remove(i);
                break;
            }
        }
    }
    Q_ASSERT(retweetStatusId(accountId)==newRetweetStatusId);
}

quint64 STObjectManager::Status::retweetStatusId(quint64 accountId) const{
    if(accountId==0)
        return 0;
    for(int i=0;i<retweets.count();i++)
        if(retweets[i].accountId==accountId)
            return retweets[i].retweetStatusId;
    return 0;
}

STObjectManager::Ref<STObjectManager::Status> STObjectManager::status(const QVariant &obj, bool weak, quint64 accountId){
    QMutexLocker locker(&(g_statusPool.mutex()));
    QVariantMap map=obj.toMap();

    if(map.isEmpty()) return Ref<Status>();

    quint64 id=map["id"].toULongLong();
    if(id==0)
        return Ref<Status>();
    if(!map.contains("text"))
        return Ref<Status>();
    if(!map.contains("user"))
        return Ref<Status>();

    Status *stObj=static_cast<Status *>(g_statusPool.object(id));
    if(!stObj){
        stObj=new Status(id, &g_statusPool);

    }else if(weak){
        qDebug()<<" status "<<id<<" exists, reusing it";
        return Ref<Status>(stObj);
    }else
        qDebug()<<" status "<<id<<" exists, overwriting";
    QMutexLocker locker2(&(stObj->mutex));

    stObj->data=map;
    stObj->user=user(stObj->data["user"]);
    stObj->data.remove("user");
    stObj->retweetedStatus=status(stObj->data["retweeted_status"], weak);
    stObj->data.remove("retweeted_status");
    stObj->createdAt=STParseDateTimeRFC2822(stObj->data["created_at"].toString());
    stObj->data.remove("created_at");
    //qDebug()<<stObj->createdAt.toString();
    if(accountId){
        // twitter bug: sometimes favorited and retweeted returns false no matter
        //              authenticated user favorited/retweeted the tweet
        // work-around: accept only true value.
        if(stObj->data["favorited"].toBool())
            stObj->setFavorited(accountId, stObj->data["favorited"].toBool());
        /*
        if(stObj->data["retweeted"].toBool())
            stObj->setRetweeted(accountId, stObj->data["retweeted"].toBool());*/
        if(stObj->data.contains("current_user_retweet")){
            QVariantMap rtwInfo=stObj->data["current_user_retweet"].toMap();
            quint64 rtwId=rtwInfo["id"].toULongLong();
            if(stObj->retweetedStatus){
                stObj->retweetedStatus->setRetweetStatusId(accountId, rtwId);
            }
        }
    }
    if(stObj->retweetedStatus){
        if(STAccountsManager::sharedManager()->account(stObj->user->id)){
            stObj->retweetedStatus->setRetweetStatusId(stObj->user->id, stObj->id);
        }
    }

    if(stObj->retweetedStatus){
        stObj->displayText=stObj->retweetedStatus->displayText;
        stObj->entityRanges=stObj->retweetedStatus->entityRanges;
    }else{
        // don't repeat entity processing because it is not fast
        if(stObj->data.contains("entities") && (stObj->entityRanges.isEmpty() || stObj->displayText.isEmpty())){
            stObj->entityRanges=handleEntities(stObj->data["entities"].toMap(), stObj->data["text"].toString(),
                                               stObj->displayText);
        }else if(stObj->displayText.isEmpty()){
            QVariantMap emptyMap;
            stObj->entityRanges=handleEntities(emptyMap, stObj->data["text"].toString(),
                                               stObj->displayText);
        }
    }
/*
    if(!stObj->user){
        qDebug()<<"status without user!!!";
        qDebug()<<QJson::Serializer().serialize(obj);
    }*/

    stObj->loadedTime=QDateTime::currentMSecsSinceEpoch();

    Q_ASSERT(stObj->user);
    return Ref<Status>(stObj);
}


QVariant STObjectManager::Status::toSTONVariant(){
    QVariantMap map=Object::toSTONVariant().toMap();
    static QString userString=STGoThroughCommonStringPool("user");
    static QString retweetedStatusString=STGoThroughCommonStringPool("retweeted_status");
    static QString createdAtString=STGoThroughCommonStringPool("created_at");
    static QString displayTextString=STGoThroughCommonStringPool("display_text");
    static QString entityRangesString=STGoThroughCommonStringPool("entity_ranges");
    static QString favoritedByString=STGoThroughCommonStringPool("favorited_by");
    static QString retweetsString=STGoThroughCommonStringPool("retweets");
    map.insert(userString, user->toRefVariant());
    if(retweetedStatus)
        map.insert(retweetedStatusString, retweetedStatus->toRefVariant());
    map.insert(createdAtString, createdAt);
    map.insert(displayTextString, displayText);
    map.insert(entityRangesString, entityRangesToSTONVariant(entityRanges));

    if(favoritedBy.count()==1){
        map.insert(favoritedByString, favoritedBy[0]);
    }else if(!favoritedBy.count()){
        QVariantList lst;
        foreach(quint64 acId, favoritedBy){
            lst.append(acId);
        }
        map.insert(favoritedByString, lst);
    }

    if(!retweets.isEmpty()){
        QVariantList lst;
        foreach(RetweetInfo r, retweets){
            lst.append(r.accountId);
            lst.append(r.retweetStatusId);
        }
        map.insert(retweetsString, lst);
    }



    return map;
}

STObjectManager::Ref<STObjectManager::Status> STObjectManager::statusFromSTON(QVariant var){
    QVariantMap map=var.toMap();
    static QString dataString=STGoThroughCommonStringPool("data");
    static QString idString=STGoThroughCommonStringPool("id");
    static QString userString=STGoThroughCommonStringPool("user");
    static QString retweetedStatusString=STGoThroughCommonStringPool("retweeted_status");
    static QString createdAtString=STGoThroughCommonStringPool("created_at");
    static QString displayTextString=STGoThroughCommonStringPool("display_text");
    static QString entityRangesString=STGoThroughCommonStringPool("entity_ranges");
    static QString favoritedByString=STGoThroughCommonStringPool("favorited_by");
    static QString retweetsString=STGoThroughCommonStringPool("retweets");
    static QString loadedAtString=STGoThroughCommonStringPool("loaded_at");
    if(!map.contains(idString)) return StatusRef();
    if(!map.contains(dataString)) return StatusRef();
    if(!map.contains(userString)) return StatusRef();
    if(!map.contains(createdAtString)) return StatusRef();

    quint64 id=map[idString].toULongLong();

    QMutexLocker locker(&(g_statusPool.mutex()));
    Status *stObj=static_cast<Status *>(g_statusPool.object(id));
    if(!stObj){
        stObj=new Status(id, &g_statusPool);
    }
    QMutexLocker locker2(&(stObj->mutex));

    if(map.contains(loadedAtString))
        stObj->loadedTime=map[loadedAtString].toULongLong();
    stObj->data=map[dataString].toMap();
    stObj->user=map[userString].value<STObjectManager::UserRef>();
    if(map.contains(retweetedStatusString))
        stObj->retweetedStatus=map[retweetedStatusString].value<STObjectManager::StatusRef>();
    stObj->createdAt=map[createdAtString].toDateTime();
    stObj->displayText=map[displayTextString].toString();

    if(map.contains(entityRangesString))
        stObj->entityRanges=entityRangesFromSTONVariant(map[entityRangesString]);

    if(map.contains(favoritedByString)){
        QVariant favBy=map[favoritedByString];
        if(favBy.type()==QVariant::List){
            QVariantList lst=favBy.toList();
            stObj->favoritedBy.reserve(lst.count());
            foreach(QVariant var, lst){
                stObj->setFavorited(var.toULongLong(), true);
            }
        }else{
            stObj->setFavorited(favBy.toULongLong(), true);
        }
    }

    if(map.contains(retweetsString)){
        QVariantList lst=map[retweetsString].toList();
        for(int i=0;i<lst.count()-1;i+=2){
            quint64 acc=lst[i].toULongLong();
            quint64 rtId=lst[i+1].toULongLong();
            stObj->setRetweetStatusId(acc, rtId);
        }
    }

    return StatusRef(stObj);
}

QVariant STObjectManager::Status::toRefVariant(){
    STObjectManager::StatusRef ref(this);
    return QVariant::fromValue(ref);
}


int STObjectManager::Status::refUserType(){
    sharedManager(); // forces meta type initialization
    static int type=QMetaType::type("STObjectManager::StatusRef");
    return type;
}

static STObjectManager::ObjectPool g_userPool;

STObjectManager::Ref<STObjectManager::User> STObjectManager::user(const QVariant &obj){
    QMutexLocker locker(&(g_userPool.mutex()));
    QVariantMap map=obj.toMap();

    if(map.isEmpty()) return Ref<User>();
    if(!map.contains("screen_name"))
        return Ref<User>();

    quint64 id=map["id"].toULongLong();

    User *stObj=static_cast<User *>(g_userPool.object(id));
    if(!stObj){
        stObj=new User(id, &g_userPool);
    }
    QMutexLocker locker2(&(stObj->mutex));

    stObj->data=map;
    stObj->data.remove("status");

    if(stObj->displayDescription.isEmpty())
    stObj->displayDescription=stObj->data["description"].toString();
    if(stObj->displayUrl.isEmpty())
    stObj->displayUrl=stObj->data["url"].toString();

    if(stObj->data.contains("entities")){
        QVariantMap entities=stObj->data["entities"].toMap();
        if(stObj->descriptionEntityRanges.isEmpty()){
            stObj->descriptionEntityRanges=handleEntities(entities["description"].toMap(),
                                                          stObj->data["description"].toString(),
                                                          stObj->displayDescription);
        }
        if(stObj->urlEntityRanges.isEmpty()){
            stObj->urlEntityRanges=handleEntities(entities["url"].toMap(),
                                                          stObj->data["url"].toString(),
                                                          stObj->displayUrl);
        }

    }

    stObj->loadedTime=QDateTime::currentMSecsSinceEpoch();

    return Ref<User>(stObj);
}


STObjectManager::Ref<STObjectManager::User> STObjectManager::userFromSTON(QVariant var){
    QVariantMap map=var.toMap();
    static QString idString=STGoThroughCommonStringPool("id");
    static QString dataString=STGoThroughCommonStringPool("data");
    static QString displayDescriptionString=STGoThroughCommonStringPool("display_description");
    static QString displayUrlString=STGoThroughCommonStringPool("display_url");
    static QString descriptionEntityRangesString=STGoThroughCommonStringPool("description_entity_ranges");
    static QString urlEntityRangesString=STGoThroughCommonStringPool("url_entity_ranges");
    static QString loadedAtString=STGoThroughCommonStringPool("loaded_at");
    if(!map.contains(dataString)) return UserRef();
    if(!map.contains(idString)) return UserRef();
    quint64 id=map[idString].toULongLong();

    QMutexLocker locker(&(g_userPool.mutex()));
    User *stObj=static_cast<User *>(g_userPool.object(id));
    if(!stObj){
        stObj=new User(id, &g_userPool);
    }
    QMutexLocker locker2(&(stObj->mutex));
    if(map.contains(loadedAtString))
        stObj->loadedTime=map[loadedAtString].toULongLong();
    stObj->data=map[dataString].toMap();

    stObj->displayDescription=map[displayDescriptionString].toString();
    stObj->displayUrl=map[displayUrlString].toString();
    stObj->descriptionEntityRanges=entityRangesFromSTONVariant(map[descriptionEntityRangesString]);
    stObj->urlEntityRanges=entityRangesFromSTONVariant(map[urlEntityRangesString]);

    return UserRef(stObj);
}

QVariant STObjectManager::User::toSTONVariant(){
    QVariantMap map=Object::toSTONVariant().toMap();
    static QString displayDescriptionString=STGoThroughCommonStringPool("display_description");
    static QString displayUrlString=STGoThroughCommonStringPool("display_url");
    static QString descriptionEntityRangesString=STGoThroughCommonStringPool("description_entity_ranges");
    static QString urlEntityRangesString=STGoThroughCommonStringPool("url_entity_ranges");
    map.insert(displayDescriptionString, displayDescription);
    map.insert(descriptionEntityRangesString, entityRangesToSTONVariant(descriptionEntityRanges));
    map.insert(displayUrlString, displayUrl);
    map.insert(urlEntityRangesString, entityRangesToSTONVariant(urlEntityRanges));


    return map;
}

QVariant STObjectManager::User::toRefVariant(){
    STObjectManager::UserRef ref(this);
    return QVariant::fromValue(ref);
}

int STObjectManager::User::refUserType(){
    sharedManager(); // forces meta type initialization
    static int type=QMetaType::type("STObjectManager::UserRef");
    return type;
}

static STObjectManager::ObjectPool g_messagePool;

STObjectManager::Ref<STObjectManager::Message> STObjectManager::message(const QVariant &obj){
    QMutexLocker locker(&(g_messagePool.mutex()));
    QVariantMap map=obj.toMap();

    if(map.isEmpty()) return Ref<Message>();

    quint64 id=map["id"].toULongLong();

    Message *stObj=static_cast<Message *>(g_messagePool.object(id));
    if(!stObj){
        stObj=new Message(id, &g_messagePool);
    }
    QMutexLocker locker2(&(stObj->mutex));

    stObj->data=map;
    stObj->recipient=user(stObj->data["recipient"]);
    stObj->data.remove("recipient");
    stObj->sender=user(stObj->data["sender"]);
    stObj->data.remove("sender");

    stObj->loadedTime=QDateTime::currentMSecsSinceEpoch();

    return Ref<Message>(stObj);
}


STObjectManager::Ref<STObjectManager::Message> STObjectManager::messageFromSTON(QVariant var){
    QVariantMap map=var.toMap();
    static QString idString=STGoThroughCommonStringPool("id");
    static QString dataString=STGoThroughCommonStringPool("data");
    static QString recipientString=STGoThroughCommonStringPool("recipient");
    static QString senderString=STGoThroughCommonStringPool("sender");
    static QString loadedAtString=STGoThroughCommonStringPool("loaded_at");
    if(!map.contains(dataString)) return MessageRef();
    if(!map.contains(idString)) return MessageRef();
    if(!map.contains(senderString)) return MessageRef();
    if(!map.contains(recipientString)) return MessageRef();
    quint64 id=map[idString].toULongLong();

    QMutexLocker locker(&(g_messagePool.mutex()));
    Message *stObj=static_cast<Message *>(g_messagePool.object(id));
    if(!stObj){
        stObj=new Message(id, &g_messagePool);
    }
    QMutexLocker locker2(&(stObj->mutex));
    if(map.contains(loadedAtString))
        stObj->loadedTime=map[loadedAtString].toULongLong();
    stObj->data=map[dataString].toMap();

    stObj->recipient=map[recipientString].value<STObjectManager::UserRef>();
    stObj->sender=map[senderString].value<STObjectManager::UserRef>();

    return MessageRef(stObj);
}

QVariant STObjectManager::Message::toSTONVariant(){
    QVariantMap map=Object::toSTONVariant().toMap();
    static QString recipientString=STGoThroughCommonStringPool("recipient");
    static QString senderString=STGoThroughCommonStringPool("sender");
    map.insert(recipientString, recipient->toRefVariant());
    map.insert(senderString, sender->toRefVariant());

    return map;
}


QVariant STObjectManager::Message::toRefVariant(){
    STObjectManager::MessageRef ref(this);
    return QVariant::fromValue(ref);
}

int STObjectManager::Message::refUserType(){
    sharedManager(); // forces meta type initialization
    static int type=QMetaType::type("STObjectManager::MessageRef");
    return type;
}


static STObjectManager::ObjectPool g_listPool;

STObjectManager::Ref<STObjectManager::List> STObjectManager::list(const QVariant &obj){
    QMutexLocker locker(&(g_listPool.mutex()));
    QVariantMap map=obj.toMap();

    if(map.isEmpty()) return Ref<List>();

    quint64 id=map["id"].toULongLong();

    List *stObj=static_cast<List *>(g_listPool.object(id));
    if(!stObj){
        stObj=new List(id, &g_listPool);
    }
    QMutexLocker locker2(&(stObj->mutex));

    stObj->data=map;
    stObj->owner=user(stObj->data["owner"]);
    stObj->data.remove("owner");

    stObj->loadedTime=QDateTime::currentMSecsSinceEpoch();

    return Ref<List>(stObj);
}



STObjectManager::Ref<STObjectManager::List> STObjectManager::listFromSTON(QVariant var){
    QVariantMap map=var.toMap();
    static QString idString=STGoThroughCommonStringPool("id");
    static QString dataString=STGoThroughCommonStringPool("data");
    static QString ownerString=STGoThroughCommonStringPool("owner");
    static QString loadedAtString=STGoThroughCommonStringPool("loaded_at");
    if(!map.contains(dataString)) return MessageRef();
    if(!map.contains(idString)) return MessageRef();
    if(!map.contains(ownerString)) return MessageRef();
    quint64 id=map[idString].toULongLong();

    QMutexLocker locker(&(g_listPool.mutex()));
    List *stObj=static_cast<List *>(g_listPool.object(id));
    if(!stObj){
        stObj=new List(id, &g_listPool);
    }
    QMutexLocker locker2(&(stObj->mutex));
    if(map.contains(loadedAtString))
        stObj->loadedTime=map[loadedAtString].toULongLong();
    stObj->data=map[dataString].toMap();

    stObj->owner=map[ownerString].value<STObjectManager::UserRef>();
    return ListRef(stObj);
}

QVariant STObjectManager::List::toSTONVariant(){
    QVariantMap map=Object::toSTONVariant().toMap();
    static QString ownerString=STGoThroughCommonStringPool("owner");
    map.insert(ownerString, owner->toRefVariant());

    return map;
}


QVariant STObjectManager::List::toRefVariant(){
    STObjectManager::ListRef ref(this);
    return QVariant::fromValue(ref);
}

int STObjectManager::List::refUserType(){
    sharedManager(); // forces meta type initialization
    static int type=QMetaType::type("STObjectManager::ListRef");
    return type;
}

