#include "stsimpleeditview.h"
#include <QImage>
#include "stfont.h"
#include <QPainter>
#include <QCursor>
#include <QKeyEvent>
#include "stcaretview.h"
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneWheelEvent>
#include <QApplication>
#include <QClipboard>
#include <QDebug>
#include <QDateTime>
#include <QWidget>
#include <QGraphicsWidget>
#include "math.h"

STSimpleEditView::STSimpleEditView(QGraphicsItem *parent) :
    QGraphicsObject(parent)
{
    m_size=QSize(0,0);
    m_backgroundColor=QColor(255,255,255);
    m_textColor=QColor(0,0,0);

    //qSwap(m_backgroundColor, m_textColor);

    m_text="";
    m_markPos=0;
    m_cursorPos=0;

    m_lineNavigationMode=false;

    m_nextActionPos=0;
    m_actions.clear();

    m_layoutedText="";
    m_layout=STFont::defaultFont()->layoutString("");

    m_scroll=0;

    m_caretView=new STCaretView(this);
    m_caretView->setSize(QSizeF(2.f,12.f));

    m_scrollBar=new STScrollBarView(this);
    m_scrollBar->setStyle(STScrollBarView::ClearBlack);

    setFlag(ItemIsFocusable);
    setFlag(ItemAcceptsInputMethod);
    setFlag(ItemClipsChildrenToShape);
    setCursor(QCursor(Qt::IBeamCursor));

    setCacheMode(DeviceCoordinateCache);

    connect(m_scrollBar, SIGNAL(valueChanged()),
            this, SLOT(scrollChanged()));
}

void STSimpleEditView::setSize(const QSize &sz){
    if(m_size==sz)return;
    prepareGeometryChange();
    m_size=sz;
    m_scrollBar->setPos(m_size.width()-10,1);
    m_scrollBar->setSize(QSize(9, m_size.height()-2));

    updateScroll();
}

void STSimpleEditView::setBackgroundColor(const QColor &color){
    if(m_backgroundColor==color)return;
    m_backgroundColor=color;
    update();
}

void STSimpleEditView::setTextColor(const QColor &color){
    if(m_textColor==color)return;
    m_textColor=color;
    update();
}

QPointF STSimpleEditView::textOrigin() const{
    return QPointF(3.f, 3.f-m_scroll);
}

QRectF STSimpleEditView::microFocusRect() const{
    //QString text=m_text;
    bool preedit=!m_preeditString.isEmpty();
    int cursorPos=m_cursorPos;

    if(preedit){
        //text=text.left(m_cursorPos)+m_preeditString;
        cursorPos=preeditCursorPos();
        if(cursorPos==-1){
            return QRectF(-10.f, -10.f, 0.f, 0.f);
        }
        cursorPos+=m_cursorPos;
    }

    const STFont::Layout& layout=textLayout();//STFont::defaultFont()->layoutString(text);
    QPointF origin=textOrigin();
    foreach(const STFont::LayoutGlyph& glyph, layout){
        if(glyph.charIndex==cursorPos){
            float pos=glyph.rect.left()+origin.x();
            if(pos<1.f && pos>=0.f)pos=1.f;
            if(pos>(float)m_size.width()-1.f && pos<=(float)m_size.width())pos=m_size.width()-1;
            return QRectF(roundf(pos)-1.f, glyph.rect.top()-1.f+origin.y(), 2.f, 12.f);
        }
    }
    return QRectF(0.f, 0.f, 1.f, 12.f);
}

