#include "stlinkedtextmanager.h"
#include <QVarLengthArray>
#include <QPainter>
#include <QDebug>

STLinkedTextManager::STLinkedTextManager():
    m_pos(0,0),m_hover(-1),m_drag(-1)
{
    m_textHover=false;
}

void STLinkedTextManager::init(const QString &str, const QList<STObjectManager::EntityRange> & entityRanges,
                               STFont *font, float maxWidth){

    init(font->layoutString(str, maxWidth), entityRanges, font);
}

void STLinkedTextManager::clear(){
    m_layout.clear();
}

void STLinkedTextManager::init(const STFont::Layout &layout, const QList<STObjectManager::EntityRange> &er, STFont *fnt){
    clear();
    m_layout=layout;
    m_entityRanges=er;
    m_font=fnt;

    if(m_layout.empty())
        return;
    if(er.isEmpty())
        return;

    const STFont::LayoutGlyph& lastGlyph=m_layout.last();
    int charCount=lastGlyph.charIndex;

    // highlight text (O(n+m) algorithm)
    QVarLengthArray<short, 256> highlightCount;
    int curCount=0;
    highlightCount.resize(charCount);
    memset(highlightCount.data(), 0, sizeof(short)*256);

    foreach(const STObjectManager::EntityRange& range, m_entityRanges){
        if(range.charIndexStart<0){
            curCount++;
        }else if(range.charIndexStart<charCount){
            highlightCount[range.charIndexStart]++;
        }
        if(range.charIndexEnd<0){
            curCount--;
        }else if(range.charIndexEnd<charCount){
            highlightCount[range.charIndexEnd]--;
        }
    }

    int lastIndex=-1;
    for(int i=0;i<m_layout.size();i++){
        STFont::LayoutGlyph& glyph=m_layout[i];
        while(lastIndex<glyph.charIndex){
            lastIndex++;
            if(lastIndex<charCount){
                curCount+=(int)highlightCount[lastIndex];
            }


        }
        if(lastIndex>=charCount)
            break;
        if(curCount){
            glyph.highlighted=true;
        }
    }


}

void STLinkedTextManager::draw(QImage &image, QPainter& painter, const QColor &textColor, const QColor &linkColor,
                               const QPointF& shadowShift, const QColor& shadowColor){
    STFont::DrawOption opt;
    opt.highlightColor=linkColor;

    if(shadowColor.alpha()!=0){
        STFont::DrawOption sopt;
        sopt.highlightColor=shadowColor;
        m_font->drawLayout(image, m_pos+shadowShift, shadowColor, m_layout, sopt);

        if(m_hover!=-1){
            Q_ASSERT(m_hover<m_entityRanges.count());
            const STObjectManager::EntityRange& range=m_entityRanges[m_hover];

            foreach(const STFont::LayoutGlyph& glyph, m_layout){
                if(glyph.combining)
                    continue;
                if(glyph.glyphId==-1)
                    continue;
                if(glyph.charIndex>=range.charIndexStart && glyph.charIndex<range.charIndexEnd){
                    int lineY=m_font->lineHeight()*glyph.line+11;
                    QRectF lineRect(glyph.rect.left()+m_pos.x()+shadowShift.x(),
                                    (float)lineY+m_pos.y()+shadowShift.y(),
                                    glyph.rect.width(), 1.f);
                    painter.fillRect(lineRect, shadowColor);
                }
            }
        }
    }
    m_font->drawLayout(image, m_pos, textColor, m_layout, opt);

    if(m_hover!=-1){
        Q_ASSERT(m_hover<m_entityRanges.count());
        const STObjectManager::EntityRange& range=m_entityRanges[m_hover];

        foreach(const STFont::LayoutGlyph& glyph, m_layout){
            if(glyph.combining)
                continue;
            if(glyph.glyphId==-1)
                continue;
            if(glyph.charIndex>=range.charIndexStart && glyph.charIndex<range.charIndexEnd){
                int lineY=m_font->lineHeight()*glyph.line+11;
                QRectF lineRect(glyph.rect.left()+m_pos.x(), (float)lineY+m_pos.y(),
                                glyph.rect.width(), 1.f);
                painter.fillRect(lineRect, linkColor);
            }
        }
    }
}

void STLinkedTextManager::draw(QImage &image, QPainter& painter, const QColor &textColor, const QColor &linkColor){
    draw(image, painter,textColor, linkColor, QPointF(0,0), QColor(0,0,0,0));
}

bool STLinkedTextManager::mouseMove(const QPointF &pt){
    m_textHover=textContainsPoint(pt);
    int newHover=entityIndexAtPoint(pt);
    if(newHover==m_hover)
        return false;
    m_hover=newHover;
    return true;
}

bool STLinkedTextManager::mouseLeave(){
    m_textHover=false;
    if(m_hover!=-1){
        m_hover=-1;
        return true;
    }
    return false;
}

bool STLinkedTextManager::mouseDown(const QPointF &pt){
    m_textHover=textContainsPoint(pt);
    int newHover=entityIndexAtPoint(pt);
    m_drag=m_hover;
    if(newHover==m_hover)
        return false;
    m_hover=newHover;
    return true;
}

bool STLinkedTextManager::mouseUp(){
    if(m_drag==-1)
        return false;

    if(m_drag==m_hover){
        m_drag=-1;
        return true;
    }else{
        m_drag=-1;
        return false;
    }
}

STObjectManager::EntityRange *STLinkedTextManager::hotEntityRange(){
    if(m_hover==-1)
        return NULL;
    return &(m_entityRanges[m_hover]);
}

int STLinkedTextManager::entityIndexAtPoint(const QPointF &pt){
    if(isEmpty()) return -1;
    int charIndex=-1;
    QPointF localPt=pt-m_pos;

    foreach(const STFont::LayoutGlyph& glyph, m_layout){
        if(glyph.rect.contains(localPt)){
            charIndex=glyph.charIndex;
            break;
        }
    }

    //qDebug()<<"input pt: "<<localPt<<", index: "<<charIndex;

    if(charIndex==-1)
        return -1;

    for(int i=0;i<m_entityRanges.count();i++){
        const STObjectManager::EntityRange& range=m_entityRanges[i];
        if(charIndex>=range.charIndexStart && charIndex<range.charIndexEnd){
            //qDebug()<<"hit: "<<i<<", "<<range.entityType;
            return i;
        }
    }

    return -1;
}
bool STLinkedTextManager::textContainsPoint(const QPointF &pt){
    QPointF localPt=pt-m_pos;
    if(isEmpty()) return false;
    foreach(const STFont::LayoutGlyph& glyph, m_layout){
        if(glyph.rect.contains(localPt)){
            return true;
        }
    }
    return false;
}



