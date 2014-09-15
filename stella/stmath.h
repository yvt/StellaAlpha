#ifndef STMATH_H
#define STMATH_H

#include "ststdafx.h"
#include <math.h>
#include <QDateTime>
#include <QString>
#include <QVariant>

struct STMapCoord{
    double longitude;
    double latitude;

    STMapCoord():
        longitude(0),latitude(0){
    }
    STMapCoord(double lon, double lat):
        longitude(lon),latitude(lat){}
    STMapCoord(const QVariant& var);
    QString toString() const;

    static STMapCoord lerp(STMapCoord a, STMapCoord b, double per);
};

float STSmoothStep(float);
float STForcedSmoothStep(float per, float initialVelocity);
float STForcedSmoothStepVelocity(float per, float initialVelocity);
float STDampedSpringStep(float, float damping=3.f);
float STOverDampedSpringStep(float);

QDateTime STParseDateTimeRFC2822(const QString&);

QString STGoThroughCommonStringPool(QString s, bool squeeze=true, int *outIndex=NULL);
QString STCommonStringAtIndex(int);
QString STGoThroughCommonStringPool(const char *, int *outIndex);

quint64 STDecodeBase36(const QString&);

#endif // STMATH_H
