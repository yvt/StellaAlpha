#include "stusericonview.h"
#include <QPainter>
#include <QMutexLocker>
#include "stsimpleanimator.h"
#include <QGraphicsSceneMouseEvent>

STUserIconView::STUserIconView(STObjectManager::UserRef user, QGraphicsItem *parent) :
    STRemoteImageView(parent)
{
    if(user){
        setUser(user);
    }


}


STUserIconView::STUserIconView( QGraphicsItem *parent) :
    STRemoteImageView(parent)
{

}


void STUserIconView::setUser(STObjectManager::UserRef user){
    setTicket(STUserIconManager::sharedManager()->requestUserIcon(user));
}
