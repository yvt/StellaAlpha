#include "stzoomedimagedownloader.h"
#include <QThread>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include "stnetworkreplyerrorparser.h"
#include <QDebug>

struct STZoomImageRequest{
    QNetworkReply *reply;
    QUrl url;
    QSet<QString> redirectedTargets;
};

static QNetworkAccessManager *g_networkManager=NULL;
static QMap<QNetworkReply *,STZoomImageRequest> g_requests;

STZoomedImageDownloader::STZoomedImageDownloader(QObject *parent) :
    QObject(parent)
{
    static QThread *th=new QThread();
    th->start(QThread::LowPriority);
    moveToThread(th);
}

STZoomedImageDownloader *STZoomedImageDownloader::sharedDownloader(){
    static STZoomedImageDownloader *downloader=NULL;
    if(!downloader){
        downloader=new STZoomedImageDownloader();
    }
    return downloader;
}

void STZoomedImageDownloader::requestLoad(QUrl url){
    if(thread()!=QThread::currentThread()){
        metaObject()->invokeMethod(this, "requestLoad",
                                   Qt::QueuedConnection,
                                   Q_ARG(QUrl, url));
        return;
    }

    Q_ASSERT(QThread::currentThread()==thread());

    if(!g_networkManager){
        g_networkManager=new QNetworkAccessManager(this);
    }

    // make sure there is no request with the same url
    for(QMap<QNetworkReply *,STZoomImageRequest>::iterator it=g_requests.begin();
        it!=g_requests.end();it++){
        const STZoomImageRequest& rq=it.value();
        if(rq.url==url)
            return;
    }

    qDebug()<<"Zooming: "<<url;

    QNetworkRequest req;
    req.setUrl(url);

    QNetworkReply *reply=g_networkManager->get(req);
    if(!reply->isRunning()){
        // failed:(
        emit imageLoadFailed(url, STNetworkReplyErrorParser::parseError(reply->error()));
        reply->deleteLater();
        return;
    }

    connect(reply, SIGNAL(finished()),
            this, SLOT(finished()));
    connect(reply, SIGNAL(downloadProgress(qint64,qint64)),
            this, SLOT(downloadProgress(qint64,qint64)));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(errorReceived(QNetworkReply::NetworkError)));

    STZoomImageRequest rq;
    rq.reply=reply;
    rq.url=url;

    Q_ASSERT(!g_requests.contains(reply));
    g_requests.insert(reply, rq);

    emit imageLoadStarted(rq.url);

}


void STZoomedImageDownloader::finished(){
    QNetworkReply *reply=static_cast<QNetworkReply *>(sender());
    if(!reply){
        // destoryed
        return;
    }
    reply->deleteLater();
    if(!g_requests.contains(reply))
        return;


    STZoomImageRequest req=g_requests[reply];

    emit imageLoadProgress(req.url, 2.f);

    QVariant redirectTarget=reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
    if(!redirectTarget.isNull()){
        // redirect.
        QString tgtStr=redirectTarget.toString();
        QUrl tgt=redirectTarget.toUrl();
        if(req.redirectedTargets.contains(tgtStr) ||req.redirectedTargets.count()>32){
            g_requests.remove(reply);
            emit imageLoadFailed(req.url, tr("Redirection loop detected."));
            return;
        }

        g_requests.remove(reply);


        QNetworkRequest nreq;
        nreq.setUrl(tgt);

        QNetworkReply *reply=g_networkManager->get(nreq);
        if(!reply->isRunning()){
            // failed:(
            emit imageLoadFailed(req.url, STNetworkReplyErrorParser::parseError(reply->error()));
            reply->deleteLater();
            return;
        }

        connect(reply, SIGNAL(finished()),
                this, SLOT(finished()));
        connect(reply, SIGNAL(downloadProgress(qint64,qint64)),
                this, SLOT(downloadProgress(qint64,qint64)));
        connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
                this, SLOT(errorReceived(QNetworkReply::NetworkError)));

        req.redirectedTargets.insert(tgtStr);
        req.reply=reply;

        Q_ASSERT(!g_requests.contains(reply));
        g_requests.insert(reply, req);

        return;
    }


    QByteArray arr=reply->readAll();

    g_requests.remove(reply);

    QPixmap pix;
    pix.loadFromData(arr);

    if(pix.isNull()){
        emit imageLoadFailed(req.url, tr("Corrupted image."));
    }else{
        emit imageLoaded(req.url, pix);
    }

}

void STZoomedImageDownloader::downloadProgress(qint64 rcvd, qint64 ttl){
    QNetworkReply *reply=static_cast<QNetworkReply *>(sender());
    if(!reply){
        // destoryed
        return;
    }
    if(!g_requests.contains(reply))
        return;
    STZoomImageRequest req=g_requests[reply];
    if(ttl<=0)return;

    float per=(float)rcvd/(float)ttl;
    if(per<.1f)per=-1.f;
    else if(per>.9f)per=-1.f;
    else per=(per-.1f)/.8f;
    emit imageLoadProgress(req.url, per);
}

void STZoomedImageDownloader::errorReceived(QNetworkReply::NetworkError er){
    QNetworkReply *reply=static_cast<QNetworkReply *>(sender());
    if(!reply){
        // destoryed
        return;
    }
    reply->deleteLater();
    if(!g_requests.contains(reply))
        return;


    STZoomImageRequest req=g_requests[reply];

    g_requests.remove(reply);

    emit imageLoadFailed(req.url, STNetworkReplyErrorParser::parseError(er));
}
