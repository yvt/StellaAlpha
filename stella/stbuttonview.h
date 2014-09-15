#ifndef STBUTTONVIEW_H
#define STBUTTONVIEW_H

#include "ststdafx.h"
#include <QGraphicsObject>

class STButtonGraphicsView;

class STButtonView : public QGraphicsObject
{
    Q_OBJECT



public:

    enum ButtonState{
        Normal=0,
        Hover,
        Pressed,
        Checked,
        ButtonStateCount
    };

    enum ButtonType{
        Standard,
        Flat,
        BackButton,
        SidebarButton
    };

    explicit STButtonView(QGraphicsItem *parent = 0);
    
    void setSize(const QSize&);
    QSize size() const{return m_size;}

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual QRectF boundingRect() const;

    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);

    void setChecked(bool);
    bool checked() const{return m_checked;}

    void setButtonType(ButtonType);
    ButtonType buttonType() const{return m_buttonType;}

    void setText(const QString&);
    QString text() const{return m_text;}

    void setFlat(bool b){setButtonType(b?Flat:Standard);}
    bool isFlat() const{return buttonType()==Flat;}

    void setBack(bool b){setButtonType(b?BackButton:Standard);}
    bool isBack() const{return buttonType()==BackButton;}

    void forceRelease();

private:

    QSize m_size;
    STButtonGraphicsView *m_graphics[ButtonStateCount];
    ButtonState m_state;
    bool m_hover;
    bool m_pressed;
    bool m_checked;
    QString m_text;
    ButtonType m_buttonType;
    QPointF m_pressPos;
    bool m_dragInitated;

    void updateState();

signals:

    void activated();
    void dragInitated(QPointF initialShift);
    
public slots:
    
};

#endif // STBUTTONVIEW_H
