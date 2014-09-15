#include "staccountitemview.h"
#include "stusericonview.h"
#include "staccount.h"
#include <QMutexLocker>
#include <QPainter>
#include <QImage>
#include <QLinearGradient>
#include "stbuttonview.h"

STAccountItemView::STAccountItemView(STAccount *acc, QSize sz, QGraphicsItem *parent) :
    QGraphicsObject(parent), m_size(sz), m_accountId(acc->userId())
{
    m_user=acc->userObject();

    m_buttonView=new STButtonView(this);
    m_buttonView->setButtonType(STButtonView::SidebarButton);
    m_buttonView->setZValue(-1.f);

    m_iconView=new STUserIconView(m_user, this);
    m_iconView->setPos(4,3);
    m_iconView->setSize(QSize(sz.width()-7, sz.width()-7));
    m_iconView->setAcceptedMouseButtons(Qt::NoButton);

    setCacheMode(DeviceCoordinateCache);
    m_buttonView->setFlag(ItemStacksBehindParent);

    m_buttonView->setSize(QSize(sz.width()+1, sz.height()+1));
    m_buttonView->setPos(0, -1);

    connect(m_buttonView, SIGNAL(activated()),
            this, SLOT(buttonActivated()));
    connect(m_buttonView, SIGNAL(dragInitated(QPointF)),
            this, SLOT(buttonDragged(QPointF)));

}

void STAccountItemView::buttonDragged(QPointF pt){
    m_buttonView->forceRelease();
    emit dragInitated(m_accountId, pt);
}

void STAccountItemView::buttonActivated(){
    emit triggered();
}

STFont::Layout STAccountItemView::layoutItem(STObjectManager::UserRef user){
    QString text;
    if(user){
        QMutexLocker lock(&(user->mutex));
        text=user->data["screen_name"].toString();
        text="@"+text;
    }else{
        text=tr("(unknown)");
    }
    return STFont::defaultFont()->layoutString(text);
}

QSize STAccountItemView::estimateSize(STObjectManager::UserRef user, int itemWidth){
    STFont::Layout layout=layoutItem(user);
    QRectF bnd=STFont::defaultFont()->boundingRectForLayout(layout);
    int height=(int)bnd.right();
    height+=itemWidth-7; // icon size
    height+=3+4+4; // margin width 3px, 4px 4px
    height+=1; // bottom border 1px
    return QSize(itemWidth, height);
}

void STAccountItemView::paint(QPainter *outPainter, const QStyleOptionGraphicsItem *, QWidget *){
    // first, draw text with 90deg rotated
    QImage image(QSize(m_size.height()+5, m_size.width()), QImage::Format_ARGB32_Premultiplied);
    image.fill(QColor(0,0,0,0));


    QPainter painter(&image);

    QLinearGradient shadowGrad(m_size.height(), 0, m_size.height()+5, 0);
    shadowGrad.setColorAt(0, QColor(0,0,0,64));
    shadowGrad.setColorAt(1, QColor(0,0,0,0));
    painter.fillRect(m_size.height(),0,5, m_size.width(),shadowGrad);

    STFont::DrawOption opt;
    opt.useSubpixelRendering=false; // cannot use subpixel render because rotated

    STFont *font=STFont::defaultFont();
    STFont::Layout layout=layoutItem(m_user);
    float top=m_size.width()+1;
    font->drawLayout(image, QPointF(top-1, (m_size.width()-12)/2), QColor(0,0,0,64),
                     layout, opt);
    font->drawLayout(image, QPointF(top, (m_size.width()-12)/2), QColor(200,200,200),
                     layout, opt);

    painter.fillRect(m_size.height()-1,0,1,m_size.width(), QColor(60,60,60));

    // do rotation
    QTransform trans;
    QImage image2=image.transformed(trans.rotate(90.f));



    outPainter->drawImage(0,0,image2);
}

QRectF STAccountItemView::boundingRect() const{
    return QRectF(0,0,m_size.width(),m_size.height()+5);
}

void STAccountItemView::accountSelected(quint64 i){
    m_buttonView->setChecked(i==m_accountId);
}
