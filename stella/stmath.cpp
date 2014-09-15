#include "stmath.h"
#include <QRegExp>
#include <QDebug>
#include <QStringList>
#include <QDateTime>
#include <QLocale>

STMapCoord::STMapCoord(const QVariant& var):
    longitude(0),latitude(0){
    const QVariantList lst=var.toList();
    if(lst.count()>=2){
        longitude=lst[0].toDouble();
        latitude=lst[1].toDouble();
    }
}

QString STMapCoord::toString() const{
    return QString("%1,%2").arg(latitude, 0, 'f', 6).arg(longitude, 0, 'f', 6);
}

STMapCoord STMapCoord::lerp(STMapCoord a, STMapCoord b, double per){
    return STMapCoord(a.longitude*(1.-per)+b.longitude*per,
                      a.latitude*(1.-per)+b.latitude*per);
}

float STSmoothStep(float value){
    if(value<0.f)value=0.f;
    if(value>1.f)value=1.f;
    //return STOverDampedSpringStep((value));
    return value*value*(3.f-2.f*value);
}

float STForcedSmoothStep(float value, float s){
    if(value<0.f)value=0.f;
    if(value>1.f)value=1.f;
    if(s<1.5f){
        return value*(s+value*(3.f*(1.f-s)+value*(3.f*s-2.f-value*s)));
    }else{
        return 1.f-powf(1.f-value, s);
    }
}
float STForcedSmoothStepVelocity(float value, float s){
    if(value<0.f)value=0.f;
    if(value>1.f)value=1.f;
    if(s<1.5f){
        return s+value*(6.f*(1.f-s)+value*(9.f*s-6.f-value*s*4.f));
    }else{
        return s*powf(1.f-value,s-1.f);
    }
}

// kinetic approximation of oscilating spring
// where f'(0)=f(0)=0, f'(1)=0, f(1)=1
// equals STSmoothStep when limit r -> +0
float STDampedSpringStep(float value, float damping){
    const float r=9.f; // [rad]
    if(value<0.f)value=0.f;
    if(value>1.f)value=1.f;

    float k=1.f-value;
    float theta=r*value;
    return 1.f-powf(k, damping)*((damping/r)*sinf(theta)+cosf(theta));
}

// substitute STDampedSpringStep's r with ri (i=imaginary unit),
// you get over-damping equation.
float STOverDampedSpringStep(float value){
    const float r=3.3f; // [???]
    if(value<0.f)value=0.f;
    if(value>1.f)value=1.f;

    float theta=r*value;
    float v=1.f;
    float tt=(value*(value-2.f)+1.f);
    v-=tt*coshf(theta);
    v-=(2.f/r)*(value*(value-2.f)+1.f)*sinhf(theta);
    return v;
}

QDateTime STParseDateTimeRFC2822(const QString& str){
    // "created_at":"Sat Sep 08 09:27:45 +0000 2012"

    QRegExp regexp("[a-zA-Z]* ([a-zA-Z]{3}) ([0-9]{1,2} "
                   "[0-9]+:[0-9]+:[0-9]+) (\\+|-)([0-9]{4}) "
                   "([0-9]{4})");
    //qDebug()<<"regexping "<<str;

    if(regexp.exactMatch(str)){
        static const QString fmt="M dd HH:mm:ss yyyy";
        QString month;
        if(regexp.cap(1)=="Jan") month="1";
        else if(regexp.cap(1)=="Feb") month="2";
        else if(regexp.cap(1)=="Mar") month="3";
        else if(regexp.cap(1)=="Apr") month="4";
        else if(regexp.cap(1)=="May") month="5";
        else if(regexp.cap(1)=="Jun") month="6";
        else if(regexp.cap(1)=="Jul") month="7";
        else if(regexp.cap(1)=="Aug") month="8";
        else if(regexp.cap(1)=="Sep") month="9";
        else if(regexp.cap(1)=="Oct") month="10";
        else if(regexp.cap(1)=="Nov") month="11";
        else if(regexp.cap(1)=="Dec") month="12";

        QString dt2=month+" "+
                regexp.cap(2)+" "
                +regexp.cap(5);
        QDateTime dt=QDateTime::fromString(dt2,
                                           fmt);

        if(!dt.isValid()){
            qWarning()<<"STParseDateTimeRFC2822: Date parse failed: "<<dt2;
            qWarning()<<"STParseDateTimeRFC2822: input string: "<<str;

            return dt;
        }

        int timezone=regexp.cap(4).toInt();
        int tzHour=timezone/100;
        int tzMin=timezone%100;
        tzMin+=tzHour*60;
        if(regexp.cap(3)=="-")
            tzMin=-tzMin;
        dt.addSecs(-tzMin*60);
        dt.setTimeSpec(Qt::UTC);
        return dt.toLocalTime();
    }else{
        qWarning()<<"STParseDateTimeRFC2822: Date parse failed: "<<str;
        return QDateTime();
    }
}