void STSimpleEditView::lineNavigation(bool shift, int delta) {
    const STFont::Layout& layout=textLayout();
    float currentLineY;
    int currentLine;

    if(!m_lineNavigationMode){
        foreach(const STFont::LayoutGlyph& glyph, layout){
            if(glyph.charIndex==m_cursorPos){
                float pos=glyph.rect.left();
                m_lineNavigationX=pos;
                break;
            }
        }
        m_lineNavigationMode=true;
    }

    foreach(const STFont::LayoutGlyph& glyph, layout){
        if(glyph.charIndex==m_cursorPos){
            currentLineY=glyph.rect.center().y();
            currentLine=glyph.line;
            break;
        }
    }

    int maxLine=layout.last().line;

    currentLineY+=(float)delta*14.f;
    currentLine+=delta;

    int newCursorPos=STFont::defaultFont()->cursorPosAtPoint(textLayout(),
                                                             QPoint(m_lineNavigationX, currentLineY));

    if(currentLine<0)newCursorPos=0;
    if(currentLine>maxLine)newCursorPos=m_text.size();

    m_cursorPos=newCursorPos;
    if(!shift)m_markPos=m_cursorPos;

    ensureCursorVisible();
    blinkCursor();
    updateCursor();

    update();
}

int STSimpleEditView::charIndexAtPoint(const QPointF &pt) const{
    return STFont::defaultFont()->cursorPosAtPoint(textLayout(), pt-textOrigin());
}

void STSimpleEditView::updateCursor(){
    if(hasSelection()){
        m_caretView->setVisible(false);
    }else{

        m_caretView->setVisible(true);
        if(hasFocus())
            m_caretView->showCaret();
        else
            m_caretView->hideCaret();

        m_caretView->setPos(microFocusRect().topLeft());
    }
}

const STFont::Layout& STSimpleEditView::textLayout() const{
    QString text=m_text;
    if(!m_preeditString.isEmpty()){
        text=text.left(m_cursorPos)+m_preeditString+text.mid(m_cursorPos);
    }
    if(text!=m_layoutedText){
        m_layout=STFont::defaultFont()->layoutString(text, m_size.width()-14);
        m_layoutedText=text;
    }
    return m_layout;
}

void STSimpleEditView::paint(QPainter *outPainter, const QStyleOptionGraphicsItem *, QWidget *){
    QImage image(m_size, (m_backgroundColor.alpha()==255)?QImage::Format_RGB32:QImage::Format_ARGB32_Premultiplied);
    QPainter painter(&image);
    image.fill(m_backgroundColor);

    QPointF origin=textOrigin();
/*
    QString text=m_text;
    if(!m_preeditString.isEmpty()){
        text=text.left(m_cursorPos)+m_preeditString+text.mid(m_cursorPos);
    }*/
    const STFont::Layout &layout=textLayout();
    STFont::DrawOption options;
    options.highlightStartIndex=qMin(m_cursorPos, m_markPos);
    options.highlightEndIndex=qMax(m_cursorPos, m_markPos);
    options.highlightColor=QColor(255,255,255);

    if(!this->hasFocus()){
        options.highlightEndIndex=0; // hide selection
    }

    if(!m_preeditString.isEmpty()){
        options.highlightStartIndex=m_cursorPos;
        options.highlightEndIndex=m_cursorPos+m_preeditString.length();
        options.highlightColor=QColor(220,128, 40);


    }else{

        QColor selectionColor(220, 128, 40);
        foreach(const STFont::LayoutGlyph& glyph, layout){
            if(glyph.charIndex>=options.highlightStartIndex &&
                    glyph.charIndex<options.highlightEndIndex){
                painter.fillRect(glyph.rect.translated(origin), selectionColor);
            }
        }

    }

    STFont::defaultFont()->drawLayout(image, origin, m_textColor,
                                        layout, options);


    outPainter->drawImage(0,0,image);
}

QRectF STSimpleEditView::boundingRect() const{
    return QRectF(0.f, 0.f, m_size.width(), m_size.height());
}

int STSimpleEditView::preeditCursorPos() const{
    for(int i=0;i<m_preeditAttrs.size();i++){
        const QInputMethodEvent::Attribute &a=m_preeditAttrs[i];
        if(a.type==QInputMethodEvent::Cursor){
            if(a.length)
                return a.start;
            else
                return -1;
        }
    }
    return -1;
}

