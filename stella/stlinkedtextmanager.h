#ifndef STLINKEDTEXTMANAGER_H
#define STLINKEDTEXTMANAGER_H

#include "stobjectmanager.h"
#include "stfont.h"
#include <QImage>

class STLinkedTextManager
{
    QList<STObjectManager::EntityRange> m_entityRanges;
    STFont::Layout m_layout;
    QPointF m_pos;
    int m_hover;
    int m_drag;
    STFont *m_font;

    bool m_textHover;

public:
    STLinkedTextManager();
    void init(const QString&, const QList<STObjectManager::EntityRange>&,
              STFont *, float maxWidth=1.e+10f);
    void init(const STFont::Layout&, const QList<STObjectManager::EntityRange>&,
              STFont *);
    void clear();

    STFont::Layout layout() const{return m_layout;}
    bool isEmpty() const{return m_layout.isEmpty();}

    STFont *font() const{return m_font;}

    QPointF pos() const{return m_pos;}
    void setPos(const QPointF& newPos){m_pos=newPos;}

    void draw(QImage&, QPainter&, const QColor& textColor, const QColor& linkColor);
    void draw(QImage&, QPainter&, const QColor& textColor, const QColor& linkColor,
              const QPointF& shadowShift, const QColor& shadowColor);

    bool mouseMove(const QPointF&); // true = update!
    bool mouseLeave();              // true = update!
    bool mouseDown(const QPointF&); // true = update!
    bool mouseUp();                 // true = link activated! (be sure to call mouseMove before calling this)

    STObjectManager::EntityRange *hotEntityRange();
    bool textHover() const{return m_textHover;}

    int entityIndexAtPoint(const QPointF&);
    bool textContainsPoint(const QPointF&);

};

#endif // STLINKEDTEXTMANAGER_H
