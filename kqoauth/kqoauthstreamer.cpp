#include "kqoauthstreamer.h"
#include "kqoauthstreamer_p.h"
#include "kqoauthutils.h"
#include <QDateTime>
#include <QSslSocket>
#include <QStringList>

static QByteArray g_crlf;

#define debugOutput false

KQOAuthStreamerPrivate::KQOAuthStreamerPrivate(KQOAuthStreamer *parent):
    q_ptr(parent),
    m_url(QUrl()),
    m_method("GET"),
    m_streaming(false){
    if(g_crlf.isEmpty()){
        g_crlf="\r\n";
    }
}

KQOAuthStreamer::KQOAuthStreamer(QObject *parent) :
    QObject(parent),
    d_ptr(new KQOAuthStreamerPrivate(this))
{
}

KQOAuthStreamer::~KQOAuthStreamer(){
    stopStreaming();
    delete d_ptr;
}

void KQOAuthStreamer::setUrl(const QUrl& url){
    Q_D(KQOAuthStreamer);
    d->m_url=url;
}

const QUrl& KQOAuthStreamer::url(){
    Q_D(KQOAuthStreamer);
    return d->m_url;
}

void KQOAuthStreamer::setHttpMethod(const QString& method){
    Q_D(KQOAuthStreamer);
    d->m_method=method;
}

const QString& KQOAuthStreamer::httpMethod(){
    Q_D(KQOAuthStreamer);
    return d->m_method;
}

void KQOAuthStreamer::setAdditionalParameters(const KQOAuthParameters&v){
    Q_D(KQOAuthStreamer);
    d->m_aditionalParameters.clear();
    QList<QString> additionalKeys = v.keys();
    QList<QString> additionalValues = v.values();

    int i=0;
    foreach(QString key, additionalKeys) {
        QString value = additionalValues.at(i);
        d->m_aditionalParameters.append( qMakePair(key, value) );
        i++;
    }
}

void KQOAuthStreamer::setConsumerKey(const QString &consumerKey){
    Q_D(KQOAuthStreamer);
    d->m_consumerKey=consumerKey;
}

void KQOAuthStreamer::setConsumerSecretKey(const QString &consumerSecretKey){
    Q_D(KQOAuthStreamer);
    d->m_consumerSecret=consumerSecretKey;
}

void KQOAuthStreamer::setTokenSecret(const QString &tokenSecret){
    Q_D(KQOAuthStreamer);
    d->m_tokenSecret=tokenSecret;
}

void KQOAuthStreamer::setToken(const QString &token){
    Q_D(KQOAuthStreamer);
    d->m_token=token;
}


void KQOAuthStreamer::startStreaming(){
    Q_D(KQOAuthStreamer);
    stopStreaming();

    bool secure=false;

    if(d->m_url.scheme()=="https"){
        secure=true;
        d->m_socket=new QSslSocket();
    }else{
        d->m_socket=new QTcpSocket();
    }

    d->m_timestamp.clear();
    d->m_nonce.clear();
    d->m_buffer.clear();

    d->m_readingHeader=true;
    connect(d->m_socket, SIGNAL(readyRead()),
            this, SLOT(processReceivedData()));
    connect(d->m_socket, SIGNAL(disconnected()),
            this, SLOT(connectionClosed()));
    //connect(d->m_socket, SIGNAL(),
    //        this, SLOT(connectionClosed()));
    connect(d->m_socket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(slotError(QAbstractSocket::SocketError)));
    connect(d->m_socket, SIGNAL(connected()),
            this, SLOT(connectionOpened()));


    if(d->m_url.port()==-1){
        if(d->m_url.scheme()=="https")
            d->m_url.setPort(443);
        else
            d->m_url.setPort(80);
    }

    if(debugOutput){
        qWarning()<<"connecting to "<<d->m_url.host()<<":"<<d->m_url.port()<<"...";
    }
    if(secure){
        static_cast<QSslSocket *>(d->m_socket)->connectToHostEncrypted(d->m_url.host(),
                                                                       d->m_url.port());
        connect(static_cast<QSslSocket *>(d->m_socket), SIGNAL(sslErrors(QList<QSslError>)),
                this, SLOT(sslErrors(QList<QSslError>)));
    }else{
        d->m_socket->connectToHost(d->m_url.host(),
                                   d->m_url.port());
    }

    d->m_streaming=true;

}

void KQOAuthStreamer::stopStreaming(){
    Q_D(KQOAuthStreamer);
    if(!d->m_streaming)
        return;

    processReceivedData();
    d->m_socket->close();
    processReceivedData();
    if(!d->m_buffer.isEmpty()){
        if(debugOutput){
            qDebug()<<"buffer remaining:";
            qDebug()<<d->m_buffer;
        }
    }
    d->m_socket->deleteLater();
    d->m_socket=NULL;

    d->m_streaming=false;
    if(!d->m_readingHeader){
        emit streamingStopped();
    }
}

KQOAuthStreamer::State KQOAuthStreamer::state(){
    Q_D(KQOAuthStreamer);
    if(d->m_streaming){
        if(d->m_readingHeader){
            return Connecting;
        }else{
            return Streaming;
        }
    }else{
        return NotConnected;
    }
}

