#include "ststdafx.h"
#include "stlinkedlabelview.h"
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneHoverEvent>
#include <QCursor>
#include <math.h>
#include <QPainter>
#include <QImage>
#include <QPixmap>
#include <QMenu>
#include <QClipboard>
#include <QKeyEvent>

STLinkedLabelView::STLinkedLabelView(QGraphicsItem *parent) :
    QGraphicsObject(parent)
{
    setAcceptsHoverEvents(true);
    m_bgColor=QColor(0,0,0,0);
    m_textColor=QColor(240,240,240);
    m_linkColor=QColor(220,128, 40);
    m_hasSelection=false;
    m_drag=false;
    setCacheMode(DeviceCoordinateCache);
    m_copyAction=NULL;
    m_imageBoundary=QRect(0,0,0,0);
    m_textBoundary=QRect(0,0,0,0);
    m_menuVisible=false;
    m_selectable=false;
}

void STLinkedLabelView::updateCursor(){
    if(m_manager.hotEntityRange())
        setCursor(Qt::PointingHandCursor);
    else if(/*m_manager.textHover() &&*/ m_selectable)
        setCursor(Qt::IBeamCursor);
    else
        setCursor(Qt::ArrowCursor);
}

static QPoint pointFloor(QPointF p){
    return QPoint((int)floorf(p.x()),
                  (int)floorf(p.y()));
}

static QPoint pointCeil(QPointF p){
    return QPoint((int)ceilf(p.x()),
                  (int)ceilf(p.y()));
}

void STLinkedLabelView::setContents(STFont *font, QString text, QList<STObjectManager::EntityRange> ranges, float maxWidth, bool singleLine,
                                    float margin){
    prepareGeometryChange();
    m_manager.clear();

    STFont::Layout lay=font->layoutString(text, maxWidth, singleLine);
    m_manager.init(lay, ranges, font);

    m_text=text;

    m_textBoundary=font->visualBoundingRectForLayout(lay);
    m_textBoundary.adjust(-margin, -margin, margin, margin);
    m_imageBoundary=QRect(pointFloor(m_textBoundary.topLeft()),
                           pointCeil(m_textBoundary.bottomRight()));
    m_manager.setPos(QPointF(-m_imageBoundary.left(), -m_imageBoundary.top()));

    m_hasSelection=false;
    m_drag=false;

    update();

}

void STLinkedLabelView::setContents(STFont *font, QString text,
                                    float maxWidth,
                                    bool singleLine,
                                    float margn){
    setContents(font, text,QList<STObjectManager::EntityRange>(), maxWidth, singleLine, margn);
}

void STLinkedLabelView::setBgColor(QColor color){
    if(color==m_bgColor)
        return;
    m_bgColor=color;
    update();
}

void STLinkedLabelView::setTextColor(QColor color){
    if(color==m_textColor) return;
    m_textColor=color;
    update();
}

void STLinkedLabelView::setLinkColor(QColor color){
    if(color==m_linkColor) return;
    m_linkColor=color;
    update();
}

void STLinkedLabelView::setSelectable(bool b){
    m_selectable=b;
    if(!b){
        if(m_hasSelection)
            update();
        m_hasSelection=false;
    }
}

void STLinkedLabelView::paint(QPainter *outPainter, const QStyleOptionGraphicsItem *, QWidget *){
    if(m_imageBoundary.isEmpty())
        return;

    QImage image(m_imageBoundary.width(), m_imageBoundary.height(),
                 (m_bgColor.alpha()==255)?QImage::Format_RGB32:QImage::Format_ARGB32_Premultiplied);
    image.fill(m_bgColor);

    QPainter painter(&image);
    m_manager.draw(image, painter, m_textColor, m_linkColor);

    if(m_hasSelection){
        STFont::DrawOption opt;
        QPointF pt=m_manager.pos();
        opt.highlightColor=QColor(255,255,255);
        opt.highlightStartIndex=qMin(m_markPos, m_cursorPos);
        opt.highlightEndIndex=qMax(m_markPos, m_cursorPos);

        QColor selectionColor(220, 128, 40);
        STFont::Layout layout=m_manager.layout();
        for(int i=0;i<layout.count();i++){
            STFont::LayoutGlyph& glyph=layout[i];
            glyph.highlighted=false;
            if(glyph.charIndex>=opt.highlightStartIndex &&
                    glyph.charIndex<opt.highlightEndIndex){
                painter.fillRect(glyph.rect.translated(pt), selectionColor);
            }
        }


        m_manager.font()->drawLayout(image, pt,
                                     QColor(0,0,0,0),layout, opt);
    }

    outPainter->drawImage(m_imageBoundary.left(), m_imageBoundary.top(),
                          image);
}

