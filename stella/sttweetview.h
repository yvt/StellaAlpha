#ifndef STTWEETVIEW_H
#define STTWEETVIEW_H

#include <QGraphicsObject>
#include <QGraphicsPixmapItem>
#include "stobjectmanager.h"
#include "stlinkedtextmanager.h"

class STSimpleEditView;
class STTweetShadowView;
class STTweetNonEditingView;
class STButtonView;
class STStripeAnimationView;
class STTweetViewNetwork;
class STAccount;
class STTweetView : public QGraphicsObject
{
    Q_OBJECT

    STTweetViewNetwork *m_network;
    STAccount *m_account;

    STTweetShadowView *m_shadow;
    STTweetNonEditingView *m_cursorItem;
    STButtonView *m_tweetButton;
    STStripeAnimationView *m_progressView;
    QGraphicsPixmapItem *m_errorView;
    QGraphicsPixmapItem *m_replyInfoView;
    STLinkedTextManager m_replyInfoLinkManager;

    int m_width;
    int m_editorHeight;
    int m_controlsHeight;
    int m_nonEditingLabelHeight;

    int m_charCount;

    STSimpleEditView *m_editor;
    STObjectManager::StatusRef m_replyTarget;

    quint64 m_animStartTime;
    float m_oldOpenState;
    float m_newOpenState;
    unsigned int m_animDuration;

    quint64 m_progressOpacityAnimStartTime;
    float m_oldProgressOpacity;
    float m_newProgressOpacity;
    unsigned int m_progressOpacityAnimDuration;

    quint64 m_errorOpacityAnimStartTime;
    float m_oldErrorOpacity;
    float m_newErrorOpacity;
    unsigned int m_errorOpacityAnimDuration;

    quint64 m_replyInfoAnimStartTime;
    float m_oldReplyInfoOpacity;
    float m_newReplyInfoOpacity;
    unsigned int m_replyInfoAnimDuration;

    int m_timer;

    float openState() const;
    float progressOpacity() const;
    float errorOpacity() const;
    float replyInfoOpacity() const;

    bool shouldAnimate() const;
    void startAnimate();

    bool isMentioning(const QString& screenName);

    void drawReplyInfo();

public:
    explicit STTweetView(QGraphicsItem *parent = 0);
    
    void relayout();

    void setWidth(int);
    int currentHeight() const;

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual QRectF boundingRect() const;

    virtual void keyPressEvent(QKeyEvent *event);

    void setAccount(STAccount *ac){m_account=ac;}

    virtual void timerEvent(QTimerEvent *);

    void setOpenStateAnimated(float v);
    void setProgressOpacityAnimated(float v);
    void setErrorOpacityAnimated(float v);
    void setReplyInfoOpacity(float);

    void updateState();

    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    virtual void hoverMoveEvent(QGraphicsSceneHoverEvent *event);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

signals:
    
    void currentHeightChanged();
    void linkActivated(STObjectManager::EntityRange *,
                       STObjectManager::LinkActivationInfo);

public slots:
    virtual void focusInEvent(QFocusEvent *event);
    virtual void focusOutEvent(QFocusEvent *event);



    void updateCharCount();
    void updateReplyInfoOpacity();

    void tweet();

    void reply(STObjectManager::StatusRef, bool additive=false);
    void quoteRetweet(STObjectManager::StatusRef);

private slots:
    void tweetSent();
    void tweetError(QString);
};

#endif // STTWEETVIEW_H
