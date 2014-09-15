#ifndef STAUTHORIZECANCELBUTTON_H
#define STAUTHORIZECANCELBUTTON_H

#include <QPushButton>

class STAuthorizeCancelButton : public QPushButton
{
    Q_OBJECT
public:
    explicit STAuthorizeCancelButton(QWidget *parent = 0);
    
    virtual void paintEvent(QPaintEvent *);
    virtual QSize sizeHint() const;
    virtual QSize minimumSizeHint() const;
    virtual bool hitButton(const QPoint &pos) const;
signals:
    
public slots:
};

#endif // STAUTHORIZECANCELBUTTON_H
