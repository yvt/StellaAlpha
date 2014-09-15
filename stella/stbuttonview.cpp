#include "stbuttonview.h"
#include "stbuttongraphicsview.h"
#include <QGraphicsSceneMouseEvent>

STButtonView::STButtonView(QGraphicsItem *parent) :
    QGraphicsObject(parent)
{
    for(int i=0;i<(int)ButtonStateCount;i++){
        STButtonGraphicsView *v=new STButtonGraphicsView(this, (ButtonState)i);
        v->setVisible((i==0));
        m_graphics[i]=v;
    }

    m_hover=false;
    m_state=Normal;
    m_pressed=false;
    m_size=QSize(0,0);
    m_checked=false;
    m_text="";

    m_buttonType=Standard;

    setFlag(ItemHasNoContents);
    setAcceptsHoverEvents(true);
}

void STButtonView::setSize(const QSize &size){
    if(m_size==size)return;
    prepareGeometryChange();
    m_size=size;
    for(int i=0;i<(int)ButtonStateCount;i++){
        m_graphics[i]->setSize(size);
    }
}

void STButtonView::setChecked(bool checked){
    if(m_checked==checked)return;
    m_checked=checked;
    updateState();
}

void STButtonView::setText(const QString &text){
    if(m_text==text)return;
    m_text=text;
    for(int i=0;i<(int)ButtonStateCount;i++){
        m_graphics[i]->update();
    }
}

void STButtonView::setButtonType(ButtonType t){
    if(t==m_buttonType)return;
    m_buttonType=t;
    for(int i=0;i<(int)ButtonStateCount;i++){
        m_graphics[i]->update();
    }
}


void STButtonView::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){

}

QRectF STButtonView::boundingRect() const{
    return QRectF(0.f, 0.f, m_size.width(), m_size.height());
}

void STButtonView::updateState() {
    m_state=Normal;
    if(m_hover){
        m_state=Hover;
    }
    if(m_checked){
        m_state=Checked;
    }
    if(m_pressed && m_hover){
        m_state=Pressed;
    }

    for(int i=0;i<(int)ButtonStateCount;i++){
        m_graphics[i]->setVisible(i==(int)m_state);
    }
}

void STButtonView::hoverEnterEvent(QGraphicsSceneHoverEvent *event){
    m_hover=true;
    updateState();
}

void STButtonView::hoverLeaveEvent(QGraphicsSceneHoverEvent *event){
    m_hover=false;
    updateState();
}

void STButtonView::mousePressEvent(QGraphicsSceneMouseEvent *event){
    if(event->buttons()&Qt::LeftButton){
        m_pressed=true;
        m_dragInitated=false;
        m_pressPos=event->pos();
        updateState();
    }
    grabMouse();
   // QGraphicsObject::mousePressEvent(event);
}

void STButtonView::mouseReleaseEvent(QGraphicsSceneMouseEvent *event){
    if(m_pressed){
        m_pressed=false;
        if(m_hover){
            emit activated();
        }
        updateState();
    }
    ungrabMouse();
   //QGraphicsObject::mouseReleaseEvent(event);
}

void STButtonView::mouseMoveEvent(QGraphicsSceneMouseEvent *event){
    QPointF pos=event->pos();
    if(m_pressed && !m_dragInitated){
        QPointF dif=event->pos()-m_pressPos;
        if(dif.manhattanLength()>=(float)QApplication::startDragDistance()){
            m_dragInitated=true;
            emit dragInitated(dif);
        }
    }
    m_hover=(boundingRect().contains(pos));
    updateState();
}

void STButtonView::forceRelease(){
    m_pressed=false;
    ungrabMouse();
}
