#include "stzoomedimageport.h"
#include "stzoomedimageview.h"
#include <QGLWidget>
#include <QGLFormat>

STZoomedImagePort::STZoomedImagePort(QWidget *parent) :
    QGraphicsView(parent)
{

    if(QGLFormat::hasOpenGL()&&false){
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
        //this->setViewport(NULL);

        this->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
    }

    this->setScene(new QGraphicsScene(this));


    scene()->setBackgroundBrush(QBrush(QColor(0,0,0)));
    scene()->setItemIndexMethod(QGraphicsScene::NoIndex);

    m_view=new STZoomedImageView();
    m_view->setPos(0,0);
    m_view->setZValue(10000.f);
    this->scene()->addItem(m_view);

    m_view->setFocus();
}

void STZoomedImagePort::resizeEvent(QResizeEvent *event){
    m_view->setSize(this->size());

    this->setSceneRect(0.f, 0.f, this->size().width(), this->size().height());
    QGraphicsView::resizeEvent(event);
}

void STZoomedImagePort::setImage(QUrl ur, QPixmap px){
    m_view->setImage(ur, px);
}
void STZoomedImagePort::setProgressValue(float v){
    m_view->setProgressValue(v);
}