QRectF STLinkedLabelView::boundingRect() const{
    return QRectF(m_imageBoundary.left(), m_imageBoundary.top(),
                  m_imageBoundary.width(), m_imageBoundary.height());
}

void STLinkedLabelView::hoverEnterEvent(QGraphicsSceneHoverEvent *){

}

void STLinkedLabelView::hoverMoveEvent(QGraphicsSceneHoverEvent *event){
    QPointF managerPos=event->pos();
    managerPos-=QPointF(m_imageBoundary.left(), m_imageBoundary.top());

    if(m_manager.mouseMove(managerPos))
        update();
    updateCursor();
}

void STLinkedLabelView::hoverLeaveEvent(QGraphicsSceneHoverEvent *){
    if(m_manager.mouseLeave())
        update();
    updateCursor();
}

void STLinkedLabelView::mousePressEvent(QGraphicsSceneMouseEvent *event){
    QPointF managerPos=event->pos();
    managerPos-=QPointF(m_imageBoundary.left(), m_imageBoundary.top());

    if(event->button()==Qt::LeftButton){
        if(m_manager.mouseDown(managerPos))
            update();
    }
    if((!m_manager.hotEntityRange()) && /*m_manager.textHover() &&*/ m_selectable &&
            event->button()==Qt::LeftButton){
        m_drag=true;
        m_hasSelection=true;
        m_cursorPos=m_manager.font()->cursorPosAtPoint(m_manager.layout(), event->pos());
        if(!(event->modifiers()&Qt::ShiftModifier)){
            m_markPos=m_cursorPos;
        }
        update();
    }
    if((!m_drag) && event->button()==Qt::LeftButton){
        if(m_hasSelection)
            update();
        m_hasSelection=false;
    }
    grabMouse();
    updateCursor();
}

void STLinkedLabelView::mouseMoveEvent(QGraphicsSceneMouseEvent *event){
    if(m_drag){

        int newCursorPos=m_manager.font()->cursorPosAtPoint(m_manager.layout(), event->pos());
        if(newCursorPos==m_cursorPos)return;
        m_cursorPos=newCursorPos;
        update();
    }
}

void STLinkedLabelView::mouseReleaseEvent(QGraphicsSceneMouseEvent *event){

    ungrabMouse();

    QPointF managerPos=event->pos();
    managerPos-=QPointF(m_imageBoundary.left(), m_imageBoundary.top());

    if(m_drag){
        m_drag=false;
    }

    if(m_manager.mouseMove(managerPos))
        update();
    updateCursor();

    if(m_manager.mouseUp()){
        STObjectManager::LinkActivationInfo info;
        emit linkActivated(m_manager.hotEntityRange(), info);
    }
}

void STLinkedLabelView::focusOutEvent(QFocusEvent *){
    if(m_hasSelection && !m_menuVisible){
        m_hasSelection=false;
        update();
    }
}

void STLinkedLabelView::contextMenuEvent(QGraphicsSceneContextMenuEvent *event){
    if(!m_hasSelection)
        return;
    int curPos=m_manager.font()->cursorPosAtPoint(m_manager.layout(), event->pos());
    if(curPos<qMin(m_markPos, m_cursorPos) ||
            curPos>qMax(m_markPos, m_cursorPos) ||
            m_markPos==m_cursorPos){
        m_hasSelection=false;
        update();
        return;
    }


    if(!m_copyAction){
        m_copyAction=new QAction(tr("Copy"), this);
        connect(m_copyAction, SIGNAL(triggered()),
                this, SLOT(copySelection()));
    }

    QMenu menu;
    menu.addAction(m_copyAction);

    m_menuVisible=true;
    menu.exec(event->screenPos());
    m_menuVisible=false;
    if(!hasFocus()){
        if(m_hasSelection){
            m_hasSelection=false;
            update();
        }
    }
}

QString STLinkedLabelView::selectedText(){
    if(!m_hasSelection)
        return QString();
    int startPos=qMin(m_markPos, m_cursorPos);
    int endPos=qMax(m_markPos, m_cursorPos);
    return m_text.mid(startPos, endPos-startPos);
}

void STLinkedLabelView::copySelection(){
    if(selectedText().isEmpty())
        return;
    QApplication::clipboard()->setText(selectedText());
}

void STLinkedLabelView::keyPressEvent(QKeyEvent *event){
    if(event==QKeySequence::Copy){
        copySelection();
    }else if(event==QKeySequence::SelectAll){
        if(m_selectable){
            m_markPos=0;
            m_cursorPos=m_text.length();
            m_hasSelection=true;
            update();
        }
    }else{
        event->ignore();
    }
}
