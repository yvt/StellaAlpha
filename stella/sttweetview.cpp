#include "sttweetview.h"
#include "stsimpleeditview.h"
#include <QPainter>
#include <QBrush>
#include <QLinearGradient>
#include "sttweetshadowview.h"
#include "stfont.h"
#include "sttweetnoneditingview.h"
#include <QCursor>
#include <QDateTime>
#include "stmath.h"
#include "stbuttonview.h"
#include <QDebug>
#include "sttwittertext.h"
#include "ststripeanimationview.h"
#include "sttweetviewnetwork.h"
#include <QKeyEvent>
#include <QRegExp>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneHoverEvent>
#include <QPixmapCache>
#include "staccount.h"

STTweetView::STTweetView(QGraphicsItem *parent) :
    QGraphicsObject(parent)
{
    m_network=new STTweetViewNetwork(this);

    // this item is used to set cursor when not editing and for hover effect
    m_errorView=new QGraphicsPixmapItem(this);
    m_errorView->setOpacity(0.f);

    m_cursorItem=new STTweetNonEditingView(this);
    m_cursorItem->setCursor(Qt::IBeamCursor);

    m_progressView=new STStripeAnimationView(this);
    m_progressView->setOpacity(0.f);

    m_width=0;
    m_editor=new STSimpleEditView(this);
    m_editor->setOpacity(0.01f);
    m_editor->setPos(0.f, 100.f);

    m_replyInfoView=new QGraphicsPixmapItem(this);
    m_replyInfoView->setAcceptedMouseButtons(Qt::NoButton);

    m_tweetButton=new STButtonView(this);
    m_tweetButton->setText(tr("Tweet"));

    m_editorHeight=80;
    m_controlsHeight=22;
    m_nonEditingLabelHeight=20;

    m_animDuration=200;
    m_newOpenState=0.f;
    m_animStartTime=0;
    m_progressOpacityAnimStartTime=0;
    m_newProgressOpacity=0.f;
    m_progressOpacityAnimDuration=200;
    m_errorOpacityAnimStartTime=0;
    m_newErrorOpacity=0.f;
    m_errorOpacityAnimDuration=200;
    m_replyInfoAnimStartTime=0;
    m_replyInfoAnimDuration=200;
    m_newReplyInfoOpacity=0.f;

    m_timer=0;


    m_charCount=0;

    m_shadow=new STTweetShadowView(this);

    setFlag(ItemIsFocusable);
    setCacheMode(DeviceCoordinateCache);
    setAcceptsHoverEvents(true);

    this->setFocusProxy(m_editor);
    connect(m_editor, SIGNAL(focusOut(QFocusEvent*)),
            this, SLOT(focusOutEvent(QFocusEvent*)));
    connect(m_editor, SIGNAL(focusIn(QFocusEvent*)),
            this, SLOT(focusInEvent(QFocusEvent*)));

    connect(m_tweetButton, SIGNAL(activated()),
            this, SLOT(tweet()));

    connect(m_editor ,SIGNAL(textChanged()),
            this, SLOT(updateCharCount()));
    connect(m_editor ,SIGNAL(textChanged()),
            this, SLOT(updateReplyInfoOpacity()));

    connect(m_network ,SIGNAL(tweetSent()),
            this, SLOT(tweetSent()));
    connect(m_network, SIGNAL(tweetErrorSending(QString)),
            this, SLOT(tweetError(QString)));
}

void STTweetView::relayout(){
    m_editor->setSize(QSize(m_width, m_editorHeight-1));
    m_cursorItem->setSize(QSize(m_width, m_nonEditingLabelHeight));
    m_progressView->setSize(QSize(m_width, m_nonEditingLabelHeight));
    m_shadow->setWidth(m_width);
    m_tweetButton->setSize(QSize(80, m_controlsHeight+1));
    m_tweetButton->setPos(QPointF(m_width-80+1, m_editorHeight));
    m_replyInfoView->setPos(QPointF(0, m_editorHeight));
    update();
}

