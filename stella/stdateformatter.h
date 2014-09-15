#ifndef STDATEFORMATTER_H
#define STDATEFORMATTER_H

#include "ststdafx.h"
#include <QObject>
#include <QDateTime>

class STDateFormatter : public QObject
{
    Q_OBJECT
public:
    explicit STDateFormatter(QObject *parent = 0);
    
    static QString toRelativePastDateString(QDateTime);
    static QString toMiniDateString(QDateTime);
    static QString toSmallDateString(QDateTime);
    static QString toLongDateTimeString(QDateTime);

signals:
    
public slots:
    
};

#endif // STDATEFORMATTER_H
