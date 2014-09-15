#ifndef STSIMPLEEDITVIEW_H
#define STSIMPLEEDITVIEW_H

#include <QGraphicsObject>
#include <QInputMethodEvent>
#include "stfont.h"
#include "stscrollbarview.h"

class STCaretView;
class STSimpleEditView : public QGraphicsObject
{
    Q_OBJECT

    struct Action{
        int charIndex;
        QString oldString;
        QString newString;

        Action(){}
        Action(int index, const QString& old, const QString& nw):
            charIndex(index), oldString(old), newString(nw){}
    };
    QList<Action> m_actions;
    int m_nextActionPos;

    STCaretView *m_caretView;
    STScrollBarView *m_scrollBar;

    mutable QString m_layoutedText;
    mutable STFont::Layout m_layout;

    QSize m_size;
    QColor m_backgroundColor;
    QColor m_textColor;

    QString m_text;
    int m_markPos;
    int m_cursorPos;

    bool m_lineNavigationMode;
    float m_lineNavigationX;

    float m_scroll;

    QString m_preeditString;
    QList<QInputMethodEvent::Attribute> m_preeditAttrs;

    void updateCursor();
    int preeditCursorPos() const;

    const STFont::Layout& textLayout() const;

    void pushAction(const Action&);
    void clearUndoStack();

    void lineNavigation(bool shift, int delta);

    void updateScroll();

public:
    explicit STSimpleEditView(QGraphicsItem *parent = 0);

    void setSize(const QSize&);
    void setBackgroundColor(const QColor&);
    void setTextColor(const QColor&);

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual QRectF boundingRect() const;

    virtual void inputMethodEvent(QInputMethodEvent *event);
    virtual QVariant inputMethodQuery(Qt::InputMethodQuery query) const;
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void focusInEvent(QFocusEvent *event);
    virtual void focusOutEvent(QFocusEvent *event);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    virtual void wheelEvent(QGraphicsSceneWheelEvent *event);

    QString selectedText() const;

    const QString& text() const{return m_text;}

    bool hasSelection() const{return m_markPos!=m_cursorPos;}

    void ensureCursorVisible();


    QRectF microFocusRect() const;
    QPointF textOrigin() const;

    int charIndexAtPoint(const QPointF&) const;



signals:

    void focusIn(QFocusEvent *);
    void focusOut(QFocusEvent *);

    void textChanged();

public slots:

    void setText(const QString&);

    void insert(const QString&);
    void insert(const QString&, int pos, int range=0);
    void removeSelectedText(bool record=true);
    void backspace();

    void home(bool shift);
    void end(bool shift);
    void cursorForward(bool shift, int delta);
    void cursorWordForward(bool shift, int delta);
    void selectAll();

    void cut();
    void copy();
    void paste();

    void blinkCursor();

    void undo();
    void redo();

    void setSelection(int mark, int cursor);

private slots:

    void scrollChanged();

};

#endif // STSIMPLEEDITVIEW_H