void STTweetView::setWidth(int width){
    if(width==m_width)return;
    prepareGeometryChange();
    m_width=width;

    relayout();
}

void STTweetView::drawReplyInfo(){
    if(!m_replyTarget)
        return;
    QMutexLocker locker(&(m_replyTarget->mutex));

    QRect boundRect(0,0,m_width-120, m_controlsHeight);
    QImage image(boundRect.size(), QImage::Format_RGB32);
    QPainter painter(&image);
    QLinearGradient grad(0.f, boundRect.top(), 0.f, boundRect.bottom());
    grad.setColorAt(0.f, QColor(110, 110, 110));
    grad.setColorAt(1.f, QColor(100, 100, 100));
    //grad.setColorAt(1.f, QColor(0,0,0));
    painter.fillRect(QRect(0, boundRect.top()+1, boundRect.width(), boundRect.height()-1), QBrush(grad));
    painter.fillRect(QRect(0, boundRect.top(), boundRect.width(), 1),
                      QColor(60,60,60));
    painter.fillRect(QRect(0, boundRect.bottom(), boundRect.width(), 1),
                      QColor(255,255,255,10));

    static QPixmapCache::Key replyIconKey;
    QPixmap replyIcon;
    if(!QPixmapCache::find(replyIconKey, &replyIcon)){
        replyIcon.load(":/stella/res/ReplyTweetIcon.png");
        replyIconKey=QPixmapCache::insert(replyIcon);
    }
    painter.drawPixmap(2,4,replyIcon);

    float tweetTextLeft=20.f;

    if(m_replyInfoLinkManager.isEmpty()){
        QString text;
        QList<STObjectManager::EntityRange> ranges;

        {
            QString fragment="@"+m_replyTarget->user->data["screen_name"].toString();
            STObjectManager::EntityRange range;
            range.charIndexStart=text.length();
            range.charIndexEnd=text.length()+fragment.length();
            range.entityType="user_stobject";
            range.stObject=m_replyTarget->user;
            ranges.push_back(range);
            text+=fragment;
        }
        text+=": ";
        {
            QString fragment=m_replyTarget->data["text"].toString();
            STObjectManager::EntityRange range;
            range.charIndexStart=text.length();
            range.charIndexEnd=text.length()+fragment.length();
            range.entityType="status_stobject";
            range.stObject=m_replyTarget;
            ranges.push_back(range);
            text+=fragment;
        }

        STFont::Layout layout=STFont::defaultFont()->layoutString(text, (float)boundRect.width()-tweetTextLeft, true);
        m_replyInfoLinkManager.init(layout, ranges, STFont::defaultFont());
        m_replyInfoLinkManager.setPos(QPointF(tweetTextLeft, (boundRect.height()-12)/2+1));
    }

    m_replyInfoLinkManager.draw(image, painter, QColor(220,220,220), QColor(220,220,220),
                                QPointF(0, -1), QColor(0,0,0,40));

    m_replyInfoView->setPixmap(QPixmap::fromImage(image));

    if(m_replyInfoLinkManager.hotEntityRange())
        setCursor(Qt::PointingHandCursor);
    else
        setCursor(Qt::ArrowCursor);

}

