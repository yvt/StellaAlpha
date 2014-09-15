#ifndef STUSERICONVIEW_H
#define STUSERICONVIEW_H

#include "stobjectmanager.h"
#include "stusericonmanager.h"
#include "stremoteimageview.h"

class STSimpleAnimator;

class STUserIconView : public STRemoteImageView
{
    Q_OBJECT

    QSize m_size;
public:
    explicit STUserIconView(STObjectManager::UserRef user, QGraphicsItem *parent = 0);
    explicit STUserIconView(QGraphicsItem *parent = 0);

    void setUser(STObjectManager::UserRef user);

};

#endif // STUSERICONVIEW_H
