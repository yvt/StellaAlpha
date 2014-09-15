#include "ststatuspane.h"
#include "stusericonview.h"
#include "ststatususernameview.h"
#include "ststatuscontentsview.h"

STStatusPane::STStatusPane(quint64 statusID, STAccount *account, QGraphicsItem *parent) :
    STPane(parent), m_account(account),m_statusId(statusID)
{

    m_contentsView=new STStatusContentsView(this);

    m_nameView=new STStatusUserNameView(contentsView());
    connect(m_nameView, SIGNAL(linkActivated(STObjectManager::EntityRange*,
                                             STObjectManager::LinkActivationInfo)),
            this, SLOT(linkActivate(STObjectManager::EntityRange*,
                                    STObjectManager::LinkActivationInfo)));

    m_iconView=new STUserIconView(STObjectManager::UserRef(), contentsView());
    m_iconView->setSize(QSize(36,36));
    m_iconView->setAcceptedMouseButtons(Qt::LeftButton);
    m_iconView->setCursor(Qt::PointingHandCursor);
    connect(m_iconView, SIGNAL(clicked()),
            this, SLOT(showUser()));

}

void STStatusPane::relayoutContents(){
    if(size().isEmpty())
        return;

    m_nameView->setWidth(size().width());
    m_iconView->setPos(4,4);

    m_contentsView->setPos(0,m_nameView->boxHeight());
    m_contentsView->setSize(QSize(contentsWidth(),
                            size().height()-m_nameView->boxHeight()));
}

void STStatusPane::setStatus(STObjectManager::StatusRef status, bool animated){
    if(status->retweetedStatus){
        status=status->retweetedStatus;

    }
    m_status=status;
    m_statusId=status->id;
    m_nameView->setUser(status->user);
    m_iconView->setUser(status->user);
    m_contentsView->setStatus(status);
}

STHeaderView::Contents STStatusPane::headerViewContents(){
    STHeaderView::Contents cont=STPane::headerViewContents();
    cont.title=tr("Tweet Details");
    return cont;
}


void STStatusPane::showUser(){
    STObjectManager::EntityRange rng;
    rng.entityType="user_stobject";
    rng.stObject=m_status->user;

    STObjectManager::LinkActivationInfo info;
    this->linkActivate(&rng,info);
}
