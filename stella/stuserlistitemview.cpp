#include "stuserlistitemview.h"
#include "stuserlistmodel.h"
#include <QTimerEvent>
#include "stfont.h"
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>
#include <QPixmapCache>
#include <QLocale>

STUserListItemView::STUserListItemView(qint64 rowId, QSize size, STUserListModel *model, QGraphicsItem *parent) :
    QGraphicsObject(parent),  m_size(size),m_rowId(rowId), m_model(model)
{
    connect(m_model, SIGNAL(someUsersLoaded()),
            this, SLOT(itemStateUpdated()));

    m_iconView=new STUserIconView(STObjectManager::UserRef(), this);
    m_iconView->setPos(6.f, 6.f);
    m_iconView->setSize(QSize(52, 52));
    m_iconView->setAcceptedMouseButtons(Qt::LeftButton);
    m_iconView->setCursor(Qt::PointingHandCursor);
    connect(m_iconView, SIGNAL(clicked()),
            this, SLOT(showUser()));

    m_animStartTime=0;
    m_animDuration=200;

    setCacheMode(DeviceCoordinateCache);

    itemStateUpdated();

    m_fetchTimer=0;
    m_animTimer=0;

    if(!m_user){
        m_fetchTimer=startTimer(400);
        m_placeholderView=new QGraphicsPixmapItem(this);

        // draw placeholder
        QImage image(size.width(), size.height()-1, QImage::Format_RGB32);
        image.fill(QColor(70,70,70));

        STFont *font=STFont::defaultBoldFont();
        QString str=tr("Loading...");//QString("Loading...").arg(m_model->userIdAtRowId(rowId));
        STFont::Layout layout=font->layoutString(str);
        QRectF bnd=font->boundingRectForLayout(layout);

        font->drawLayout(image, QPointF(((float)image.width()-bnd.right())*.5f,
                                        ((float)image.height()-bnd.bottom())*.5f),
                         QColor(200,200,200, 100), layout, STFont::DrawOption());

        m_placeholderView->setPixmap(QPixmap::fromImage((image)));
        m_placeholderView->setPos(0,0);

        m_placeholderView->setAcceptedMouseButtons(Qt::LeftButton|Qt::RightButton);
        m_placeholderView->setCursor(Qt::ArrowCursor);

    }else{
        m_placeholderView=0;
    }

    setAcceptsHoverEvents(true);
}

static QString shortStringfy(qulonglong value){
    QLocale loc=QLocale::system();
    if(value<10000000ULL)
        return loc.toString(value);
    else if(value<100000000000ULL)
        return QString("%1 K").arg(loc.toString(value/1000ULL));
    else if(value<100000000000000ULL)
        return QString("%1 M").arg(loc.toString(value/1000000ULL));
    else if(value<100000000000000000ULL)
        return QString("%1 G").arg(loc.toString(value/1000000000ULL));
    else
        return QString("%1 T").arg(loc.toString(value/1000000000000ULL));
}

void STUserListItemView::paint(QPainter *outPainter, const QStyleOptionGraphicsItem *, QWidget *){
    QImage img(QSize(m_size.width(), m_size.height()+1), QImage::Format_RGB32);
    img.fill(QColor(70,70,70));

    QPainter painter(&img);
    painter.fillRect(QRect(0, 0, m_size.width(), 1), QColor(60,60,60));
    painter.fillRect(QRect(0,m_size.height(), m_size.width(), 1), QColor(60,60,60));

    STObjectManager::UserRef user=m_user;

    if(user){

        QVariantMap data;
        QList<STObjectManager::EntityRange> ranges;
        QString bio;
        {
            QMutexLocker mutex(&(user->mutex));
            data=user->data;
            ranges=user->descriptionEntityRanges;
            bio=user->displayDescription;
        }

        if(m_userNameLinkManager.isEmpty()){
            QString name=data["name"].toString();
            QList<STObjectManager::EntityRange> ranges;
            STObjectManager::EntityRange range;
            range.charIndexStart=0;
            range.charIndexEnd=name.length();
            range.entityType="user_object";
            range.entity=user->data;
            ranges.push_back(range);
            m_userNameLinkManager.init(name, ranges, STFont::defaultBoldFont());
            m_userNameLinkManager.setPos(QPointF(64.f, 7.f));
        }
        m_userNameLinkManager.draw(img, painter, QColor(240,240,240), QColor(240,240,240));

        if(m_userScreenNameLinkManager.isEmpty()){
            QString screenName="@"+data["screen_name"].toString();
            QList<STObjectManager::EntityRange> ranges;
            STObjectManager::EntityRange range;
            range.charIndexStart=0;
            range.charIndexEnd=screenName.length();
            range.entityType="user_object";
            range.entity=user->data;
            ranges.push_back(range);
            float x=64.f+STFont::defaultBoldFont()->boundingRectForLayout(m_userNameLinkManager.layout()).right()+2.f;
            STFont::Layout lay=STFont::defaultFont()->layoutString(screenName, (float)m_size.width()-3.f-x, true);
            m_userScreenNameLinkManager.init(lay, ranges, STFont::defaultFont());
            m_userScreenNameLinkManager.setPos(QPointF(x, 7.f));
        }
        m_userScreenNameLinkManager.draw(img, painter, QColor(200,200,200), QColor(200,200,200));

        if(m_bioLinkManager.isEmpty()){
            STFont::Layout lay=STFont::defaultFont()->layoutString(bio,(float)m_size.width()-3.f-64.f, true);
            m_bioLinkManager.init(lay, ranges, STFont::defaultFont());
            m_bioLinkManager.setPos(QPointF(64.f, 43.f));
        }
        m_bioLinkManager.draw(img, painter, QColor(255,255,255,100), QColor(220,128, 40, 100));



        QString str;
        STFont::Layout layout;
        float cx=64.f;
        QPixmap pix;

        static QPixmapCache::Key followersTextKey;
        if(!QPixmapCache::find(followersTextKey, &pix)){
            pix.load(":/stella/res/FollowersText.png");
            followersTextKey=QPixmapCache::insert(pix);
        }
        painter.drawPixmap((int)cx, 25+(10-pix.height()), pix);
        cx+=(float)pix.width()+3;

        str=shortStringfy(data["followers_count"].toULongLong());
        layout=STFont::defaultFont()->layoutString(str);
        STFont::defaultFont()->drawLayout(img, QPointF(cx, 25.f),
                                              QColor(255,255,255,200), layout, STFont::DrawOption());
        cx+=STFont::defaultFont()->boundingRectForLayout(layout).right()+10.f;

        cx=qMax(cx, (64.f+(float)m_size.width())*.5f);

        static QPixmapCache::Key followingTextKey;
        pix=QPixmap();
        if(!QPixmapCache::find(followingTextKey, &pix)){
            pix.load(":/stella/res/FollowingText.png");
            followingTextKey=QPixmapCache::insert(pix);
        }
        painter.drawPixmap((int)cx, 25+(10-pix.height()), pix);
        cx+=(float)pix.width()+3;

        str=shortStringfy(data["friends_count"].toULongLong());
        layout=STFont::defaultFont()->layoutString(str);
        STFont::defaultFont()->drawLayout(img, QPointF(cx, 25.f),
                                              QColor(255,255,255,200), layout, STFont::DrawOption());
        cx+=STFont::defaultFont()->boundingRectForLayout(layout).right();


    }



    outPainter->drawImage(0,-1,img);
}