void KQOAuthStreamer::processReceivedData(){
    Q_D(KQOAuthStreamer);
    if(d->m_socket==NULL)
        return;

    int newLinePos=-1;

    forever{
        QByteArray data=d->m_socket->read(65536);
        if(data.isEmpty())
            break;
        if(newLinePos==-1){
            newLinePos=data.indexOf(g_crlf);
            if(newLinePos!=-1){
                newLinePos+=d->m_buffer.size();
            }
        }
        d->m_buffer.append(data);
    }

    int firstPos=0;
    while(newLinePos!=-1){
        lineReceived(d->m_buffer.mid(firstPos, newLinePos-firstPos));
        firstPos=newLinePos+g_crlf.size();
        newLinePos=d->m_buffer.indexOf(g_crlf, firstPos);
        if(!d->m_streaming){
            return;
        }
    }

    d->m_buffer=d->m_buffer.mid(firstPos);
}

void KQOAuthStreamer::lineReceived(const QByteArray& bytes){
    Q_D(KQOAuthStreamer);
    if(d->m_readingHeader){
        if(debugOutput)
            qWarning()<<" HDR: "<<bytes;
        if(bytes.isEmpty()){
            d->m_readingHeader=false;
            emit streamingStarted();
        }else{
            if(bytes.startsWith("HTTP/")){
                QStringList lst=QString(bytes).split(QChar(' '));
                int respId=0;
                if(lst.count()>=2){
                    respId=QString(lst[1]).toInt();
                }
                if(respId/100!=2){
                    if(debugOutput){
                        qDebug()<<"HTTP Code: "<<respId<<", aborting.";
                    }
                    stopStreaming();
                    emit streamingErrorStarting();
                }
            }else if(bytes.contains(':')){
                int pos=bytes.indexOf(':');
                Q_ASSERT(pos>=0);
                // TODO: do with headers
            }
        }
    }else{
        emit streamingLineReceived(bytes);
    }
}

void KQOAuthStreamer::connectionOpened(){
    Q_D(KQOAuthStreamer);

    QByteArray header=requestHeader();

    if(debugOutput){
        qDebug()<<"Header:";
        qDebug()<<header;
    }

    d->m_socket->write(header);
    d->m_socket->write(g_crlf);
    d->m_socket->flush();

    // TODO: POST

}

void KQOAuthStreamer::connectionClosed(){
    stopStreaming();
}

void KQOAuthStreamer::slotError(QAbstractSocket::SocketError){
    Q_D(KQOAuthStreamer);
    qWarning()<<"KQOAuthStreamer: socket error.";
    if(d->m_readingHeader){
        emit streamingErrorStarting();
    }
    stopStreaming();
}


QByteArray KQOAuthStreamer::requestHeader(){
    Q_D(KQOAuthStreamer);
    QByteArray header;
    QUrl url=d->m_url;

    if(d->m_method=="GET"){
        url.setQueryItems(d->m_aditionalParameters);
    }

    header+=d->m_method;
    header+=" ";
    header+=url.encodedPath();
    if(!url.encodedQuery().isEmpty()){
        header+="?";
        header+=url.encodedQuery().replace('@', "%40");
    }
    header+=" HTTP/1.1";
    header+=g_crlf;
    header+="Host: ";
    header+=d->m_url.encodedHost();
    header+=g_crlf;
    header+="Accept: */*";
    header+=g_crlf;
    header+="User-Agent: YaViT Stella Alpha";
    header+=g_crlf;

    QList<QByteArray> requestHeaders = requestParameterBytes();
    QByteArray authHeader;

    bool first = true;
    foreach (const QByteArray header, requestHeaders) {
        if (!first) {
            authHeader.append(", ");
        } else {
            authHeader.append("OAuth ");
            first = false;
        }

        authHeader.append(header);
    }

    header+="Authorization: ";
    header+=authHeader;
    header+=g_crlf;

    return header;
}

QList< QPair<QString, QString> > KQOAuthStreamer::requestParameters(){
    QList< QPair<QString, QString> > p;
    Q_D(KQOAuthStreamer);
    p.append( qMakePair( OAUTH_KEY_SIGNATURE_METHOD, (QString)"HMAC-SHA1" ));
    p.append( qMakePair( OAUTH_KEY_CONSUMER_KEY, d->m_consumerKey ));
    p.append( qMakePair( OAUTH_KEY_VERSION, (QString)"1.0" ));
    p.append( qMakePair( OAUTH_KEY_TIMESTAMP, this->oauthTimestamp() ));
    p.append( qMakePair( OAUTH_KEY_NONCE, this->oauthNonce() ));
    p.append( qMakePair( OAUTH_KEY_TOKEN, d->m_token ));
    return p;
}

