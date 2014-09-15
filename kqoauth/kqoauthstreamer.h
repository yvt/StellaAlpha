#ifndef KQOAUTHSTREAMER_H
#define KQOAUTHSTREAMER_H

#include <QObject>
#include <QUrl>
#include "kqoauthrequest.h"
#include <QAbstractSocket>
#include <QSslError>

class KQOAuthStreamerPrivate;
class KQOAuthStreamer : public QObject
{

    Q_OBJECT

    Q_PROPERTY( const QUrl& url READ url WRITE setUrl )
    Q_PROPERTY( const QString& httpMethod READ httpMethod WRITE setHttpMethod );


public:
    explicit KQOAuthStreamer(QObject *parent = 0);
    ~KQOAuthStreamer();

    void setUrl(const QUrl&);
    const QUrl& url();

    void setHttpMethod(const QString&);
    const QString& httpMethod();

    void setAdditionalParameters(const KQOAuthParameters&);

    void setConsumerKey(const QString &consumerKey);
    void setConsumerSecretKey(const QString &consumerSecretKey);
    void setTokenSecret(const QString &tokenSecret);
    void setToken(const QString &token);

    enum State{NotConnected, Connecting, Streaming};
    State state();

signals:
    
    void streamingStarted();
    void streamingErrorStarting();
    void streamingStopped();
    void streamingLineReceived(const QByteArray&);

public slots:

    void startStreaming();
    void stopStreaming();

private slots:

    void processReceivedData();
    void lineReceived(const QByteArray&);
    void connectionOpened();
    void connectionClosed();
    void slotError(QAbstractSocket::SocketError);
    void sslErrors(QList<QSslError>);

private:

    QByteArray requestHeader();
    QList< QPair<QString, QString> > requestParameters();
    QString oauthSignature();
    QByteArray requestBaseString();
    QByteArray encodedParamaterList(const QList< QPair<QString, QString> > &parameters);
    QString oauthTimestamp() ;
    QString oauthNonce()  ;
    QList<QByteArray> requestParameterBytes() ;

protected:
    KQOAuthStreamerPrivate *d_ptr;
private:
    Q_DECLARE_PRIVATE(KQOAuthStreamer);

};

#endif // KQOAUTHSTREAMER_H
