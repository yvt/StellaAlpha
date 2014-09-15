#include "stmainwindow.h"
#include "ui_stmainwindow.h"
#include <QCloseEvent>
#include <QMacCocoaViewContainer>
#include <QMacNativeWidget>
#include <QVBoxLayout>

STMainWindow::STMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::STMainWindow)
{
    ui->setupUi(this);
    //this->setAttribute(Qt::WA_NoSystemBackground);
    //this->setAttribute(Qt::WA_TranslucentBackground);
    //this->setWindowFlags(this->windowFlags()|Qt::FramelessWindowHint);

}


void STMainWindow::closeEvent(QCloseEvent *ev){
    //initMainWindow();
    //ev->ignore();
    QMainWindow::closeEvent(ev);
}

STMainWindow::~STMainWindow()
{
    delete ui;
}