void STSimpleEditView::inputMethodEvent(QInputMethodEvent *event){
    removeSelectedText();

    m_lineNavigationMode=false;

    int c=m_cursorPos;
    if(event->replacementStart()<=0)
        c+=event->commitString().length()+qMin(-event->replacementStart(), event->replacementLength());

    m_cursorPos+=event->replacementStart();

    if(event->replacementLength()){
        m_markPos=m_cursorPos+event->replacementLength();
    }

    insert(event->commitString());

    m_cursorPos=m_markPos=c;

    m_preeditString=event->preeditString();
    m_preeditAttrs=event->attributes();

    ensureCursorVisible();
    blinkCursor();
    updateCursor();

    update();

    emit textChanged();
}

QVariant STSimpleEditView::inputMethodQuery(Qt::InputMethodQuery query) const{
    switch(query){
    case Qt::ImMicroFocus:
        return microFocusRect();
    case Qt::ImFont:
        return STFont::defaultFont()->compatibleNativeFont();
    case Qt::ImCursorPosition:
        return m_cursorPos;
    case Qt::ImAnchorPosition:
        return m_markPos;
    case Qt::ImSurroundingText:
        return m_text;
    case Qt::ImCurrentSelection:
        return selectedText();
    default:
        return QVariant();
    }
}

void STSimpleEditView::focusInEvent(QFocusEvent *event){
    emit focusIn(event);
    updateCursor(); update();
}
void STSimpleEditView::focusOutEvent(QFocusEvent *event){
    emit focusOut(event);
    updateCursor(); update();
}

