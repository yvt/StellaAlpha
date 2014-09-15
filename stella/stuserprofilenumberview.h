#ifndef STUSERPROFILENUMBERVIEW_H
#define STUSERPROFILENUMBERVIEW_H

#include "ststdafx.h"
#include "stbuttonview.h"

class STUserProfileNumberView : public STButtonView
{
    Q_OBJECT

    QString m_labelText;
    QString m_numberText;
public:
    explicit STUserProfileNumberView(QGraphicsItem *parent = 0);
    
    void setLabelText(QString);
    void setNumberText(QString);

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

signals:

public slots:
    
};

#endif // STUSERPROFILENUMBERVIEW_H
