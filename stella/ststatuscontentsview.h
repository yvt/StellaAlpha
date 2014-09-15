#ifndef STSTATUSCONTENTSVIEW_H
#define STSTATUSCONTENTSVIEW_H

#include "ststdafx.h"
#include "stscrolledpane.h"
#include "stobjectmanager.h"
#include "stactionmanager.h"

class STButtonView;
class STUserProfileNumberView;
class STLinkedLabelView;
class STStatusPane;
class STTweetView;
class STRemoteImageView;
class STPlaceView;

class STStatusContentsView : public STScrolledPane
{
    Q_OBJECT

    STObjectManager::StatusRef m_status;

    STLinkedLabelView *m_textView;
    STLinkedLabelView *m_dateView;

    QList<STRemoteImageView *> m_imageViews;

    STUserProfileNumberView *m_retweetsView;
    STButtonView *m_replyButton;
    STButtonView *m_retweetButton;
    STButtonView *m_favoriteButton;
    STButtonView *m_optionsButton;

    STPlaceView *m_placeView;

    bool m_favorited;
    int m_lastWidth;

    STAccount *account();
    STTweetView *tweetView();
    STBaseTabPageView *baseTabPageView();

public:
    explicit STStatusContentsView(STStatusPane *parent = 0);
    
    virtual void relayoutContents();

    void setStatus(STObjectManager::StatusRef);

signals:
    
private slots:
    void statusActionFailed(quint64 statusId, quint64 accountId,
                            STActionManager::StatusAction);
    void statusActionDone(STObjectManager::StatusRef, quint64 accountId,
                          STActionManager::StatusAction);
    void reply();
    void showRetweetMenu();
    void toggleFavorite();
    void showOptionsMenu();
    void viewRetweets();

    void thumbnailClicked();
};

#endif // STSTATUSCONTENTSVIEW_H
