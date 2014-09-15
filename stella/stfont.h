#ifndef STFONT_H
#define STFONT_H

#include "ststdafx.h"
#include <QObject>
#include <QImage>
#include <QVector>
#include <QColor>
#include <QFont>


class STFont : public QObject
{
    Q_OBJECT
    const uint8_t *m_bitmapData;
    const uint32_t *m_indexData;
    int m_lineHeight;
    QFont m_compatibleNativeFont;
public:

    struct LayoutGlyph{
        int32_t glyphId;
        uint32_t ucs:31;
        bool pseudoNewLine:1;
        uint32_t charIndex:28;
        uint32_t charLength:2;
        bool combining:1;
        bool highlighted:1;
        uint32_t line; // >= 0
        QRectF rect;
    };

    struct DrawOption{
        int highlightStartIndex;
        int highlightEndIndex;
        QColor highlightColor;
        bool useSubpixelRendering;

        DrawOption():
            highlightStartIndex(0),
            highlightEndIndex(0),
            useSubpixelRendering(true){
        }
    };

    typedef QVector<LayoutGlyph> Layout;

    STFont(const uint8_t *bitmapData,
           const uint32_t *indexData);
    
    static STFont *defaultFont();
    static STFont *defaultBoldFont();
    static bool isLeadingSurrogatePair(ushort);
    static bool isTrailSurrogatePair(ushort);
    static uint32_t decodeSurrogatePair(ushort, ushort);

    Layout layoutString(const QString&, float maxWidth=1.e+10f, bool singleLine=false);

    void drawLayout(QImage& image, const QPointF& pos,
                    const QColor& color,
                    const Layout& layout, const DrawOption&);
    void drawString(QImage& image, const QPointF& pos,
                    const QColor& color, const QString& str,
                    const DrawOption& opt=DrawOption());

    QRectF visualBoundingRectForLayoutGlyph(const LayoutGlyph&);

    QRectF visualBoundingRectForLayout(const Layout&);
    QRectF visualBoundingRectForString(const QString&);

    QRectF boundingRectForLayout(const Layout&);
    QRectF boundingRectForString(const QString&);

    const QFont& compatibleNativeFont() const{return m_compatibleNativeFont;}

    int nextCursorPos(const QString& str, int currentCursorPos);
    int previousCursorPos(const QString& str, int currentCursorPos);

    int cursorPosAtPoint(const Layout&, const QPointF&);

    int lineHeight() const{return m_lineHeight;}

private:

    void drawGlyph(QImage& image, const QPointF& pos,
                   const QColor& color, const uint8_t *bitmap,
                   const DrawOption&);

signals:
    
public slots:
    
};

#endif // STFONT_H
