#ifndef STOBJECTNOTATION_H
#define STOBJECTNOTATION_H

#include "ststdafx.h"
#include <QObject>

// Stella Object Notation is a good way to serialize variant object tree.

class STObjectNotation : public QObject
{
    Q_OBJECT
public:
    explicit STObjectNotation(QObject *parent = 0);
    
    static QByteArray serialize(QVariant);
    static QVariant parse(const QByteArray&);

    static bool serialize(QVariant, QString path, bool atomic=true);
    static QVariant parse(QString path);
signals:
    
public slots:
    
};

#endif // STOBJECTNOTATION_H