void STTweetView::paint(QPainter *outPainter, const QStyleOptionGraphicsItem *, QWidget *){
    QRectF boundRect(0,0,m_width, m_editorHeight+m_controlsHeight);
    QImage image(boundRect.width(),boundRect.height(),QImage::Format_RGB32);

    image.fill(0);

    QPainter painter(&image);

    QRect controlsRect(0, m_editorHeight, m_width, m_controlsHeight);
    QLinearGradient grad(0.f, controlsRect.top(), 0.f, controlsRect.bottom());
    grad.setColorAt(0.f, QColor(110, 110, 110));
    grad.setColorAt(1.f, QColor(100, 100, 100));
    //grad.setColorAt(1.f, QColor(0,0,0));
    painter.fillRect(QRect(0, controlsRect.top()+1, controlsRect.width(), controlsRect.height()-1), QBrush(grad));
    painter.fillRect(QRect(0, controlsRect.top(), controlsRect.width(), 1),
                      QColor(60,60,60));
    painter.fillRect(QRect(0, controlsRect.bottom(), controlsRect.width(), 1),
                      QColor(255,255,255,10));

    QPointF charCountPos(controlsRect.width()-85, controlsRect.top()+(controlsRect.height()-12)/2+1);
    int leftCount=140-m_charCount;
    {
        QString str=QString::number(leftCount);
        STFont::Layout layout=STFont::defaultFont()->layoutString(str);
        float wid=STFont::defaultFont()->boundingRectForLayout(layout).right();
        charCountPos.setX(charCountPos.x()-wid);
        STFont::defaultFont()->drawLayout(image, charCountPos-QPointF(0,1), QColor(0,0,0,64),
                                          layout, STFont::DrawOption());
        STFont::defaultFont()->drawLayout(image, charCountPos,(leftCount<0)?QColor(255,88,64):
                                                                            (leftCount<32)?QColor(240,220,64):
                                                                                QColor(240,240,240),
                                          layout, STFont::DrawOption());
    }

    QRect editorRect(0, 0, m_width, m_editorHeight);
    painter.fillRect(editorRect, QColor(90, 90, 90));


    STFont::defaultFont()->drawString(image, QPointF(5.f, (m_nonEditingLabelHeight-12)/2+1),
                                      QColor(255,255,255,100),
                                      tr("What's happening?"));

    outPainter->drawImage(0,0,image);

}

QRectF STTweetView::boundingRect() const{
    return QRectF(0.f, 0.f, m_width, m_editorHeight+m_controlsHeight);
}

int STTweetView::currentHeight() const{
    return m_nonEditingLabelHeight+(int)((float)(m_editorHeight+m_controlsHeight-m_nonEditingLabelHeight)*
                                         openState());
}

void STTweetView::focusInEvent(QFocusEvent *){
    updateState();
    updateCharCount();
}

void STTweetView::focusOutEvent(QFocusEvent *){
    updateState();
    if(m_editor->text().isEmpty()){
        m_replyTarget=STObjectManager::StatusRef();
        setReplyInfoOpacity(0.f);
    }
}

float STTweetView::openState() const{
    float per=(float)(QDateTime::currentMSecsSinceEpoch()-m_animStartTime)/(float)m_animDuration;
    if(per<0.f)per=0.f;
    if(per>1.f)per=1.f;
    per= STSmoothStep(per);
    return m_oldOpenState*(1.f-per)+m_newOpenState*per;
}

float STTweetView::progressOpacity() const{
    float per=(float)(QDateTime::currentMSecsSinceEpoch()-m_progressOpacityAnimStartTime)/(float)m_progressOpacityAnimDuration;
    if(per<0.f)per=0.f;
    if(per>1.f)per=1.f;
    return m_oldProgressOpacity*(1.f-per)+m_newProgressOpacity*per;
}

float STTweetView::errorOpacity() const{
    float per=(float)(QDateTime::currentMSecsSinceEpoch()-m_errorOpacityAnimStartTime)/(float)m_errorOpacityAnimDuration;
    if(per<0.f)per=0.f;
    if(per>1.f)per=1.f;
    return m_oldErrorOpacity*(1.f-per)+m_newErrorOpacity*per;
}

float STTweetView::replyInfoOpacity() const{
    float per=(float)(QDateTime::currentMSecsSinceEpoch()-m_replyInfoAnimStartTime)/(float)m_replyInfoAnimDuration;
    if(per<0.f)per=0.f;
    if(per>1.f)per=1.f;
    return m_oldReplyInfoOpacity*(1.f-per)+m_newReplyInfoOpacity*per;
}

void STTweetView::setOpenStateAnimated(float v){
    if(v==m_newOpenState)return;
    m_oldOpenState=openState();
    m_newOpenState=v;
    m_animStartTime=QDateTime::currentMSecsSinceEpoch();
    startAnimate();;
}

