#ifndef STLINKEDLABELVIEW_H
#define STLINKEDLABELVIEW_H

#include <QGraphicsObject>
#include "stobjectmanager.h"
#include "stlinkedtextmanager.h"
#include "stfont.h"
#include <QAction>

class STLinkedLabelView : public QGraphicsObject
{
    Q_OBJECT

    QString m_text;
    STLinkedTextManager m_manager;
    QRectF m_textBoundary;
    QRect m_imageBoundary;
    QColor m_bgColor;
    QColor m_textColor;
    QColor m_linkColor;
    bool m_selectable;
    bool m_drag;
    bool m_hasSelection;
    int m_markPos;
    int m_cursorPos;
    bool m_menuVisible;

    QAction *m_copyAction;

    void updateCursor();
public:
    explicit STLinkedLabelView(QGraphicsItem *parent = 0);

    void setContents(STFont *font,
                     QString text,
                     QList<STObjectManager::EntityRange> ranges,
                     float maxWidth=1.e+20f,
                     bool singleLine=false,
                     float margin=0.f);
    void setContents(STFont *font,
                     QString text,
                     float maxWidth=1.e+20f,
                     bool singleLine=false,
                     float margin=0.f);

    void setBgColor(QColor);
    void setTextColor(QColor);
    void setLinkColor(QColor);
    void setSelectable(bool); // enable focusing to enable copy

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual QRectF boundingRect() const;

    QString selectedText();

protected:
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    virtual void hoverMoveEvent(QGraphicsSceneHoverEvent *event);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    virtual void focusOutEvent(QFocusEvent *event);
    virtual void keyPressEvent(QKeyEvent *event);

signals:
    void linkActivated(STObjectManager::EntityRange *,
                       STObjectManager::LinkActivationInfo);
public slots:
    void copySelection();
};

#endif // STLINKEDLABELVIEW_H
