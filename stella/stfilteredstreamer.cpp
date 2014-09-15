#include "stfilteredstreamer.h"
#include <QStringList>
#include "stjsonparser.h"
#include <QThread>
#include <QDebug>
#include "staccount.h"

STFilteredStreamer::STFilteredStreamer(STAccount *account) :
    STStreamer(account, QUrl("https://stream.twitter.com/1.1/statuses/filter.json?include_my_retweet=1"))
{
    static bool registered=false;
    if(!registered){
        qRegisterMetaType<STFilteredStreamer::Filter>("STFilteredStreamer::Filter");
        registered=true;
    }

    connect(this, SIGNAL(streamingStarted()),
            this, SLOT(reportNewFilters()));
}

static bool matchSingleKeyword(const QString& text, const QString& keyword){
    return text.indexOf(keyword, 0, Qt::CaseInsensitive)>=0;
}


bool STFilteredStreamer::Filter::matches(const STObjectManager::StatusRef & status) const{
    if(type==STFilteredStreamer::User){
        return value.toULongLong()==status->user->id;
    }else if(type==STFilteredStreamer::Keyword){
        QStringList lst=value.split(" ");
        QString text=status->displayText;

        // replacing @mention #hashtag with placeholder and put the original to back
        foreach(const STObjectManager::EntityRange& range, status->entityRanges){
            QString outText;
            if(range.displayText.isEmpty()){
                outText=text.mid(range.charIndexStart, range.charIndexEnd-range.charIndexStart);
            }else{
                // TODO: maybe we shouldn't use displayText??
                //       for example http://twitter.jp/dummyafaafa/23423423423431423522 becomes
                //       twitter.jp/dumm... and it doesn't contain 23423423423431423522.
                outText=range.displayText;
            }

            // substitute
            for(int i=range.charIndexStart;i<range.charIndexEnd;i++){
                text[i]=QChar(',');
            }

            text+=QChar(' ');
            text+=outText;
        }

        foreach(const QString& str, lst){
            if(str.isEmpty())
                continue;
            if(!matchSingleKeyword(text, str))
                return false;
        }
        return true;
    }else if(type==STFilteredStreamer::Location){
        QStringList lst=value.split(",");
        if(lst.count()!=4) // invalid format?
            return false;

        QVariantMap coords=status->data["coordinates"].toMap();
        QVariantList coords2=coords["coordinates"].toList();
        if(coords2.count()<2){ // no geotag?
            return false;
        }

        double lngMin=lst[0].toDouble();
        double lngMax=lst[2].toDouble();
        double latMin=lst[1].toDouble();
        double latMax=lst[3].toDouble();

        double lng=coords2[0].toDouble();
        double lat=coords2[1].toDouble();

        return lng>=lngMin && lng<=lngMax && lat>=latMin && lat<=latMax;
    }else{
        Q_ASSERT(false);
    }
    return false;
}

STFilteredStreamer::FilterHandle::FilterHandle():m_streamer(NULL),m_valid(false){}

STFilteredStreamer::FilterHandle::FilterHandle(STFilteredStreamer *streamer):
    m_streamer(streamer),m_valid(false){}
STFilteredStreamer::FilterHandle::~FilterHandle(){
    clear();
}
void STFilteredStreamer::FilterHandle::setFilter(const Filter &flt){
    Q_ASSERT(m_streamer);
    if(m_valid && m_filter==flt)
        return;
    m_streamer->addFilter(flt);
    if(m_valid){
        m_streamer->removeFilter(m_filter);
    }
    m_filter=flt;
    m_valid=true;
}
void STFilteredStreamer::FilterHandle::clear(){
    if(m_valid){
        m_streamer->removeFilter(m_filter);
        m_valid=false;
    }
}

void STFilteredStreamer::reportNewFilters(){
    qDebug()<<"STFilteredStreamer: reportNewFilters "<<m_newFilters.count()<<" filters";
    foreach(const Filter& flt, m_newFilters.keys()){
        emit filterAdded(flt);
    }
    m_newFilters.clear();
}

void STFilteredStreamer::lineReceived(const QByteArray &data){
    STJsonParser parser;
    //qDebug()<<data;
    if(data.isEmpty())
        return;
    if(data.at(0)!='{') // not array nor hash
        return;

    QVariant var=parser.parse(data);
    if(var.isNull())
        return;



    STObjectManager::StatusRef ref=STObjectManager::sharedManager()->status(var, false,
                                                                                            account()->userId());
    if(ref){
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

void STFilteredStreamer::addFilter(Filter flt){
    if(QThread::currentThread()!=this->thread()){
        this->metaObject()->invokeMethod(this, "addFilter",
                                         Qt::QueuedConnection,
                                         Q_ARG(STFilteredStreamer::Filter, flt));
        return;
    }

    FilterMap::Iterator it=m_filters.find(flt);
    if(it==m_filters.end()){
        m_filters.insert(flt, 1);
    }else{
        it.value()++;
    }

    m_newFilters.insert(flt, 1);

    updateParameter();

    if(state()==NotConnected){

        startStreaming();
    }

}

void STFilteredStreamer::removeFilter(Filter flt){
    if(QThread::currentThread()!=this->thread()){
        this->metaObject()->invokeMethod(this, "removeFilter",
                                         Qt::QueuedConnection,
                                         Q_ARG(STFilteredStreamer::Filter, flt));
        return;
    }

    FilterMap::Iterator it=m_filters.find(flt);
    if(it==m_filters.end())
        return;
    it.value()--;
    if(it.value()==0){
        m_filters.erase(it);
    }

    if(m_filters.isEmpty()){
        stopStreaming();
    }else{
        updateParameter();
    }
}

void STFilteredStreamer::updateParameter(){
    QUrl url("https://stream.twitter.com/1.1/statuses/filter.json?include_my_retweet=1");
    QStringList follow, track, locations;
    for(FilterMap::Iterator it=m_filters.begin();it!=m_filters.end();it++){
        const Filter& flt=it.key();
        switch(flt.type){
        case User: follow.push_back(flt.value); break;
        case Keyword: track.push_back(flt.value); break;
        case Location: locations.push_back(flt.value); break;
        }
    }

    if(!follow.isEmpty()){
        url.addQueryItem("follow", follow.join(","));
    }
    if(!track.isEmpty()){
        url.addQueryItem("track", track.join(","));
    }
    if(!locations.isEmpty()){
        url.addQueryItem("locations", locations.join(","));
    }



    setEndpoint(url);
}