void STTweetView::setProgressOpacityAnimated(float v){
    if(v==m_newProgressOpacity)return;
    m_oldProgressOpacity=progressOpacity();
    m_newProgressOpacity=v;
    m_progressOpacityAnimStartTime=QDateTime::currentMSecsSinceEpoch();
    startAnimate();;
}

void STTweetView::setErrorOpacityAnimated(float v){
    if(v==m_newErrorOpacity)return;
    m_oldErrorOpacity=errorOpacity();
    m_newErrorOpacity=v;
    m_errorOpacityAnimStartTime=QDateTime::currentMSecsSinceEpoch();
    startAnimate();;
}

void STTweetView::setReplyInfoOpacity(float v){
    if(v==m_newReplyInfoOpacity)
        return;
    m_oldReplyInfoOpacity=replyInfoOpacity();
    m_newReplyInfoOpacity=v;
    m_replyInfoAnimStartTime=QDateTime::currentMSecsSinceEpoch();
    startAnimate();
}

void STTweetView::updateState(){
    bool opened=this->hasFocus()||(m_editor->text().length()>0&&m_newErrorOpacity<.1f);
    bool showProgress=false;
    if(opened){
        // close error, if user clicked the tweet view.
        setErrorOpacityAnimated(0.f);
    }
    if(m_network->isSending()){
        showProgress=true;
        opened=false;
    }

    setProgressOpacityAnimated(showProgress?1.f:0.f);
    setOpenStateAnimated(opened?1.f:0.f);
}

void STTweetView::tweetSent(){
    if(this->hasFocus()){
        this->clearFocus();
        this->parentItem()->setFocus();
    }
    Q_ASSERT(!hasFocus());
    m_editor->setText("");
    setReplyInfoOpacity(0.f);
    m_replyTarget=STObjectManager::StatusRef();
    updateState();
}

void STTweetView::tweetError(QString msg){
    // draw error image.

    QRectF boundRect(0,0,m_width, m_nonEditingLabelHeight);
    QImage image(boundRect.width(),boundRect.height(),QImage::Format_RGB32);

    image.fill(QColor(100, 10, 20));;


    STFont::defaultFont()->drawString(image, QPointF(5.f, (m_nonEditingLabelHeight-12)/2+2),
                                      QColor(0,0,0,60),
                                      msg);
    STFont::defaultFont()->drawString(image, QPointF(5.f, (m_nonEditingLabelHeight-12)/2+1),
                                      QColor(255,255,255,255),
                                      msg);


    this->m_errorView->setPixmap(QPixmap::fromImage(image));
    setErrorOpacityAnimated(1.f);

    if(this->hasFocus()){
        this->clearFocus();
        this->parentItem()->setFocus();
    }
    Q_ASSERT(!hasFocus());
    updateState();

}

bool STTweetView::shouldAnimate() const{
    return (quint64)QDateTime::currentMSecsSinceEpoch()<(m_animStartTime+m_animDuration) ||
           (quint64)QDateTime::currentMSecsSinceEpoch()<(m_progressOpacityAnimStartTime+m_progressOpacityAnimDuration) ||
            (quint64)QDateTime::currentMSecsSinceEpoch()<(m_errorOpacityAnimStartTime+m_errorOpacityAnimDuration)||
            (quint64)QDateTime::currentMSecsSinceEpoch()<(m_replyInfoAnimStartTime+m_replyInfoAnimDuration);
}

void STTweetView::startAnimate(){
    if(m_timer) return;
    if(!shouldAnimate())return;

    // TODO: progress, and error animation.
    m_timer=this->startTimer(15);
}

void STTweetView::timerEvent(QTimerEvent *){

    m_editor->setOpacity(openState());
    if(openState()<.01f){
        m_editor->setPos(0.f, 100.f);
        m_editor->setOpacity(.01f);
    }else{
        m_editor->setOpacity(openState());
        m_editor->setPos(0.f, 1.f);
    }
    m_progressView->setOpacity(progressOpacity());
    m_errorView->setOpacity(errorOpacity());
    m_replyInfoView->setOpacity(replyInfoOpacity());
    emit currentHeightChanged();

    if(!shouldAnimate()){
        this->killTimer(m_timer);
        m_timer=0;
    }
}

