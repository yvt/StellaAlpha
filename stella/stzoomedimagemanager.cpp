#include "stzoomedimagemanager.h"
#include "stzoomedimagedownloader.h"
#include "stzoomedimagewindow.h"
#include <QDebug>

static QMap<QString, STZoomedImageWindow *> g_windows;

STZoomedImageManager::STZoomedImageManager(QObject *parent) :
    QObject(parent)
{
    connect(STZoomedImageDownloader::sharedDownloader(),
            SIGNAL(imageLoaded(QUrl,QPixmap)),
            this, SLOT(imageReceived(QUrl,QPixmap)));
    connect(STZoomedImageDownloader::sharedDownloader(),
            SIGNAL(imageLoadFailed(QUrl,QString)),
            this, SLOT(imageLoadFailed(QUrl,QString)));
    connect(STZoomedImageDownloader::sharedDownloader(),
            SIGNAL(imageLoadProgress(QUrl,float)),
            this, SLOT(imageLoadProgress(QUrl,float)));
}

STZoomedImageManager * STZoomedImageManager::sharedManager(){
    static STZoomedImageManager *manager=NULL;
    if(!manager){
        manager=new STZoomedImageManager();
    }
    return manager;
}

void STZoomedImageManager::imageLoadFailed(QUrl u, QString reason){
    if(g_windows.contains(u.toString())){
        STZoomedImageWindow *wnd=g_windows[u.toString()];
        wnd->close();
    }
}

void STZoomedImageManager::imageClosing(){
    STZoomedImageWindow *wnd=dynamic_cast<STZoomedImageWindow *>(sender());
    for(QMap<QString, STZoomedImageWindow *>::iterator it=g_windows.begin();it!=g_windows.end();it++){
        if(it.value()==wnd){
            g_windows.erase(it);
            return;
        }
    }
    qWarning()<<"STZoomedImageManager::imageClosing: unrecognized window closed";
}

void STZoomedImageManager::imageReceived(QUrl u, QPixmap pix){
    if(g_windows.contains(u.toString())){
        STZoomedImageWindow *wnd=g_windows[u.toString()];
        //wnd->setFocus(); // needed?
        wnd->setImage(u,pix);
        return;
    }
    STZoomedImageWindow *wnd=new STZoomedImageWindow();
    connect(wnd, SIGNAL(willClose()),
            this, SLOT(imageClosing()));
    wnd->setAttribute(Qt::WA_DeleteOnClose);
    wnd->setImage(u, pix);
    wnd->show();
    g_windows.insert(u.toString(), wnd);
}

void STZoomedImageManager::imageLoadProgress(QUrl u, float prg){
    if(g_windows.contains(u.toString())){
        STZoomedImageWindow *wnd=g_windows[u.toString()];
        wnd->setProgressValue(prg);
    }
}

void STZoomedImageManager::zoomImageWithUrl(QUrl u, bool needsProgressIndicator){
    if(g_windows.contains(u.toString())){
        STZoomedImageWindow *wnd=g_windows[u.toString()];
        wnd->setFocus();
        return;
    }
    if(!needsProgressIndicator){
        STZoomedImageDownloader::sharedDownloader()->requestLoad(u);
        return;
    }

    STZoomedImageWindow *wnd=new STZoomedImageWindow();
    connect(wnd, SIGNAL(willClose()),
            this, SLOT(imageClosing()));
    wnd->setAttribute(Qt::WA_DeleteOnClose);
    wnd->setImage(u, QPixmap());
    wnd->show();
    g_windows.insert(u.toString(), wnd);

    STZoomedImageDownloader::sharedDownloader()->requestLoad(u);
}