void STSimpleEditView::keyPressEvent(QKeyEvent *event){
    if(event->key()!=Qt::Key_Up && event->key()!=Qt::Key_Down){
        m_lineNavigationMode=false;
    }
    if(event->key()==Qt::Key_Return || event->key()==Qt::Key_Enter){
        if(event->modifiers()&Qt::ControlModifier){
            event->ignore();
            return;
        }
        blinkCursor();
        insert("\n");
        event->ignore();
        emit textChanged();
        return;
    }

    blinkCursor();

    QString otxt=m_text;

    if(event==QKeySequence::Undo){
        undo();
        event->ignore();
        if(m_text!=otxt){
            emit textChanged();
        }
        return;
    }else if(event==QKeySequence::Redo){
        redo();
        event->ignore();
        if(m_text!=otxt){
            emit textChanged();
        }
        return;
    }else if(event==QKeySequence::SelectAll){
        selectAll();
        event->ignore();
        return;
    }else if(event==QKeySequence::Cut){
        cut();
        event->ignore();
        if(m_text!=otxt){
            emit textChanged();
        }
        return;
    }else if(event==QKeySequence::Copy){
        copy();
        event->ignore();
        return;
    }else if(event==QKeySequence::Paste){
        paste();
        event->ignore();
        if(m_text!=otxt){
            emit textChanged();
        }
        return;
    }else if(event==QKeySequence::DeleteEndOfLine){
        end(true);
        removeSelectedText();
        event->ignore();
        if(m_text!=otxt){
            emit textChanged();
        }
        return;
    }else if(event==QKeySequence::MoveToStartOfLine){
        home(false);
        event->ignore();
        return;
    }else if(event==QKeySequence::MoveToEndOfLine){
        end(false);
        event->ignore();
        return;
    }else if(event==QKeySequence::SelectStartOfLine){
        home(true);
        event->ignore();
        return;
    }else if(event==QKeySequence::SelectEndOfLine){
        home(false);
        event->ignore();
        return;
    }else if(event==QKeySequence::MoveToNextChar){
        cursorForward(false, 1);
        event->ignore();
        return;
    }else if(event==QKeySequence::SelectNextChar){
        cursorForward(true, 1);
        event->ignore();
        return;
    }else if(event==QKeySequence::MoveToPreviousChar){
        cursorForward(false, -1);
        event->ignore();
        return;
    }else if(event==QKeySequence::SelectPreviousChar){
        cursorForward(true, -1);
        event->ignore();
        return;
    }else if(event==QKeySequence::MoveToNextWord){
        cursorWordForward(false, 1);
        event->ignore();
        return;
    }else if(event==QKeySequence::SelectNextWord){
        cursorWordForward(true, 1);
        event->ignore();
        return;
    }else if(event==QKeySequence::MoveToPreviousWord){
        cursorWordForward(false, -1);
        event->ignore();
        return;
    }else if(event==QKeySequence::SelectPreviousWord){
        cursorWordForward(true, -1);
        event->ignore();
        return;
    }else if(event==QKeySequence::Delete){
        if(hasSelection()){
            removeSelectedText();
        }else{
            cursorForward(true, 1);
            removeSelectedText();
        }
        event->ignore();
        if(m_text!=otxt){
            emit textChanged();
        }
        return;
    }else if(event==QKeySequence::DeleteEndOfWord){
        cursorWordForward(true, 1);
        removeSelectedText();
        event->ignore();
        if(m_text!=otxt){
            emit textChanged();
        }
        return;
    }else if(event==QKeySequence::DeleteStartOfWord){
        cursorWordForward(true, -1);
        removeSelectedText();
        event->ignore();
        if(m_text!=otxt){
            emit textChanged();
        }
        return;
    }else if(event->key()==Qt::Key_Up){
        lineNavigation((event->modifiers()&Qt::ShiftModifier)!=0, -1);
        event->ignore();
        return;
    }else if(event->key()==Qt::Key_Down){
        lineNavigation((event->modifiers()&Qt::ShiftModifier)!=0, 1);
        event->ignore();
        return;
    }else if(event->key()==Qt::Key_Backspace){
        backspace();
        event->ignore();
        if(m_text!=otxt){
            emit textChanged();
        }
        return;
    }else{
        if(event->key()==Qt::Key_Escape){
            event->ignore();
            return;
        }
        QString t=event->text();
        if(!t.isEmpty()){
            insert(t);
            event->ignore();
            if(m_text!=otxt){
                emit textChanged();
            }
            return;
        }
    }
}

void STSimpleEditView::mousePressEvent(QGraphicsSceneMouseEvent *event){
    if(event->button()==Qt::RightButton) return;

    m_lineNavigationMode=false;

    int pos=charIndexAtPoint(event->pos());
    m_cursorPos=pos;
    if(!(event->modifiers()&Qt::ShiftModifier)){
        m_markPos=pos;
    }

    ensureCursorVisible();
    blinkCursor();
    updateCursor();

    update();

    this->grabMouse();
}

void STSimpleEditView::mouseMoveEvent(QGraphicsSceneMouseEvent *event){
    if(event->buttons()&Qt::LeftButton){

        m_lineNavigationMode=false;

        int pos=charIndexAtPoint(event->pos());
        m_cursorPos=pos;

        ensureCursorVisible();
        blinkCursor();
        updateCursor();

        update();
    }
}

void STSimpleEditView::mouseReleaseEvent(QGraphicsSceneMouseEvent *event){
    if(event->button()==Qt::RightButton) return;

    this->ungrabMouse();
}

void STSimpleEditView::wheelEvent(QGraphicsSceneWheelEvent *event){
    QGraphicsWidget *wid=dynamic_cast<QGraphicsWidget *>(event->widget());

    m_scrollBar->setValue(m_scrollBar->value()-event->delta());
}

void STSimpleEditView::setText(const QString &text){
    if(text==m_text)return;
    m_lineNavigationMode=false;
    m_text=text;
    m_markPos=0;
    m_cursorPos=0;
    clearUndoStack();
    update();
    blinkCursor();
    updateCursor();
    ensureCursorVisible();

}

