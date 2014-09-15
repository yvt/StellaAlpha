#ifndef STNETWORKREPLYERRORPARSER_H
#define STNETWORKREPLYERRORPARSER_H

#include <QObject>
#include <QNetworkReply>

class STNetworkReplyErrorParser : public QObject
{
    Q_OBJECT
public:
    explicit STNetworkReplyErrorParser(QObject *parent = 0);
    
    static QString parseError(QNetworkReply::NetworkError);
    static QString parseError(int);

signals:
    
public slots:
    
};

#endif // STNETWORKREPLYERRORPARSER_H
