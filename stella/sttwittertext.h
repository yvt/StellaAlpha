#ifndef STTWITTERTEXT_H
#define STTWITTERTEXT_H

#include <QObject>
#include <QStringList>

class STTwitterText : public QObject
{
    Q_OBJECT
public:
    explicit STTwitterText(QObject *parent = 0);
    static STTwitterText *sharedInstance();

    int lengthOfTweet(const QString&);
    QStringList extractMentions(const QString&);
signals:
    
public slots:
    
};

#endif // STTWITTERTEXT_H