void STTweetView::updateReplyInfoOpacity(){
    float opacity=0.f;
    if(m_replyTarget){
        QString screenName;
        {
            QMutexLocker locker(&(m_replyTarget->mutex));
            screenName=m_replyTarget->user->data["screen_name"].toString();
        }

        if(isMentioning(screenName)){
            opacity=1.f;
        }else{
            opacity=0.5f;
        }
    }

    setReplyInfoOpacity(opacity);
}

void STTweetView::updateCharCount(){
    int cnt=STTwitterText::sharedInstance()->lengthOfTweet(m_editor->text());
    if(cnt==m_charCount)return;
    m_charCount=cnt;
    update();
}

void STTweetView::tweet(){
    if(m_network->isSending())
        return;
    if(!this->hasFocus()) // stange... but happen
        return;

    // TODO: Reply

    m_network->sendTweet(m_editor->text(),
                         m_account,
                         m_replyTarget);

    updateState();

    /*
    QImage img(1024, 256, QImage::Format_RGB32);
    quint64 ot=QDateTime::currentMSecsSinceEpoch();
    QString str="ｵﾙｩｱｱｱｱｱｱｱｱｱｱｱｱｱｱｱｱｱｱｱｱｱｱﾜﾚｪwwwwwwwwwww";
    int count=0;
    while(QDateTime::currentMSecsSinceEpoch()<ot+1000){
        STFont::defaultFont()->drawString(img, QPointF(0,128), QColor(255,255,255),
                                          str);
        count++;
    }
    qDebug()<<count<<" draws/sec, "<<str.length()<<" chars";*/
}

void STTweetView::keyPressEvent(QKeyEvent *event){
    if(event->key()==Qt::Key_Enter ||
        event->key()==Qt::Key_Return){
        if(event->modifiers()&&Qt::ControlModifier){

            tweet();

            return;
        }
    }else if(event->key()==Qt::Key_Escape){
        if(m_replyTarget){
            m_replyTarget=STObjectManager::StatusRef();
            setReplyInfoOpacity(0.f);
            return;
        }else{
            m_editor->setText("");
        }
        this->clearFocus();
        this->parentItem()->setFocus();
    }
    event->ignore();
}