void STUserListItemView::hoverEnterEvent(QGraphicsSceneHoverEvent *){

}

void STUserListItemView::hoverLeaveEvent(QGraphicsSceneHoverEvent *){
    if(m_userNameLinkManager.mouseLeave())
        update();
    if(m_userScreenNameLinkManager.mouseLeave())
        update();
    if(m_bioLinkManager.mouseLeave())
        update();
    updateCursor();
}

void STUserListItemView::hoverMoveEvent(QGraphicsSceneHoverEvent *event){
    if(m_userNameLinkManager.mouseMove(event->pos()))
        update();
    if(m_userScreenNameLinkManager.mouseMove(event->pos()))
        update();
    if(m_bioLinkManager.mouseMove(event->pos()))
        update();
    updateCursor();
}

void STUserListItemView::mousePressEvent(QGraphicsSceneMouseEvent *event){
    if(m_userNameLinkManager.mouseDown(event->pos()))
        update();
    if(m_userScreenNameLinkManager.mouseDown(event->pos()))
        update();
    if(m_bioLinkManager.mouseDown(event->pos()))
        update();
    updateCursor();
}

void STUserListItemView::mouseReleaseEvent(QGraphicsSceneMouseEvent *event){
    if(m_userNameLinkManager.mouseMove(event->pos()))
        update();
    if(m_userScreenNameLinkManager.mouseMove(event->pos()))
        update();
    if(m_bioLinkManager.mouseMove(event->pos()))
        update();
    STObjectManager::LinkActivationInfo info;
    if(m_userNameLinkManager.mouseUp()){
        emit linkActivated(m_userNameLinkManager.hotEntityRange(),info);
    }else if(m_userScreenNameLinkManager.mouseUp()){
        emit linkActivated(m_userScreenNameLinkManager.hotEntityRange(),info);
    }else if(m_bioLinkManager.mouseUp()){
        emit linkActivated(m_bioLinkManager.hotEntityRange(),info);
    }
    updateCursor();
}

void STUserListItemView::updateCursor(){
    if(m_userNameLinkManager.hotEntityRange()||
            m_userScreenNameLinkManager.hotEntityRange()||
            m_bioLinkManager.hotEntityRange()){
        setCursor(Qt::PointingHandCursor);
    }else{
        setCursor(Qt::ArrowCursor);
    }
}

QRectF STUserListItemView::boundingRect() const{
    return QRectF(0,-1,m_size.width(),m_size.height()+1);
}

void STUserListItemView::itemStateUpdated(){
    if(m_user)
        return;
    m_user=m_model->userAtRowId(m_rowId);
    if(m_user){
        m_iconView->setUser(m_user);
        update();

        m_animStartTime=QDateTime::currentMSecsSinceEpoch();
        if(!m_animTimer){
            m_animTimer=startTimer(15);
        }
    }
}

void STUserListItemView::timerEvent(QTimerEvent *evt){
    if(evt->timerId()==m_fetchTimer){
        killTimer(evt->timerId());
        if(!m_user){
            m_model->loadPageForRowId(m_rowId);
        }
    }else if(evt->timerId()==m_animTimer){
        m_placeholderView->setOpacity(placeholderOpacity());
        if(!shouldAnimate())
            killTimer(evt->timerId());
    }
}

bool STUserListItemView::shouldAnimate(){
    return QDateTime::currentMSecsSinceEpoch()<(m_animStartTime+m_animDuration);
}

float STUserListItemView::placeholderOpacity(){
    float per=(float)(QDateTime::currentMSecsSinceEpoch()-m_animStartTime)/(float)m_animDuration;
    if(per>1.f)
        per=1.f;
    if(per<0.f)
        per=0.f;
    return 1.f-per;
}

void STUserListItemView::showUser(){
    STObjectManager::EntityRange rng;
    if(m_user){
        rng.entityType="user_stobject";
        rng.stObject=m_user;
    }else{
        rng.entityType="user_id";
        rng.entity=m_userId;
    }

    STObjectManager::LinkActivationInfo info;
    emit linkActivated(&rng,info);
}
