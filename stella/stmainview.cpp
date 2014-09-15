#include "stmainview.h"
#include <QDebug>
#include "stjsonparser.h"
#include "ststandardview.h"
#include <QGLWidget>
#include "stauthorizeview.h"
#include <QLayout>
#include "ststartupview.h"
#include <QTimer>
#include "staccountsmanager.h"
#include "sttwittertext.h"
#include "stusericonmanager.h"
#include "stactionmanager.h"
#include "stthumbnailmanager.h"
#include "stzoomedimagemanager.h"
#include "stzoomedimagedownloader.h"

STMainView::STMainView(QWidget *parent) :
    QGraphicsView(parent)
{
    m_useOpenGL=true;
    m_newViewportReq=true;
    setupNewViewport();;



    // TWITPIC: 24c1b7abacb7a60ff04cd8ca5f213c20


    this->setScene(new QGraphicsScene(this));


    scene()->setBackgroundBrush(QBrush(QColor(70, 70, 70)));
    scene()->setItemIndexMethod(QGraphicsScene::NoIndex);

    m_startupView=new STStartupView();
    m_startupView->setPos(0,0);
    m_startupView->setZValue(10000.f);
    this->scene()->addItem(m_startupView);

    m_mainItem=NULL;

    //

    QTimer::singleShot(50, this, SLOT(startInitialization()));


}


void STMainView::startInitialization(){
    STObjectManager::sharedManager();
    STAccountsManager::sharedManager();
    STTwitterText::sharedInstance();
    STUserIconManager::sharedManager();
    STActionManager::sharedManager();
    STThumbnailManager::sharedManager();
    STZoomedImageDownloader::sharedDownloader();
    STZoomedImageManager::sharedManager();

    m_mainItem=new STStandardView();
    m_mainItem->setPos(0.f, 0.f);
    m_mainItem->setSize(this->size());
    this->scene()->addItem(m_mainItem);


    QTimer::singleShot(500, this, SLOT(startupDone()));

    m_mainItem->initializeInitialView();
}

void STMainView::startupDone(){
    m_startupView->deleteLater();
    m_startupView=NULL;
    m_mainItem->startOperation();
}

void STMainView::setupNewViewport(){
    if(!m_newViewportReq)return;
    m_newViewportReq=false;

    QGraphicsScene *sc=this->scene();
    if(QGLFormat::hasOpenGL()&&m_useOpenGL){
        QGLFormat glFormat(QGL::SampleBuffers);
        glFormat.setSwapInterval(1); // vsync
        glFormat.setDepth(false);
        //glFormat.setStencil(false);
        //this->setRenderHints(QPainter::SmoothPixmapTransform|
        //                     QPainter::Antialiasing);
        this->setViewport(new QGLWidget(glFormat));
        this->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
        //this->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
    }else{
        this->setViewport(NULL);

        this->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
    }

    if(sc)
        setScene(sc);
}

void STMainView::setOpenGLEnable(bool enable){
    if(enable==m_useOpenGL)return;
    m_useOpenGL=enable;
    m_newViewportReq=true;
 //   setupNewViewport();;
    metaObject()->invokeMethod(this, "setupNewViewport",
                               Qt::QueuedConnection);
}

void STMainView::addOpenGLIncompatibleWidget(QWidget *w){
    m_openGLincompatibleWidgets.insert(w);
    setOpenGLEnable(m_openGLincompatibleWidgets.empty());
}

void STMainView::removeOpenGLIncompatibleWidget(QWidget *w){
    m_openGLincompatibleWidgets.remove(w);
    setOpenGLEnable(m_openGLincompatibleWidgets.empty());
}


STMainView::~STMainView(){
    if(m_mainItem){
        STAccountsManager::sharedManager()->saveAllAccountsInfo();
        STAccountsManager::sharedManager()->saveAllAccountsTimeline();
    }
}


void STMainView::resizeEvent(QResizeEvent *event){
    if(m_mainItem)
    m_mainItem->setSize(this->size());
    if(m_startupView)
    m_startupView->setSize(this->size());


    this->setSceneRect(0.f, 0.f, this->size().width(), this->size().height());
    QGraphicsView::resizeEvent(event);
}

void STMainView::startAuthorizeView(STAuthorizeView *vw, QPoint pt, QRect r){
    QPixmap pixmap=QPixmap::grabWidget(this);
    vw->start(pixmap, pt, r);

    vw->setGeometry(0,0,pixmap.width(),pixmap.height());
    vw->setParent(this->parentWidget());
    vw->show();

    QTimer::singleShot(50, this, SLOT(hide()));
}

void STMainView::stopAuthorizeView(STAuthorizeView *vw){
    this->setVisible(true);
    vw->setParent(NULL);
}
