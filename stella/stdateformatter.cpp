#include "stdateformatter.h"
#include <QDateTime>
#include <QLocale>

STDateFormatter::STDateFormatter(QObject *parent) :
    QObject(parent)
{
}



QString STDateFormatter::toRelativePastDateString(QDateTime dt){
    qint64 dif=dt.toUTC().msecsTo((QDateTime::currentDateTimeUtc()));
    if(dif<0)
        dif=0;
    quint64 msec=(quint64)dif;
    quint64 secs=((quint64)msec)/1000ULL; // rounding

    if(secs<=0ULL){
        static const QString fmt=tr("now");
        return fmt;
    }else if(secs<60ULL){
        static const QString fmt=tr("%1s");
        return fmt.arg(secs);
    }else if(secs<3600ULL){
        static const QString fmt=tr("%1m");
        return fmt.arg(secs/60ULL);
    }else if(secs<86400ULL){
        static const QString fmt=tr("%1h");
        return fmt.arg(secs/3600ULL);
    }else{
        static const QString fmt=tr("%1d");
        return fmt.arg(secs/86400ULL);
    }

}

QString STDateFormatter::toMiniDateString(QDateTime dt){
    QDateTime local=dt.toLocalTime();
    QDateTime cur=dt.currentDateTime();
    if(local.date().year()!=cur.date().year())
        return toSmallDateString(dt);
    return local.toString(tr("MMM d"));
}

QString STDateFormatter::toSmallDateString(QDateTime dt){
    QDateTime local=dt.toLocalTime();
    QDateTime cur=dt.currentDateTime();
    return local.toString(tr("MMM d, yyyy"));
}


QString STDateFormatter::toLongDateTimeString(QDateTime dt){
    QDateTime local=dt.toLocalTime();
    QDateTime cur=dt.currentDateTime();
    return local.toString(tr("MMM d, yyyy h:mm:ss AP"));
}




