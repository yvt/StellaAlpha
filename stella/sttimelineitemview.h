#ifndef STTIMELINEITEMVIEW_H
#define STTIMELINEITEMVIEW_H

#include <QGraphicsObject>
#include "stobjectmanager.h"
#include "stfont.h"
#include "sttimelineitemtimeview.h"
#include <QGraphicsPixmapItem>
#include "stlinkedtextmanager.h"
#include "stactionmanager.h"
#include <QAction>
#include "stsimpleanimator.h"

class STUserIconView;
class STClearButtonView;
class STTweetView;
class STBaseTabPageView;
class STRemoteImageView;

class STTimelineItemView : public QGraphicsObject
{
    Q_OBJECT

    QSize m_size;
    STObjectManager::StatusRef m_status;
    quint64 m_accountId;

    STLinkedTextManager m_userNameLinkManager;
    STLinkedTextManager m_userScreenNameLinkManager;
    STLinkedTextManager m_tweetTextLinkManager;
    STLinkedTextManager m_retweetUserLinkManager;

   STUserIconView *m_iconView;
    STRemoteImageView *m_imageView;
    STTimelineItemTimeView *m_timeView;

    STSimpleAnimator *m_buttonsAnimator;

    QGraphicsPixmapItem *m_buttonsBgItem;

    STClearButtonView *m_replyButton;
    STClearButtonView *m_retweetButton;
    STClearButtonView *m_favoriteButton;
    STClearButtonView *m_optionsButton;


    bool m_favorited;

    void updateCursor();
    void updateButton();
    STTweetView *tweetView();
    STBaseTabPageView *baseTabPageView();

public:
    explicit STTimelineItemView(const QSize& sz, STObjectManager::StatusRef status,quint64 accountId, QGraphicsItem *parent = 0);
    
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual QRectF boundingRect() const;

    static QSize estimateSize(int width, STObjectManager::StatusRef);
    static STFont::Layout layoutStatusText(int width, STObjectManager::StatusRef status);
    static STObjectManager::EntityRange entityRangeForThumbnailForStatus(STObjectManager::StatusRef);
    static bool statusNeedsThumbnail(STObjectManager::StatusRef);

    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    virtual void hoverMoveEvent(QGraphicsSceneHoverEvent *event);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

signals:

    void linkActivated(STObjectManager::EntityRange *,
                       STObjectManager::LinkActivationInfo);

    
private slots:
    void statusActionDone(STObjectManager::StatusRef,
                          quint64 accountId,
                          STActionManager::StatusAction);
    void statusActionFailed(quint64 statusId,
                            quint64 accountId,
                            STActionManager::StatusAction);

    void reply();
    void showRetweetMenu();
    void toggleFavorite();

    void showOptionsMenu();

    void showUser();

    void showThumbnailedImage();

    void setButtonsOpacity(float);
};

#endif // STTIMELINEITEMVIEW_H