bool STTweetView::isMentioning(const QString &screenName){
    QStringList list=STTwitterText::sharedInstance()->extractMentions(m_editor->text());
    foreach(const QString& str, list){
        if(!str.compare(screenName, Qt::CaseInsensitive))
            return true;
    }
    return false;
}
/*
static QString removeQuoteRetweetText(QString text){
    QRegExp quoteRTRegExp("[ 　]?(RT|DT) ([ 　]*[@＠][a-zA-Z0-9_]{1,20}):.*");
    int pos=quoteRTRegExp.indexIn(text);
    if(pos==-1)
        return text;
    else
        return text.left(pos);
}
*/
void STTweetView::reply(STObjectManager::StatusRef target, bool additive){
    m_replyTarget=target;
    this->setFocus();

    m_replyInfoLinkManager.clear();
    drawReplyInfo();
    setReplyInfoOpacity(1.f);

    QRegExp repliesRegExp("(([ 　.,]*[@＠][a-zA-Z0-9_]{1,20})*[ 　]*).*");

    // Tweet author
    QString screenName=target->user->data["screen_name"].toString();

    // check reply target's entities, find out the current account's screen name
    QString curAccountScreenName;
    quint64 curAccountId=m_account->userId();
    foreach(const STObjectManager::EntityRange& er, target->entityRanges){
        if(er.entityType=="user_mentions"){
            QVariantMap map=er.entity.toMap();
            if(map["id"].toULongLong()==curAccountId){
                curAccountScreenName=map["screen_name"].toString();
            }
        }
    }

    // mentioned users as reply
    QStringList screenNames;
    screenNames.append(screenName);
    if(repliesRegExp.exactMatch(target->data["text"].toString())){
        QStringList lst=STTwitterText::sharedInstance()->extractMentions(repliesRegExp.cap(1));
        foreach(QString s, lst){
            if(!s.compare(curAccountScreenName, Qt::CaseInsensitive))
                continue;
            screenNames<<s;
        }
    }

    // remove duplicate
    QStringList screenNames2;
    QSet<QString> screenNamesSet;
    foreach(QString str, screenNames){
        if(screenNamesSet.contains(str))
            continue;
        screenNames2.append(str);
        screenNamesSet.insert(str);
    }

    int repRange=0;
    QString text=m_editor->text();

    if(!additive){


        if(repliesRegExp.exactMatch(text)){
            repRange=repliesRegExp.cap(1).length();
        }
    }else{
        // remove already written user names
        QStringList writtenScreenNames;
        if(repliesRegExp.exactMatch(text)){
            writtenScreenNames=STTwitterText::sharedInstance()->extractMentions(repliesRegExp.cap(1));
        }

        foreach(QString scr, writtenScreenNames){
            for(int i=0;i<screenNames2.count();i++){
                if(!scr.compare(screenNames2[i], Qt::CaseInsensitive)){
                    screenNames2.removeAt(i);
                    break;
                }
            }
        }
    }

    QString reps;
    foreach(QString scr, screenNames2){
        reps+="@";
        reps+=scr;
        reps+=" ";
    }

    if(repRange!=0 || !reps.isEmpty())
    m_editor->insert(reps, 0, repRange);

    if(!additive){
        // select mentions generated from reply target mentions
        m_editor->setSelection(screenNames2[0].length()+2, reps.length());

    }

    updateCharCount();

    //



}

void STTweetView::quoteRetweet(STObjectManager::StatusRef target){
    m_replyTarget=target;
    this->setFocus();

    m_replyInfoLinkManager.clear();
    drawReplyInfo();
    setReplyInfoOpacity(1.f);

    QString screenName=target->user->data["screen_name"].toString();

    QRegExp quoteRTRegExp("[ 　]?(RT|DT) ([ 　]*[@＠][a-zA-Z0-9_]{1,20}):.*");
    QString text=m_editor->text();
    int pos=quoteRTRegExp.indexIn(text);
    int insPos=text.length();
    int insRange=0;
    if(pos!=-1){
        // remove old quote RT
        insPos=pos;
        insRange=text.length()-insPos;
    }

    QString quoteText=" RT @%1: %2";
    quoteText=quoteText.arg(screenName, target->data["text"].toString());

    m_editor->insert(quoteText, insPos, insRange);
    m_editor->setSelection(insPos, insPos);

    updateCharCount();


}

void STTweetView::hoverLeaveEvent(QGraphicsSceneHoverEvent *){
    if(m_replyInfoLinkManager.mouseLeave()){
        drawReplyInfo();
    }
}

void STTweetView::mousePressEvent(QGraphicsSceneMouseEvent *event){
    if(m_replyInfoLinkManager.mouseDown(event->pos()-m_replyInfoView->pos())){
        drawReplyInfo();
    }
}

void STTweetView::hoverMoveEvent(QGraphicsSceneHoverEvent *event){
    if(m_replyInfoLinkManager.mouseMove(event->pos()-m_replyInfoView->pos())){
        drawReplyInfo();
    }
}

void STTweetView::mouseReleaseEvent(QGraphicsSceneMouseEvent *event){
    if(m_replyInfoLinkManager.mouseMove(event->pos()-m_replyInfoView->pos())){
        drawReplyInfo();
    }
    if(m_replyInfoLinkManager.mouseUp()){
        STObjectManager::LinkActivationInfo info;
        emit linkActivated(m_replyInfoLinkManager.hotEntityRange(), info);
    }
}