QList<QByteArray> KQOAuthStreamer::requestParameterBytes() {
    //Q_D(KQOAuthStreamer);

    QList<QByteArray> requestParamList;

    QList< QPair<QString, QString> > prm=requestParameters();
    prm.append( qMakePair( OAUTH_KEY_SIGNATURE, oauthSignature()) );

    QPair<QString, QString> requestParam;
    QString param;
    QString value;
    foreach (requestParam, prm) {
        param = requestParam.first;
        value = requestParam.second;
        requestParamList.append(QString(param + "=\"" + value +"\"").toUtf8());
    }

    return requestParamList;
}
/*
void KQOAuthRequestPrivate::signRequest() {
    QString signature = this->oauthSignature();

}*/

QString KQOAuthStreamer::oauthSignature()  {
    Q_D(KQOAuthStreamer);
    /**
     * http://oauth.net/core/1.0/#anchor16
     * The HMAC-SHA1 signature method uses the HMAC-SHA1 signature algorithm as defined in [RFC2104] where the
     * Signature Base String is the text and the key is the concatenated values (each first encoded per Parameter
     * Encoding) of the Consumer Secret and Token Secret, separated by an ‘&’ character (ASCII code 38) even if empty.
     **/
    QByteArray baseString = this->requestBaseString();

    QString secret = QString(QUrl::toPercentEncoding(d->m_consumerSecret)) + "&" + QString(QUrl::toPercentEncoding(d->m_tokenSecret));
    QString signature = KQOAuthUtils::hmac_sha1(baseString, secret);

    if (true) {
        qDebug() << "========== KQOAuthRequest has the following signature:";
        qDebug() << " * Signature : " << QUrl::toPercentEncoding(signature) << "\n";
    }
    return QString( QUrl::toPercentEncoding(signature) );
}



static bool normalizedParameterSort(const QPair<QString, QString> &left, const QPair<QString, QString> &right) {
    QString keyLeft = left.first;
    QString valueLeft = left.second;
    QString keyRight = right.first;
    QString valueRight = right.second;

    if(keyLeft == keyRight) {
        return (valueLeft < valueRight);
    } else {
        return (keyLeft < keyRight);
    }
}
QByteArray KQOAuthStreamer::requestBaseString() {
    QByteArray baseString;
    Q_D(KQOAuthStreamer);
    // Every request has these as the commont parameters.
    baseString.append( d->m_method.toUtf8() + "&");                                                     // HTTP method
    baseString.append( QUrl::toPercentEncoding( d->m_url.toString(QUrl::RemoveQuery|QUrl::RemovePort) ) + "&" ); // The path and query components

    QList< QPair<QString, QString> > baseStringParameters;
    baseStringParameters.append(this->requestParameters());
    baseStringParameters.append(d->m_aditionalParameters);

    // Sort the request parameters. These parameters have been
    // initialized earlier.
    qSort(baseStringParameters.begin(),
          baseStringParameters.end(),
          normalizedParameterSort
          );

    // Last append the request parameters correctly encoded.
    baseString.append( encodedParamaterList(baseStringParameters) );

    if (debugOutput) {
        qDebug() << "========== KQOAuthRequest has the following base string:";
        qDebug() << baseString << "\n";
    }

    return baseString;
}

QByteArray KQOAuthStreamer::encodedParamaterList(const QList< QPair<QString, QString> > &parameters) {
    QByteArray resultList;

    bool first = true;
    QPair<QString, QString> parameter;

    // Do the debug output.
    if (debugOutput) {
        qDebug() << "========== KQOAuthRequest has the following parameters:";
    }
    foreach (parameter, parameters) {
        if(!first) {
            resultList.append( "&" );
        } else {
            first = false;
        }

        // Here we don't need to explicitely encode the strings to UTF-8 since
        // QUrl::toPercentEncoding() takes care of that for us.
        resultList.append( QUrl::toPercentEncoding(parameter.first)     // Parameter key
                           + "="
                           + QUrl::toPercentEncoding(parameter.second)  // Parameter value
                          );
        if (debugOutput) {
            qDebug() << " * "
                     << parameter.first
                     << " : "
                     << parameter.second;
        }
    }
    if (debugOutput) {
        qDebug() << "\n";
    }

    return QUrl::toPercentEncoding(resultList);
}

QString KQOAuthStreamer::oauthTimestamp() {
    Q_D(KQOAuthStreamer);
    if(!d->m_timestamp.isEmpty())
        return d->m_timestamp;
    QString s;
#if QT_VERSION >= 0x040700
    s=QString::number(QDateTime::currentDateTimeUtc().toTime_t());
#else
    s=QString::number(QDateTime::currentDateTime().toUTC().toTime_t());
#endif
    d->m_timestamp=s;
    return s;
}

QString KQOAuthStreamer::oauthNonce()  {
    Q_D(KQOAuthStreamer);
    if(!d->m_nonce.isEmpty())
        return d->m_nonce;
    QString s;
    s=QString::number(qrand());
    s+=QString::number(qrand());
    s+=QString::number(qrand());
    s+=QString::number(qrand());
    d->m_nonce=s;
    return s;
}

void KQOAuthStreamer::sslErrors(QList<QSslError>){
    // TODO: handle SSL errors
    QSslSocket *sock=dynamic_cast<QSslSocket *>(sender());
    Q_ASSERT(sock);
    sock->ignoreSslErrors();
}
