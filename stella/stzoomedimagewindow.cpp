#include "stzoomedimagewindow.h"
#include "ui_stzoomedimagewindow.h"
#include <QKeyEvent>

STZoomedImageWindow::STZoomedImageWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::STZoomedImageWindow)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::Dialog|
                         Qt::Tool|
                         Qt::CustomizeWindowHint|
                         Qt::WindowCloseButtonHint|
                         Qt::WindowMinMaxButtonsHint|
                         Qt::WindowTitleHint);
    this->setAttribute(Qt::WA_MacAlwaysShowToolWindow);

    QPalette pal=this->palette();
    pal.setColor(QPalette::Window, QColor(0,0,0));
    this->setPalette(pal);
}

STZoomedImageWindow::~STZoomedImageWindow()
{
    delete ui;
}

void STZoomedImageWindow::setImage(QUrl ur, QPixmap px){
    m_url=ur;
    ui->graphicsView->setImage(ur, px);
}

void STZoomedImageWindow::setProgressValue(float v){
    ui->graphicsView->setProgressValue(v);
}

void STZoomedImageWindow::keyPressEvent(QKeyEvent *ke){
    if(ke->key()==Qt::Key_Escape){
        close();
        ke->accept();
    }else{
        ke->ignore();
    }

}

void STZoomedImageWindow::closeEvent(QCloseEvent *){
    emit willClose();
}
