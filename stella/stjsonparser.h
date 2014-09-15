#ifndef STJSONPARSER_H
#define STJSONPARSER_H

#include "ststdafx.h"
#include <QObject>
#include <QVariant>
#include <QVariantMap>
#include <QVariantList>

class STJsonParser : public QObject
{
    Q_OBJECT
public:
    explicit STJsonParser(QObject *parent = 0);
    
    QVariant parse(QByteArray);
    QVariant parse(QByteArray, int& pos);

private:
    void skipWhitespace(QByteArray, int& pos);
    QVariant parseMap(QByteArray, int& pos);
    QVariant parseList(QByteArray, int& pos);
    QVariant parseString(QByteArray, int& pos);
    QVariant parseKeyword(QByteArray, int& pos);
    QVariant parseNumber(QByteArray, int& pos);

signals:
    
public slots:
    
};

#endif // STJSONPARSER_H
