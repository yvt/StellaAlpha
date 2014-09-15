#ifndef STUSERPROFILEVIEW_H
#define STUSERPROFILEVIEW_H

#include <QGraphicsObject>
#include "stscrolledpane.h"
#include "stobjectmanager.h"
#include "stlinkedtextmanager.h"

class STButtonView;
class STUserProfileNumberView;
class STUserPane;
class STLinkedLabelView;

class STUserProfileView : public STScrolledPane
{
    Q_OBJECT

    STObjectManager::UserRef m_user;

    STUserProfileNumberView *m_followersView;
    STUserProfileNumberView *m_friendsView;
    STUserProfileNumberView *m_tweetsView;
    STUserProfileNumberView *m_favoritesView;
    STUserProfileNumberView *m_listedView;

    STLinkedLabelView *m_bioLabel;
    STLinkedLabelView *m_bioText;

    STLinkedLabelView *m_urlLabel;
    STLinkedLabelView *m_urlText;

    STLinkedLabelView *m_locLabel;
    STLinkedLabelView *m_locText;

    STLinkedLabelView *m_sinceLabel;
    STLinkedLabelView *m_sinceText;

    int m_lastWidth;

public:
    explicit STUserProfileView(STUserPane *parent = 0);

    virtual void relayoutContents();

    void setUser(STObjectManager::UserRef);

private slots:
    void showFollowers();
    void showFriends();
    void showAddedLists();
};

#endif // STUSERPROFILEVIEW_H
