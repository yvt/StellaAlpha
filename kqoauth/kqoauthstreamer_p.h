#ifndef KQOAUTHSTREAMER_P_H
#define KQOAUTHSTREAMER_P_H

#include "kqoauthstreamer.h"
#include <QUrl>
#include <QTcpSocket>

class KQOAuthStreamerPrivate{
public:
    KQOAuthStreamerPrivate(KQOAuthStreamer *parent);

    KQOAuthStreamer * const q_ptr;
    QUrl m_url;
    QString m_method;
    QList< QPair<QString, QString> > m_aditionalParameters;

    QString m_consumerKey;
    QString m_consumerSecret;
    QString m_token;
    QString m_tokenSecret;

    QTcpSocket *m_socket;
    bool m_streaming;
    QByteArray m_buffer;
    bool m_readingHeader;

    QString m_timestamp;
    QString m_nonce;
};

#endif // KQOAUTHSTREAMER_P_H