void STSimpleEditView::insert(const QString &text){
    QString oldString=selectedText();

    removeSelectedText(false);

    Q_ASSERT(!hasSelection());

    if(m_text.size()+text.size()>8192){
        pushAction(Action(m_cursorPos, oldString, ""));
        return;
    }

    pushAction(Action(m_cursorPos, oldString, text));

    m_text=m_text.insert(m_cursorPos, text);
    m_cursorPos+=text.length();
    m_markPos=m_cursorPos;

    m_lineNavigationMode=false;

    ensureCursorVisible();
    blinkCursor();
    updateCursor();

    update();


}

QString STSimpleEditView::selectedText() const{
    int minPos=qMin(m_markPos, m_cursorPos);
    int maxPos=qMax(m_markPos, m_cursorPos);
    return m_text.mid(minPos, maxPos-minPos);
}

void STSimpleEditView::removeSelectedText(bool record){
    if(!hasSelection()) return;
    int minPos=qMin(m_markPos, m_cursorPos);
    int maxPos=qMax(m_markPos, m_cursorPos);

    if(record)
        pushAction(Action(minPos, m_text.mid(minPos, maxPos-minPos), ""));

    m_text=m_text.left(minPos)+m_text.mid(maxPos);
    m_markPos=m_cursorPos=minPos;
    ensureCursorVisible();
    blinkCursor();
    updateCursor();

    update();
}

void STSimpleEditView::cut(){
    if(!hasSelection())return;
    copy();
    removeSelectedText();
}

void STSimpleEditView::copy(){
    QApplication::clipboard()->setText(selectedText());
}

void STSimpleEditView::paste(){
    QString str=QApplication::clipboard()->text();
    insert(str);
}

void STSimpleEditView::backspace(){
    if(hasSelection()){
        removeSelectedText();
        return;
    }

    cursorForward(true, -1);
    removeSelectedText();
}

void STSimpleEditView::selectAll(){
    m_cursorPos=m_text.size();
    m_markPos=0;

    ensureCursorVisible();
    blinkCursor();
    updateCursor();

    update();
}

void STSimpleEditView::home(bool shift){
    m_cursorPos=0;
    if(!shift)m_markPos=0;

    ensureCursorVisible();
    blinkCursor();
    updateCursor();

    update();
}

void STSimpleEditView::end(bool shift){
    m_cursorPos=m_text.size();
    if(!shift)m_markPos=m_text.size();

    ensureCursorVisible();
    blinkCursor();
    updateCursor();

    update();
}

void STSimpleEditView::cursorForward(bool shift, int delta){
    int newPos=m_cursorPos;
    while(delta>0){
        newPos=STFont::defaultFont()->nextCursorPos(m_text, newPos);
        delta--;
    }
    while(delta<0){
        newPos=STFont::defaultFont()->previousCursorPos(m_text, newPos);
        delta++;
    }

    m_cursorPos=newPos;
    if(!shift)m_markPos=m_cursorPos;

    ensureCursorVisible();
    blinkCursor();
    updateCursor();

    update();

}

void STSimpleEditView::cursorWordForward(bool shift, int delta){
    // TODO:
    cursorForward(shift, delta);
}

void STSimpleEditView::ensureCursorVisible(){
    updateScroll();



    float maxScroll=(float)m_scrollBar->maxValue();
    float newScroll=m_scroll;

    QRectF caretPos=this->microFocusRect();
    float fieldHeight=(float)m_size.height();

    if(caretPos.top()<0.f){
        newScroll+=caretPos.top()-6.f;
    }else if(caretPos.bottom()>fieldHeight){
        newScroll+=caretPos.bottom()-fieldHeight+6.f;
    }
    if(newScroll<0.f)newScroll=0.f;
    if(newScroll>maxScroll)newScroll=maxScroll;


    if(newScroll==m_scroll) return;

    m_scroll=newScroll;
    m_scrollBar->setValue(newScroll);
    updateCursor();
    update();

}

