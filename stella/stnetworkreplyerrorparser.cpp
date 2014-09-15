#include "stnetworkreplyerrorparser.h"

STNetworkReplyErrorParser::STNetworkReplyErrorParser(QObject *parent) :
    QObject(parent)
{
}

QString STNetworkReplyErrorParser::parseError(QNetworkReply::NetworkError err){
    switch(err){
    case QNetworkReply::NoError:
        return tr("No error.");
    case QNetworkReply::ConnectionRefusedError:
        return tr("Connection refused.");
    case QNetworkReply::RemoteHostClosedError:
        return tr("Remote host closed the connection.");
    case QNetworkReply::HostNotFoundError:
        return tr("Host not found.");
    case QNetworkReply::TimeoutError:
        return tr("Timed out.");
    case QNetworkReply::OperationCanceledError:
        return tr("Operation canceled.");
    case QNetworkReply::SslHandshakeFailedError:
        return tr("SSL handsshake failed.");
    case QNetworkReply::TemporaryNetworkFailureError:
        return tr("Network temporary unavailable.");
    case QNetworkReply::ProxyConnectionRefusedError:
        return tr("Proxy connection refused.");
    case QNetworkReply::ProxyConnectionClosedError:
        return tr("Proxy server closed the connection.");
    case QNetworkReply::ProxyNotFoundError:
        return tr("Proxy server not found.");
    case QNetworkReply::ProxyTimeoutError:
        return tr("Proxy timed out.");
    case QNetworkReply::ProxyAuthenticationRequiredError:
        return tr("Unauthorized by the proxy server.");
    case QNetworkReply::ContentAccessDenied:
        return tr("Access denied.");
    case QNetworkReply::ContentOperationNotPermittedError:
        return tr("Operation not permitted.");
    case QNetworkReply::ContentNotFoundError:
        return tr("Content not found.");
    case QNetworkReply::AuthenticationRequiredError:
        return tr("Unauthorized.");
    case QNetworkReply::ContentReSendError:
        return tr("Resubmission failure.");
    case QNetworkReply::ProtocolUnknownError:
        return tr("Unknown protocol.");
    case QNetworkReply::ProtocolInvalidOperationError:
        return tr("Invalid request.");
    case QNetworkReply::UnknownNetworkError:
        return tr("Network error.");
    case QNetworkReply::UnknownProxyError:
        return tr("Proxy error.");
    case QNetworkReply::UnknownContentError:
        return tr("Data error.");
    case QNetworkReply::ProtocolFailure:
        return tr("Protocol failure.");
    default:
        return tr("Unknown network error.");
    }
}

QString STNetworkReplyErrorParser::parseError(int ec){
    switch(ec){
    case 400:
        return tr("Bad request.");
    case 401:
        return tr("Unauthorized.");
    case 403:
        return tr("Forbidden.");
    case 404:
        return tr("Not found.");
    case 500:
        return tr("Internal server error.");
    case 501:
        return tr("Not implemented by server.");
    case 502:
        return tr("Bad gateway.");
    case 503:
        return tr("Server over capacity.");
    default:
        return parseError((QNetworkReply::NetworkError)ec);
    }
}
