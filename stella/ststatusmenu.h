#ifndef STSTATUSMENU_H
#define STSTATUSMENU_H

#include "ststdafx.h"
#include <QMenu>
#include "stobjectmanager.h"
#include <QAction>

class STBaseTabPageView;
class STTweetView;

class STStatusMenu : public QMenu
{
    Q_OBJECT


public:

    enum MenuType{
        Retweet,
        Options,
        OptionsWithoutViewDetail
    };

    explicit STStatusMenu(STBaseTabPageView *tabPage,
                          STObjectManager::StatusRef status,
                          MenuType type);
    

private:
    quint64 m_accountId;
    STAccount *m_account;
    STBaseTabPageView *m_tabPage;
    STObjectManager::StatusRef m_status;


    STTweetView *tweetView();

signals:
    
private slots:
    void retweet();
    void undoRetweet();
    void quoteRetweet();
    void viewRetweets();
    void viewConversation();
    void viewDetail();
    void deleteTweet();
    void copyTweet();
    void copyLinkToTweet();
    void viewTweetInWebsite();
};

#endif // STSTATUSMENU_H