void STSimpleEditView::updateScroll(){
    const STFont::Layout& layout=textLayout();
    float textHeight=STFont::defaultFont()->boundingRectForLayout(layout).bottom()+4.f;
    float fieldHeight=(float)m_size.height();
    float maxScroll=qMax(0.f, textHeight-fieldHeight);
    m_scrollBar->setRange(0., (double)maxScroll);
    m_scrollBar->setLargeIncrement(fieldHeight);

}

void STSimpleEditView::scrollChanged(){
    m_scroll=(int)m_scrollBar->value();
    updateCursor();
    update();
}

void STSimpleEditView::blinkCursor(){
    m_caretView->blankCaret();
}

void STSimpleEditView::clearUndoStack(){
    m_actions.clear();
    m_nextActionPos=0;
}

void STSimpleEditView::pushAction(const Action &action){
    if(action.newString==action.oldString) return;

    while(m_actions.size()>m_nextActionPos){
        m_actions.removeAt(m_nextActionPos);
    }

    if(action.newString.size()<=2 && action.oldString.isEmpty() &&
            m_nextActionPos>0 && action.newString!=" "){
        // can combine with old action?
        Action& last=m_actions[m_nextActionPos-1];
        if(last.oldString.isEmpty()){
            if(last.charIndex+last.newString.size()==action.charIndex){
                // combine
               // qDebug()<<"combined";
                last.newString+=action.newString;
                return;
            }
        }
    }
    //qDebug()<<action.charIndex<<": "<<action.oldString<<" -> "<<action.newString;
    m_actions.push_back(action);
    m_nextActionPos++;
    Q_ASSERT(m_nextActionPos==m_actions.size());

    if(m_actions.size()>256){
        m_actions.pop_front();
        m_nextActionPos--;
    }
}

void STSimpleEditView::undo(){
    if(m_nextActionPos>0){
        const Action& a=m_actions[m_nextActionPos-1];
        m_nextActionPos--;

        m_text=m_text.left(a.charIndex)+a.oldString+m_text.mid(a.charIndex+a.newString.length());
        m_cursorPos=a.charIndex+a.oldString.size();
        m_markPos=a.charIndex;


        ensureCursorVisible();
        blinkCursor();
        updateCursor();

        update();
    }
}

void STSimpleEditView::redo(){
    if(m_nextActionPos<m_actions.size()){
        const Action& a=m_actions[m_nextActionPos];
        m_nextActionPos++;


        m_text=m_text.left(a.charIndex)+a.newString+m_text.mid(a.charIndex+a.oldString.length());
        m_cursorPos=a.charIndex+a.newString.size();
        m_markPos=a.charIndex;

        ensureCursorVisible();
        blinkCursor();
        updateCursor();

        update();
    }
}

void STSimpleEditView::insert(const QString &text, int pos, int range){

    range+=pos;
    if(pos<0)pos=0;
    if(pos>m_text.size())pos=m_text.size();
    if(range<0)range=0;
    if(range>m_text.size())range=m_text.size();

    if(m_text.size()+text.size()-(range-pos)>8192){
        return;
    }

    if(m_markPos>=range)
        m_markPos+=text.length()-(range-pos);
    if(m_cursorPos>=range)
        m_cursorPos+=text.length()-(range-pos);

    pushAction(Action(pos, text.mid(pos, range-pos), text));
    m_text=m_text.left(pos)+text+m_text.mid(range);


    ensureCursorVisible();
    blinkCursor();
    updateCursor();

    update();
}

void STSimpleEditView::setSelection(int mark, int cursor){
    m_markPos=mark;
    m_cursorPos=cursor;

    ensureCursorVisible();
    blinkCursor();
    updateCursor();

    update();
}
