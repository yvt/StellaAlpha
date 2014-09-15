#ifndef STACCOUNTITEMVIEW_H
#define STACCOUNTITEMVIEW_H

#include "ststdafx.h"
#include <QGraphicsObject>
#include "stobjectmanager.h"
#include "stfont.h"

class STAccount;
class STUserIconView;
class STButtonView;

class STAccountItemView : public QGraphicsObject
{
    Q_OBJECT

    QSize m_size;
    quint64 m_accountId;
    STObjectManager::UserRef m_user;
    STUserIconView*m_iconView;
    STButtonView *m_buttonView;

public:
    explicit STAccountItemView(STAccount *account, QSize, QGraphicsItem *parent = 0);
    
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual QRectF boundingRect() const;

    static STFont::Layout layoutItem(STObjectManager::UserRef user);
    static QSize estimateSize(STObjectManager::UserRef user, int itemWidth);

    quint64 accountId() const{return m_accountId;}

signals:
    void triggered();
    void dragInitated(quint64, QPointF);
private slots:
    void buttonActivated();
    void buttonDragged(QPointF);
public slots:
    void accountSelected(quint64);
};

#endif // STACCOUNTITEMVIEW_H
