#include "stauthorizeview.h"
#include <QPainter>
#include "stauthorizecancelbutton.h"
#include "stwebview.h"
#include <QDebug>

STAuthorizeView::STAuthorizeView(QWidget *parent) :
    QWidget(parent)
{
    m_cancelButton=new STAuthorizeCancelButton(this);
    connect(m_cancelButton, SIGNAL(clicked()),
            this, SLOT(cancelButtonPressed()));
    m_webView=new STWebView(this);
}

void STAuthorizeView::start(QPixmap pix, QPoint buttonPos, QRect rt){
    m_bgPixmap=pix;
    setFixedSize(pix.size());
    update();
    m_cancelButton->move(buttonPos);

    m_webView->setParent(this);
    m_webView->setGeometry(rt);
    m_webView->show();
   // qDebug()<<rt;
}

void STAuthorizeView::setUrl(QUrl ur){
    m_webView->setUrl(ur);
}

void STAuthorizeView::paintEvent(QPaintEvent *){
    QPainter painter(this);
    painter.drawPixmap(0,0,m_bgPixmap);
    //painter.drawEllipse(0,0,200,200);
}

void STAuthorizeView::cancelButtonPressed(){
    emit cancelled();;
}