static const char * g_commonnStrings[]={
    "char_index_start",
    "char_index_end",
    "contributors",
    "contributors_enabled",
    "coordinates",
    "created_at",
    "default_profile",
    "default_profile_image",
    "data",
    "description",
    "description_entity_ranges",
    "display_description",
    "display_text",
    "display_url",
    "entities",
    "entity",
    "entity_type",
    "entity_ranges",
    "expanded_url",
    "favorited",
    "favorited_by",
    "favourites_count",
    "follow_request_sent",
    "followers_count",
    "following",
    "friends_count",
    "geo",
    "geo_enabled",
    "hashtags",
    "id",
    "id_str",
    "in_reply_to_screen_name",
    "in_reply_to_status_id",
    "in_reply_to_status_id_str",
    "in_reply_to_user_id",
    "in_reply_to_user_id_str",
    "indices",
    "is_translator",
    "lang",
    "listed_count",
    "location",
    "name",
    "notifications",
    "owner",
    "place",
    "profile_background_color",
    "profile_background_image_url",
    "profile_background_image_url_https",
    "profile_background_tile",
    "profile_image_url",
    "profile_image_url_https",
    "profile_link_color",
    "profile_sidebar_border_color",
    "profile_sidebar_fill_color",
    "profile_text_color",
    "profile_use_background_image",
    "protected",
    "recipient",
    "retweet_count",
    "retweets",
    "retweeted",
    "retweeted_status",
    "screen_name",
    "sender",
    "source",
    "statuses_count",
    "text",
    "time_zone",
    "truncated",
    "url",
    "url_entity_ranges",
    "urls",
    "user",
    "user_mentions",
    "utc_offset",
    "verified",
    "loaded_at",
    "media_url",
    "small_url",
    "st_image",
    "st_video",
    NULL
};

static QStringList g_commonStringsQList;
static QMap<QString, int> g_commonStringPool;
struct CommonCStringPair{
    const char *text;
    int index;
};
static QVector<CommonCStringPair> g_commonsStringsVector;

static bool cStringComparer(const CommonCStringPair& a,
                            const CommonCStringPair& b){
    return strcmp(a.text,b.text)<0;
}

static void initCommonStringPool(){
    if(!g_commonsStringsVector.isEmpty())
        return;
    const char **strs=g_commonnStrings;
    for(const char **str=strs;*str;str++){
        QString s=*str;
        g_commonStringsQList.append(s);
        g_commonStringPool.insert(s, str-strs);

        CommonCStringPair pair;
        pair.text=*str;
        pair.index=str-strs;
        g_commonsStringsVector.push_back(pair);
    }
    qSort(g_commonsStringsVector.begin(),
          g_commonsStringsVector.end(),
          cStringComparer);
}

QString STGoThroughCommonStringPool(QString s, bool squeeze, int *outIndex){
    initCommonStringPool();
    QMap<QString, int>::iterator it=g_commonStringPool.find(s);
    if(it==g_commonStringPool.end()){
        if(outIndex)
            *outIndex=-1;
        if(!squeeze)return s;
        s.squeeze();
        return s;
    }
    if(outIndex)
        *outIndex=it.value();
    return it.key();
}

QString STGoThroughCommonStringPool(const char *s, int *outIndex){
    initCommonStringPool();
    QVector<CommonCStringPair>::iterator it;
    CommonCStringPair pair;
    pair.text=s;
    pair.index=0;
    it=qLowerBound(g_commonsStringsVector.begin(),
                   g_commonsStringsVector.end(),
                   pair, cStringComparer);
    if(it==g_commonsStringsVector.end() || strcmp(it->text, s)){
        if(outIndex)
            *outIndex=-1;
        return QString(s);
    }
    int index=it->index;
    if(outIndex)
        *outIndex=index;
    return g_commonStringsQList[index];
}

QString STCommonStringAtIndex(int i){
    initCommonStringPool();
    if(i>=g_commonStringsQList.size() || i<0)
        return QString();
    return g_commonStringsQList[i];
}

quint64 STDecodeBase36(const QString&s ){
    quint64 v=0;
    for(int i=0;i<s.length();i++){
        QChar c=s[i];
        v*=36ULL;
        if(c.isDigit()){
            v+=c.unicode()-'0';
        }else if(c.unicode()>='a' && c.unicode()<='z'){
            v+=c.unicode()-'a';
        }else if(c.unicode()>='A' && c.unicode()<='Z'){
            v+=c.unicode()-'A';
        }else{
            v/=36ULL;
            return v;
        }
    }
    return v;
}
