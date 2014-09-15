#include "stfont.h"
#include <QFile>
#include <QImage>
#include <QColor>

STFont::STFont(const uint8_t *bitmapData,
               const uint32_t *indexDatat) :
    QObject(NULL),
    m_bitmapData(bitmapData),
    m_indexData(indexDatat),
    m_lineHeight(17)
{
    m_compatibleNativeFont=QFont("Stella Sans", 11);

}

static const void *mapFile(const QString& path){
    QFile file(path);
    if(!file.open(QFile::ReadOnly)){
        QByteArray ba=path.toUtf8();
        qFatal("cannot open resource %s", ba.constData());
    }
    if(file.size()==0){
        QByteArray ba=path.toUtf8();
        qFatal("%s is empty", ba.constData());
    }


    const void *ptr= (const void *)file.map(0,file.size());
    if(!ptr){
        QByteArray ba=path.toUtf8();
        qFatal("failed to mmap %s", ba.constData());
    }
    return ptr;
}

STFont *STFont::defaultFont(){
    //return defaultBoldFont();
    static STFont *fnt=NULL;
    if(!fnt){
        fnt=new STFont((const uint8_t *)mapFile(":/stella/res/Sans11.ufbmp"),
                       (const uint32_t *)mapFile(":/stella/res/Sans11.ufindex"));
    }
    return fnt;
}
STFont *STFont::defaultBoldFont(){
    static STFont *fnt=NULL;
    if(!fnt){
        fnt=new STFont((const uint8_t *)mapFile(":/stella/res/SansBold11.ufbmp"),
                       (const uint32_t *)mapFile(":/stella/res/SansBold11.ufindex"));
    }
    return fnt;
}

bool STFont::isLeadingSurrogatePair(ushort ucs){
    return (ucs>=0xd800) && (ucs<0xdc00);
}

bool STFont::isTrailSurrogatePair(ushort ucs){
    return (ucs>=0xdc00) && (ucs<0xe000);
}

uint32_t STFont::decodeSurrogatePair(ushort c1, ushort c2){
    Q_ASSERT(isLeadingSurrogatePair(c1));
    Q_ASSERT(isTrailSurrogatePair(c2));
    return 0x10000+(uint32_t)(c2-0xdc00)+((uint32_t)(c1-0xd800)<<10);
}

struct STCombiningCharInfo{
    bool isCombiningChar;
    float axisX;
    enum VerticalPosition{
        Top=-1,
        Middle=0,
        Bottom=1
    } verticalPosition;
    enum HorizonalPosition{
        Left=-1,
        Center=0,
        Right=1
    } horizonalPosition;

    STCombiningCharInfo():
        isCombiningChar(false){}
    STCombiningCharInfo(float _axisX, VerticalPosition vPos,
                        HorizonalPosition hPos=Center):
        isCombiningChar(true),
        axisX(_axisX), verticalPosition(vPos),
        horizonalPosition(hPos){

    }
};

#define CCInfo STCombiningCharInfo

static STCombiningCharInfo combiningCharInfo(uint32_t ucs){
    if(ucs<0x100)return STCombiningCharInfo();
    switch(ucs){

    case 0x300: return CCInfo(0.5f, CCInfo::Top);
    case 0x301: return CCInfo(0.5f, CCInfo::Top);
    case 0x302: return CCInfo(0.5f, CCInfo::Top);
    case 0x303: return CCInfo(0.5f, CCInfo::Top);
    case 0x304: return CCInfo(0.5f, CCInfo::Top);
    case 0x305: return CCInfo(0.5f, CCInfo::Top);
    case 0x306: return CCInfo(0.5f, CCInfo::Top);
    case 0x307: return CCInfo(0.5f, CCInfo::Top);
    case 0x308: return CCInfo(0.5f, CCInfo::Top);
    case 0x309: return CCInfo(0.5f, CCInfo::Top);
    case 0x30a: return CCInfo(0.5f, CCInfo::Top);
    case 0x30b: return CCInfo(0.5f, CCInfo::Top);
    case 0x30c: return CCInfo(0.5f, CCInfo::Top);
    case 0x30d: return CCInfo(0.5f, CCInfo::Top);
    case 0x30e: return CCInfo(0.5f, CCInfo::Top);
    case 0x30f: return CCInfo(0.5f, CCInfo::Top);

    case 0x310: return CCInfo(0.5f, CCInfo::Top);
    case 0x311: return CCInfo(0.5f, CCInfo::Top);
    case 0x312: return CCInfo(0.5f, CCInfo::Top);
    case 0x313: return CCInfo(0.5f, CCInfo::Top);
    case 0x314: return CCInfo(0.5f, CCInfo::Top);
    case 0x315: return CCInfo(1.f, CCInfo::Top, CCInfo::Right);
    case 0x316: return CCInfo(0.5f, CCInfo::Bottom);
    case 0x317: return CCInfo(0.5f, CCInfo::Bottom);
    case 0x318: return CCInfo(0.5f, CCInfo::Bottom);
    case 0x319: return CCInfo(0.5f, CCInfo::Bottom);
    case 0x31a: return CCInfo(0.5f, CCInfo::Top);
    case 0x31b: return CCInfo(1.f, CCInfo::Top, CCInfo::Right);
    case 0x31c: return CCInfo(0.5f, CCInfo::Bottom);
    case 0x31d: return CCInfo(0.5f, CCInfo::Bottom);
    case 0x31e: return CCInfo(0.5f, CCInfo::Bottom);
    case 0x31f: return CCInfo(0.5f, CCInfo::Bottom);

    case 0x320: return CCInfo(0.5f, CCInfo::Bottom);
    case 0x321: return CCInfo(0.5f, CCInfo::Bottom);
    case 0x322: return CCInfo(0.5f, CCInfo::Bottom);
    case 0x323: return CCInfo(0.5f, CCInfo::Bottom);
    case 0x324: return CCInfo(0.5f, CCInfo::Bottom);
    case 0x325: return CCInfo(0.5f, CCInfo::Bottom);
    case 0x326: return CCInfo(0.5f, CCInfo::Bottom);
    case 0x327: return CCInfo(0.5f, CCInfo::Bottom);
    case 0x328: return CCInfo(0.5f, CCInfo::Bottom);
    case 0x329: return CCInfo(0.5f, CCInfo::Bottom);
    case 0x32a: return CCInfo(0.5f, CCInfo::Bottom);
    case 0x32b: return CCInfo(0.5f, CCInfo::Bottom);
    case 0x32c: return CCInfo(0.5f, CCInfo::Bottom);
    case 0x32d: return CCInfo(0.5f, CCInfo::Bottom);
    case 0x32e: return CCInfo(0.5f, CCInfo::Bottom);
    case 0x32f: return CCInfo(0.5f, CCInfo::Bottom);

    case 0x330: return CCInfo(0.5f, CCInfo::Bottom);
    case 0x331: return CCInfo(0.5f, CCInfo::Bottom);
    case 0x332: return CCInfo(0.5f, CCInfo::Bottom);
    case 0x333: return CCInfo(0.5f, CCInfo::Bottom);
    case 0x334: return CCInfo(0.5f, CCInfo::Middle);
    case 0x335: return CCInfo(0.5f, CCInfo::Middle);
    case 0x336: return CCInfo(0.5f, CCInfo::Middle);
    case 0x337: return CCInfo(0.5f, CCInfo::Middle);
    case 0x338: return CCInfo(0.5f, CCInfo::Middle);
    case 0x339: return CCInfo(0.5f, CCInfo::Bottom);
    case 0x33a: return CCInfo(0.5f, CCInfo::Bottom);
    case 0x33b: return CCInfo(0.5f, CCInfo::Bottom);
    case 0x33c: return CCInfo(0.5f, CCInfo::Bottom);
    case 0x33d: return CCInfo(0.5f, CCInfo::Top);
    case 0x33e: return CCInfo(0.5f, CCInfo::Top);
    case 0x33f: return CCInfo(0.5f, CCInfo::Top);

    case 0x340: return CCInfo(0.5f, CCInfo::Top);
    case 0x341: return CCInfo(0.5f, CCInfo::Top);
    case 0x342: return CCInfo(0.5f, CCInfo::Top);
    case 0x343: return CCInfo(0.5f, CCInfo::Top);
    case 0x344: return CCInfo(0.5f, CCInfo::Top);
    case 0x345: return CCInfo(0.5f, CCInfo::Bottom);
    case 0x346: return CCInfo(0.5f, CCInfo::Top);
    case 0x347: return CCInfo(0.5f, CCInfo::Bottom);
    case 0x348: return CCInfo(0.5f, CCInfo::Bottom);
    case 0x349: return CCInfo(0.5f, CCInfo::Bottom);
    case 0x34a: return CCInfo(0.5f, CCInfo::Top);
    case 0x34b: return CCInfo(0.5f, CCInfo::Top);
    case 0x34c: return CCInfo(0.5f, CCInfo::Top);
    case 0x34d: return CCInfo(0.5f, CCInfo::Bottom);
    case 0x34e: return CCInfo(0.5f, CCInfo::Bottom);
    case 0x34f: return CCInfo(0.5f, CCInfo::Bottom); // COMBINING GRAPHEME JOINER?

    case 0x350: return CCInfo(0.5f, CCInfo::Top);
    case 0x351: return CCInfo(0.5f, CCInfo::Top);
    case 0x352: return CCInfo(0.5f, CCInfo::Top);
    case 0x353: return CCInfo(0.5f, CCInfo::Bottom);
    case 0x354: return CCInfo(0.5f, CCInfo::Bottom);
    case 0x355: return CCInfo(0.5f, CCInfo::Bottom);
    case 0x356: return CCInfo(0.5f, CCInfo::Bottom);
    case 0x357: return CCInfo(0.5f, CCInfo::Top);
    case 0x358: return CCInfo(1.0f, CCInfo::Top, CCInfo::Right);
    case 0x359: return CCInfo(0.5f, CCInfo::Bottom);
    case 0x35a: return CCInfo(0.5f, CCInfo::Bottom);
    case 0x35b: return CCInfo(0.5f, CCInfo::Top);
    case 0x35c: return CCInfo(0.25f, CCInfo::Bottom);
    case 0x35d: return CCInfo(0.25f, CCInfo::Top);
    case 0x35e: return CCInfo(0.25f, CCInfo::Top);
    case 0x35f: return CCInfo(0.25f, CCInfo::Bottom);

    case 0x360: return CCInfo(0.25f, CCInfo::Top);
    case 0x361: return CCInfo(0.25f, CCInfo::Top);
    case 0x362: return CCInfo(0.25f, CCInfo::Bottom);
    case 0x363: return CCInfo(0.5f, CCInfo::Top);
    case 0x364: return CCInfo(0.5f, CCInfo::Top);
    case 0x365: return CCInfo(0.5f, CCInfo::Top);
    case 0x366: return CCInfo(0.5f, CCInfo::Top);
    case 0x367: return CCInfo(0.5f, CCInfo::Top);
    case 0x368: return CCInfo(0.5f, CCInfo::Top);
    case 0x369: return CCInfo(0.5f, CCInfo::Top);
    case 0x36a: return CCInfo(0.5f, CCInfo::Top);
    case 0x36b: return CCInfo(0.5f, CCInfo::Top);
    case 0x36c: return CCInfo(0.5f, CCInfo::Top);
    case 0x36d: return CCInfo(0.5f, CCInfo::Top);
    case 0x36e: return CCInfo(0.5f, CCInfo::Top);
    case 0x36f: return CCInfo(0.5f, CCInfo::Top);

    case 0x483: return CCInfo(0.5f, CCInfo::Top);
    case 0x484: return CCInfo(0.5f, CCInfo::Top);
    case 0x485: return CCInfo(0.5f, CCInfo::Top);
    case 0x486: return CCInfo(0.5f, CCInfo::Top);
    case 0x487: return CCInfo(0.5f, CCInfo::Top);
    case 0x488: return CCInfo(0.5f, CCInfo::Middle);
    case 0x489: return CCInfo(0.5f, CCInfo::Middle);

    case 0x7eb: return CCInfo(0.5f, CCInfo::Top);
    case 0x7ec: return CCInfo(0.5f, CCInfo::Top);
    case 0x7ed: return CCInfo(0.5f, CCInfo::Top);
    case 0x7ee: return CCInfo(0.5f, CCInfo::Top);
    case 0x7ef: return CCInfo(0.5f, CCInfo::Top);
    case 0x7f0: return CCInfo(0.5f, CCInfo::Top);
    case 0x7f1: return CCInfo(0.5f, CCInfo::Top);
    case 0x7f2: return CCInfo(0.5f, CCInfo::Bottom);
    case 0x7f3: return CCInfo(0.5f, CCInfo::Top);

    case 0x135f: return CCInfo(0.5f, CCInfo::Top);

    case 0x1b6b: return CCInfo(0.5f, CCInfo::Top);
    case 0x1b6c: return CCInfo(0.5f, CCInfo::Top);
    case 0x1b6d: return CCInfo(0.5f, CCInfo::Top);
    case 0x1b6e: return CCInfo(0.5f, CCInfo::Top);
    case 0x1b6f: return CCInfo(0.5f, CCInfo::Top);
    case 0x1b70: return CCInfo(0.5f, CCInfo::Top);
    case 0x1b71: return CCInfo(0.5f, CCInfo::Top);
    case 0x1b72: return CCInfo(0.5f, CCInfo::Top);
    case 0x1b73: return CCInfo(0.5f, CCInfo::Top);

    case 0x1dc0: return CCInfo(0.5f, CCInfo::Top);
    case 0x1dc1: return CCInfo(0.5f, CCInfo::Top);
    case 0x1dc2: return CCInfo(0.5f, CCInfo::Bottom);
    case 0x1dc3: return CCInfo(0.5f, CCInfo::Top);
    case 0x1dc4: return CCInfo(0.5f, CCInfo::Top);
    case 0x1dc5: return CCInfo(0.5f, CCInfo::Top);
    case 0x1dc6: return CCInfo(0.5f, CCInfo::Top);
    case 0x1dc7: return CCInfo(0.5f, CCInfo::Top);
    case 0x1dc8: return CCInfo(0.5f, CCInfo::Top);
    case 0x1dc9: return CCInfo(0.5f, CCInfo::Top);
    case 0x1dca: return CCInfo(0.5f, CCInfo::Bottom);
    case 0x1dcb: return CCInfo(0.5f, CCInfo::Top);
    case 0x1dcc: return CCInfo(0.5f, CCInfo::Top);
    case 0x1dcd: return CCInfo(0.5f, CCInfo::Top);
    case 0x1dce: return CCInfo(0.5f, CCInfo::Top);
    case 0x1dcf: return CCInfo(0.5f, CCInfo::Bottom);

    case 0x1dd0: return CCInfo(0.5f, CCInfo::Bottom);
    case 0x1dd1: return CCInfo(0.5f, CCInfo::Top);
    case 0x1dd2: return CCInfo(0.5f, CCInfo::Top);
    case 0x1dd3: return CCInfo(0.5f, CCInfo::Top);
    case 0x1dd4: return CCInfo(0.5f, CCInfo::Top);
    case 0x1dd5: return CCInfo(0.5f, CCInfo::Top);
    case 0x1dd6: return CCInfo(0.5f, CCInfo::Top);
    case 0x1dd7: return CCInfo(0.5f, CCInfo::Top);
    case 0x1dd8: return CCInfo(0.5f, CCInfo::Top);
    case 0x1dd9: return CCInfo(0.5f, CCInfo::Top);
    case 0x1dda: return CCInfo(0.5f, CCInfo::Top);
    case 0x1ddb: return CCInfo(0.5f, CCInfo::Top);
    case 0x1ddc: return CCInfo(0.5f, CCInfo::Top);
    case 0x1ddd: return CCInfo(0.5f, CCInfo::Top);
    case 0x1dde: return CCInfo(0.5f, CCInfo::Top);
    case 0x1ddf: return CCInfo(0.5f, CCInfo::Top);

    case 0x1de0: return CCInfo(0.5f, CCInfo::Top);
    case 0x1de1: return CCInfo(0.5f, CCInfo::Top);
    case 0x1de2: return CCInfo(0.5f, CCInfo::Top);
    case 0x1de3: return CCInfo(0.5f, CCInfo::Top);
    case 0x1de4: return CCInfo(0.5f, CCInfo::Top);
    case 0x1de5: return CCInfo(0.5f, CCInfo::Top);
    case 0x1de6: return CCInfo(0.5f, CCInfo::Top);

    case 0x1dfe: return CCInfo(0.5f, CCInfo::Top);
    case 0x1dff: return CCInfo(0.5f, CCInfo::Bottom);

    case 0x20d0: return CCInfo(0.5f, CCInfo::Top);
    case 0x20d1: return CCInfo(0.5f, CCInfo::Top);
    case 0x20d2: return CCInfo(0.5f, CCInfo::Middle);
    case 0x20d3: return CCInfo(0.5f, CCInfo::Middle);
    case 0x20d4: return CCInfo(0.5f, CCInfo::Top);
    case 0x20d5: return CCInfo(0.5f, CCInfo::Top);
    case 0x20d6: return CCInfo(0.5f, CCInfo::Top);
    case 0x20d7: return CCInfo(0.5f, CCInfo::Top);
    case 0x20d8: return CCInfo(0.5f, CCInfo::Middle);
    case 0x20d9: return CCInfo(0.5f, CCInfo::Middle);
    case 0x20da: return CCInfo(0.5f, CCInfo::Middle);
    case 0x20db: return CCInfo(0.5f, CCInfo::Top);
    case 0x20dc: return CCInfo(0.5f, CCInfo::Top);
    case 0x20dd: return CCInfo(0.5f, CCInfo::Middle);
    case 0x20de: return CCInfo(0.5f, CCInfo::Middle);
    case 0x20df: return CCInfo(0.5f, CCInfo::Middle);

    case 0x20e0: return CCInfo(0.5f, CCInfo::Middle);
    case 0x20e1: return CCInfo(0.5f, CCInfo::Top);
    case 0x20e2: return CCInfo(0.5f, CCInfo::Middle);
    case 0x20e3: return CCInfo(0.5f, CCInfo::Middle);
    case 0x20e4: return CCInfo(0.5f, CCInfo::Middle);
    case 0x20e5: return CCInfo(0.5f, CCInfo::Middle);
    case 0x20e6: return CCInfo(0.5f, CCInfo::Middle);
    case 0x20e7: return CCInfo(0.9f, CCInfo::Middle, CCInfo::Right);
    case 0x20e8: return CCInfo(0.5f, CCInfo::Bottom);
    case 0x20e9: return CCInfo(0.5f, CCInfo::Top);
    case 0x20ea: return CCInfo(0.5f, CCInfo::Middle);
    case 0x20eb: return CCInfo(0.5f, CCInfo::Middle);
    case 0x20ec: return CCInfo(0.5f, CCInfo::Bottom);
    case 0x20ed: return CCInfo(0.5f, CCInfo::Bottom);
    case 0x20ee: return CCInfo(0.5f, CCInfo::Bottom);
    case 0x20ef: return CCInfo(0.5f, CCInfo::Bottom);

    case 0x20f0: return CCInfo(0.5f, CCInfo::Top);

    case 0x3099: return CCInfo(1.0f, CCInfo::Top, CCInfo::Right);
    case 0x309a: return CCInfo(1.0f, CCInfo::Top, CCInfo::Right);

    case 0xa66f: return CCInfo(0.5f, CCInfo::Top);
    case 0xa670: return CCInfo(0.5f, CCInfo::Middle);
    case 0xa671: return CCInfo(0.5f, CCInfo::Middle);
    case 0xa672: return CCInfo(0.5f, CCInfo::Middle);

    case 0xfe20: return CCInfo(0.5f, CCInfo::Top);
    case 0xfe21: return CCInfo(0.5f, CCInfo::Top);
    case 0xfe22: return CCInfo(0.5f, CCInfo::Top);
    case 0xfe23: return CCInfo(0.5f, CCInfo::Top);
    case 0xfe24: return CCInfo(0.5f, CCInfo::Top);
    case 0xfe25: return CCInfo(0.5f, CCInfo::Top);
    case 0xfe26: return CCInfo(0.5f, CCInfo::Top);

        // Thai (looks like combining, but not "combining char"?)
        // usage example: Tofu's "mayuge" kaomoji

    case 0x0e31: return CCInfo(0.5f, CCInfo::Top);
    case 0x0e34: return CCInfo(0.5f, CCInfo::Top);
    case 0x0e35: return CCInfo(0.5f, CCInfo::Top);
    case 0x0e36: return CCInfo(0.5f, CCInfo::Top);
    case 0x0e37: return CCInfo(0.5f, CCInfo::Top);
    case 0x0e38: return CCInfo(0.5f, CCInfo::Bottom);
    case 0x0e39: return CCInfo(0.5f, CCInfo::Bottom);
    case 0x0e3a: return CCInfo(0.5f, CCInfo::Bottom);
    case 0x0e47: return CCInfo(0.5f, CCInfo::Top);
    case 0x0e48: return CCInfo(0.5f, CCInfo::Top);
    case 0x0e49: return CCInfo(0.5f, CCInfo::Top);
    case 0x0e4a: return CCInfo(0.5f, CCInfo::Top);
    case 0x0e4b: return CCInfo(0.5f, CCInfo::Top);
    case 0x0e4c: return CCInfo(0.5f, CCInfo::Top);
    case 0x0e4d: return CCInfo(0.5f, CCInfo::Top);
    case 0x0e4e: return CCInfo(0.5f, CCInfo::Top);

    default: return CCInfo();
    }
}

static bool isWhitespace(uint32_t ucs){
    switch(ucs){
    case 0x20:
    case 0x3000:
    case 0x2002:
    case 0x2003:
    case 0x2004:
    case 0x2005:
    case 0x2006:
    case 0x2007:
    case 0x2008:
    case 0x2009:
    case 0x200a:
    case 0x200b:
    case 0x205f:
        return true;
    }
    return false;
}

static bool isKinsokuStart(uint32_t ucs){
    switch(ucs){
    case 0x2c: case 0x29: case 0x5d: case 0xff5d:
    case 0x3001:    case 0x3015:    case 0x3009:    case 0x300b:
    case 0x300d:    case 0x300f:    case 0x3011:    case 0x3019:
    case 0x3017:    case 0x301f:    case 0x2019:    case 0x201d:
    case 0xff60:    case 0x00bb:
    case 0x30fd:    case 0x30fe:    case 0x30fc:    case 0x30a1:
    case 0x30a3:    case 0x30a5:    case 0x30a7:    case 0x30a9:
    case 0x30c3:    case 0x30e3:    case 0x30e5:    case 0x30e7:
    case 0x30ee:    case 0x30f5:    case 0x30f6:    case 0x3041:
    case 0x3043:    case 0x3045:    case 0x3047:    case 0x3049:
    case 0x3063:    case 0x3083:    case 0x3085:    case 0x3087:
    case 0x308e:    case 0x3095:    case 0x3096:    case 0x31f0:
    case 0x31f1:    case 0x31f2:    case 0x31f3:    case 0x31f4:
    case 0x31f5:    case 0x31f6:    case 0x31f7:    case 0x31f8:
    case 0x31f9:    case 0x31fa:    case 0x31fb:    case 0x31fc:
    case 0x31fd:    case 0x31fe:    case 0x31ff:    case 0x3005:
    case 0x303b:
    case 0x2010:    case 0x30a0:    case 0x2013:    case 0x301c:
    case 0x003f:    case 0x0021:    case 0x203c:    case 0x2047:
    case 0x2048:    case 0x2049:
    case 0x30fb:    case 0x003a:    case 0x003b:
    case 0x3002:    case 0x002e:
    case 0xff1f:    case 0xff0e:
        return true;
    }
    return false;
}

static bool isKinsokuEnd(uint32_t ucs){
    switch(ucs){
    case 0x28:      case 0x5b:      case 0xff5b:    case 0x3014:
    case 0x3008:    case 0x300a:    case 0x300c:    case 0x300e:
    case 0x3010:    case 0x3018:    case 0x3016:    case 0x301d:
    case 0x2018:    case 0x201c:    case 0xff5f:    case 0x00ab:
        return true;
    }
    return false;
}

static bool isLatin(uint32_t ucs){
    if(ucs>='0' && ucs<='9') return true;
    if(ucs>='A' && ucs<='Z') return true;
    if(ucs>='a' && ucs<='z') return true;
    if(ucs=='\'')return true;
    return false;
}

#undef CCInfo

STFont::Layout STFont::layoutString(const QString& str, float maxWidth, bool singleLine){
    Layout outLayout;
    unsigned int pos=0;
    unsigned int length=(unsigned int)str.size();
    const QChar *chars=str.constData();

    float xPos=0.f;
    int line=0;

    int lastCharIndex=-1;
    float lastCharMinY;
    float lastCharMaxY;
    float lastCharMinX;
    float lastCharMaxX;

    static const uint32_t ellipsisChar=0x2026;

    if(singleLine){
        float glyphWidth=float(m_bitmapData[m_indexData[ellipsisChar+1]])*.5f;
        maxWidth-=glyphWidth;
    }

    outLayout.reserve(str.size()*2+1);

    while(pos<length){
        uint32_t ucs=chars[pos].unicode();
        LayoutGlyph glyph;
        bool singleLineSpaceRanOut=false;

        glyph.charIndex=pos;
        glyph.combining=false;
        glyph.line=line;
        glyph.pseudoNewLine=false;
        glyph.highlighted=false;

        if(pos<length-1){
            // is this surrogate pair?
            if(isLeadingSurrogatePair(chars[pos].unicode()) &&
                isTrailSurrogatePair(chars[pos+1].unicode())){
                ucs=decodeSurrogatePair(chars[pos].unicode(),
                                        chars[pos+1].unicode());
            }
        }
        if(ucs>=0x10000){
            glyph.charLength=2;
            pos+=2;
        }else{
            glyph.charLength=1;
            pos++;
        }

        if(ucs=='\n'){
            // new line
            glyph.glyphId=-1;
            if(singleLine){
                ucs=(uint32_t)' ';
                glyph.glyphId=ucs;
            }
        }else{
            if(ucs>=m_indexData[0]){
                glyph.glyphId=-1;
            }else if((int32_t)m_indexData[ucs+1]==-1){
                glyph.glyphId=-1;
            }else{
                glyph.glyphId=ucs;
            }
        }

        glyph.ucs=ucs;

        STCombiningCharInfo combInfo=combiningCharInfo(ucs);
        if(lastCharIndex==-1 || glyph.glyphId==-1){
            combInfo.isCombiningChar=false;
        }

        //uint v=ucs;
        //QString s=QString::fromUcs4(&v, 1);
        //printf("U+%08X [ あ%s ] : %d\n",(unsigned int)ucs, s.toUtf8().data(), combInfo.isCombiningChar?1:0);

        if(combInfo.isCombiningChar){

            Q_ASSERT(glyph.glyphId!=-1);
            //Glyph& lastChar=outLayout[lastCharIndex];
            uint32_t addr=m_indexData[ucs+1];
            float glyphMinY=(m_bitmapData+addr)[1]&0xf;
            float glyphMaxY=((m_bitmapData+addr)[1])>>4;
            float glyphWidth=float((m_bitmapData+addr)[0])*.5f;

            float gxPos, gyPos;

            if(combInfo.horizonalPosition==STCombiningCharInfo::Left){
                gxPos=lastCharMinX;
            }else if(combInfo.horizonalPosition==STCombiningCharInfo::Center){
                gxPos=(lastCharMinX+lastCharMaxX)*0.5f;
            }else if(combInfo.horizonalPosition==STCombiningCharInfo::Right){
                gxPos=lastCharMaxX;
            }else{Q_ASSERT(false);}
            gxPos-=glyphWidth*combInfo.axisX;

            if(combInfo.verticalPosition==STCombiningCharInfo::Top){
                gyPos=lastCharMinY-glyphMaxY;
            }else if(combInfo.verticalPosition==STCombiningCharInfo::Middle){
                gyPos=(lastCharMinY+lastCharMaxY)*.5f;
                gyPos-=(glyphMinY+glyphMaxY)*.5f;
            }else if(combInfo.verticalPosition==STCombiningCharInfo::Bottom){
                gyPos=lastCharMaxY-glyphMinY;
            }

            if(gxPos<lastCharMinX) lastCharMinX=gxPos;
            if(gxPos+glyphWidth>lastCharMaxX) lastCharMaxX=gxPos+glyphWidth;
            if(gyPos+glyphMinY<lastCharMinY) lastCharMinY=gyPos+glyphMinY;
            if(gyPos+glyphMaxY>lastCharMaxY) lastCharMaxY=gyPos+glyphMaxY;

            glyph.rect=QRectF(gxPos, gyPos, glyphWidth, 12.f);
            glyph.combining=true;
            outLayout.push_back(glyph);

        }else{

            float yPos=(float)(line*m_lineHeight);
            float width=0.f;
            float glyphMinY=0;
            float glyphMaxY=12;

            if(glyph.glyphId!=-1){
                // this char is visible.
                uint32_t addr=m_indexData[ucs+1];
                uint8_t widthInt=m_bitmapData[addr];
                width=(float)widthInt*0.5f;
                glyphMinY=m_bitmapData[addr+1]&0xf;
                glyphMaxY=(m_bitmapData[addr+1])>>4;
            }

            if(xPos+width>maxWidth && xPos!=0.f && width!=0.f){
                if(singleLine){
                    // substitute with ellipsis, and end layout.
                    ucs=ellipsisChar;
                    glyph.ucs=ucs;
                    glyph.glyphId=ucs;

                    uint32_t addr=m_indexData[ucs+1];
                    uint8_t widthInt=m_bitmapData[addr];
                    width=(float)widthInt*0.5f;
                    glyphMinY=m_bitmapData[addr+1]&0xf;
                    glyphMaxY=(m_bitmapData[addr+1])>>4;

                    singleLineSpaceRanOut=true;
                }else if(isWhitespace(ucs)){
                    // act line newline
                    width=0.f;
                    line++;
                    xPos=0.f;
                    glyph.glyphId=-1;
                    glyph.pseudoNewLine=true;
                }else if(isKinsokuStart(ucs)){
                    // cannot break here.
                    // bring more prior chars to keep the rule
                    int j=outLayout.size()-1;
                    while(j>=0){
                        LayoutGlyph& g=outLayout[j];
                        if(!g.combining){
                            if(g.ucs=='\n' || g.pseudoNewLine){
                                goto giveupKinsoku;
                            }
                            if(!isKinsokuStart(g.ucs)){
                                // found...?
                                // there can be kinsoku-end char just before.

                                int k=j-1;
                                while(k>=0){
                                    LayoutGlyph& g2=outLayout[k];
                                    if(g2.combining){
                                        k--; continue;
                                    }
                                    if(isKinsokuEnd(g2.ucs)){
                                        // kinsoku-end!!
                                        j=k;
                                    }else{
                                        // not found.
                                        break;
                                    }
                                    k--;
                                }

                                break;
                            }
                        }
                        j--;
                    }
                    if(j<0) goto giveupKinsoku;
                    if(outLayout[j].rect.x()==0.f){
                        goto giveupKinsoku;
                    }

                    QPointF delta(-outLayout[j].rect.x(), m_lineHeight);
                    for(int k=j;k<outLayout.size();k++){
                        LayoutGlyph& g=outLayout[k];
                        g.rect.translate(delta);
                        g.line++;
                    }

                    // there let be pseudo-newline
                   /* LayoutGlyph pseudoNewline;
                    pseudoNewline.charIndex=glyph.charIndex;
                    pseudoNewline.charLength=0;
                    pseudoNewline.combining=false;
                    pseudoNewline.glyphId=-1;
                    pseudoNewline.line=line;
                    pseudoNewline.pseudoNewLine=true;
                    pseudoNewline.rect=QRect(xPos, yPos, 0.f, 12.f);
                    pseudoNewline.ucs=0;
                    outLayout.push_back(pseudoNewline);*/

                    line++;
                    glyph.line++;
                    xPos+=delta.x();
                    yPos+=m_lineHeight;
                }else if(isLatin(glyph.ucs)){
                    // latin should be word-wrapped.
                    int j=outLayout.size()-1;
                    int k=j+1;
                    bool wasKinsokuEnd=false;
                    while(j>=0){
                        LayoutGlyph& g=outLayout[j];
                        if(!isLatin(g.ucs) || wasKinsokuEnd){
                            if(!isKinsokuEnd(g.ucs)){
                                break;
                            }else{
                                wasKinsokuEnd=true;
                            }
                        }else{
                            wasKinsokuEnd=false;
                        }
                        k=j;
                        j--;
                    }
                    j=k;

                    if(j<0) goto giveupKinsoku;
                    if(j==outLayout.size()) goto giveupKinsoku;
                    if(outLayout[j].rect.x()==0.f){
                        goto giveupKinsoku;
                    }

                    QPointF delta(-outLayout[j].rect.x(), m_lineHeight);
                    for(int k=j;k<outLayout.size();k++){
                        LayoutGlyph& g=outLayout[k];
                        g.rect.translate(delta);
                        g.line++;
                    }

                    // there let be pseudo-newline
                    /*LayoutGlyph pseudoNewline;
                    pseudoNewline.charIndex=glyph.charIndex;
                    pseudoNewline.charLength=0;
                    pseudoNewline.combining=false;
                    pseudoNewline.glyphId=-1;               // TODO: something goes wrong
                    pseudoNewline.line=line;
                    pseudoNewline.pseudoNewLine=true;
                    pseudoNewline.rect=QRect(xPos, yPos, 0.f, 12.f);
                    pseudoNewline.ucs=0;
                    outLayout.push_back(pseudoNewline);*/

                    line++;
                    glyph.line++;
                    xPos+=delta.x();
                    yPos+=m_lineHeight;
                }else{

                    {
                        int j=outLayout.size()-1;
                        int k=j+1;
                        bool wasKinsokuEnd=false, wasLatin=false;
                        while(j>=0){
                            LayoutGlyph& g=outLayout[j];
                            if((!isKinsokuEnd(g.ucs) || wasLatin) &&
                                    (!isLatin(g.ucs) || wasKinsokuEnd)){
                                break;
                            }
                            wasKinsokuEnd=isKinsokuEnd(g.ucs);
                            wasLatin=isLatin(g.ucs);
                            k=j;
                            j--;
                        }
                        j=k;

                        if(j<0) goto giveupKinsoku;
                        if(j==outLayout.size()) goto giveupKinsoku;
                        if(outLayout[j].rect.x()==0.f){
                            goto giveupKinsoku;
                        }

                        QPointF delta(-outLayout[j].rect.x(), m_lineHeight);
                        for(int k=j;k<outLayout.size();k++){
                            LayoutGlyph& g=outLayout[k];
                            g.rect.translate(delta);
                            g.line++;
                        }

                        line++;
                        glyph.line++;
                        xPos+=delta.x();
                        yPos+=m_lineHeight;
                    }

                    goto noPseudoNewline;

giveupKinsoku:
                    {
                        // there let be pseudo-newline
                        LayoutGlyph pseudoNewline;
                        pseudoNewline.charIndex=glyph.charIndex;
                        pseudoNewline.charLength=0;
                        pseudoNewline.combining=false;
                        pseudoNewline.glyphId=-1;
                        pseudoNewline.line=line;
                        pseudoNewline.pseudoNewLine=true;
                        pseudoNewline.highlighted=false;
                        pseudoNewline.rect=QRect(xPos, yPos, 0.f, 12.f);
                        pseudoNewline.ucs=0;
                        outLayout.push_back(pseudoNewline);

                        xPos=0.f;
                        line++;
                        yPos+=m_lineHeight;
                        glyph.line++;
                    }

noPseudoNewline:    ;
                }
            }

            // TODO: embeding adjust map in font data?
            float yAdjust=0.f;
            if(ucs>=0x21 && ucs<=0x7a){ // latin
                yAdjust=1.f;
            }else if(ucs>=0x370 && ucs<0x590){ // greek/coptic/cyrillic
                yAdjust=1.f;
            }else if(ucs>=0x1e00 && ucs<0x2000){// latin with accents
                // when accent is in the top, char becomes +(0,1) because of font converter.
                // but we want all the latin adjusted +(0, 1).
                // so adjust only characters with accent in the bottom to +(0, 1).
                switch(ucs){
                case 0x1e00:    case 0x1e01:    case 0x1e04:    case 0x1e05:
                case 0x1e06:    case 0x1e07:    case 0x1e08:    case 0x1e09:
                case 0x1e0c:    case 0x1e0d:    case 0x1e0e:    case 0x1e0f:
                case 0x1e10:    case 0x1e11:    case 0x1e12:    case 0x1e13:
                case 0x1e18:    case 0x1e19:    case 0x1e1a:    case 0x1e1b:
                case 0x1e1d:    case 0x1e24:    case 0x1e25:    case 0x1e28:
                case 0x1e29:    case 0x1e2a:    case 0x1e2b:    case 0x1e2c:
                case 0x1e2d:    case 0x1e32:    case 0x1e33:    case 0x1e34:
                case 0x1e35:    case 0x1e36:    case 0x1e37:    case 0x1e3a:
                case 0x1e3b:    case 0x1e3c:    case 0x1e3d:    case 0x1e42:
                case 0x1e43:    case 0x1e46:    case 0x1e47:    case 0x1e48:
                case 0x1e49:    case 0x1e4a:    case 0x1e4b:    case 0x1e55:
                case 0x1e57:    case 0x1e59:    case 0x1e5a:    case 0x1e5b:
                case 0x1e5c:    case 0x1e5d:    case 0x1e5e:    case 0x1e5f:
                case 0x1e61:    case 0x1e62:    case 0x1e63:    case 0x1e65:
                case 0x1e67:    case 0x1e69:    case 0x1e6c:    case 0x1e6d:
                case 0x1e6e:    case 0x1e6f:    case 0x1e70:    case 0x1e71:
                case 0x1e72:    case 0x1e73:    case 0x1e74:    case 0x1e75:
                case 0x1e76:    case 0x1e77:    case 0x1e7d:    case 0x1e7e:
                case 0x1e7f:    case 0x1e81:    case 0x1e83:    case 0x1e85:
                case 0x1e87:    case 0x1e88:    case 0x1e89:    case 0x1e8b:
                case 0x1e8d:    case 0x1e8f:    case 0x1e91:    case 0x1e92:
                case 0x1e93:    case 0x1e94:    case 0x1e95:    case 0x1e96:
                case 0x1e97:    case 0x1e98:    case 0x1e99:    case 0x1e9a:
                case 0x1e9c:    case 0x1e9d:
                case 0x1ea0:    case 0x1ea1:    case 0x1ea3:    case 0x1eab:
                case 0x1eb6:    case 0x1eb7:    case 0x1eb8:    case 0x1eb9:
                case 0x1ebb:    case 0x1ebd:    case 0x1ec1:    case 0x1ec7:
                case 0x1eca:    case 0x1ecc:    case 0x1ecd:    case 0x1ecf:
                case 0x1ed1:    case 0x1ed3:    case 0x1ed5:    case 0x1ed7:
                case 0x1ed9:    case 0x1edb:    case 0x1edd:    case 0x1edf:
                case 0x1ee1:    case 0x1ee2:    case 0x1ee3:    case 0x1ee4:
                case 0x1ee5:    case 0x1ee7:    case 0x1ee9:    case 0x1eeb:
                case 0x1eed:    case 0x1eef:    case 0x1ef0:    case 0x1ef1:
                case 0x1ef2:    case 0x1ef3:    case 0x1ef4:    case 0x1ef5:
                case 0x1ef6:    case 0x1ef7:    case 0x1ef8:    case 0x1ef9:
                    yAdjust=1.f;
                    break;
                    // TODO: greek extended
                }
            }
            if(ucs=='@'){
                yAdjust-=1.f;
            }

            glyph.rect=QRectF(xPos, yPos+yAdjust, width, 12.f);

            outLayout.push_back(glyph);
            if(glyph.glyphId!=-1){
                lastCharIndex=outLayout.size()-1;
                lastCharMinY=(float)glyph.rect.top()+glyphMinY;
                lastCharMaxY=(float)glyph.rect.top()+glyphMaxY;
                lastCharMinX=glyph.rect.left();
                lastCharMaxX=glyph.rect.right();
            }

            if(singleLineSpaceRanOut){
                // ???: eof layout glyph is not needed?
                return outLayout;
            }

            if(ucs=='\n'){
                xPos=0.f;
                lastCharIndex=-1;
                line++;
            }else{
                xPos+=width;
            }

        }
    }

    // something like eof
    {
        LayoutGlyph gl;
        float yPos=(float)(line*m_lineHeight);
        gl.charIndex=length;
        gl.charLength=0;
        gl.glyphId=-1;
        gl.line=line;
        gl.ucs=0;
        gl.rect=QRectF(xPos, yPos, 0.f, 12.f);
        gl.highlighted=false;
        gl.pseudoNewLine=false;

        outLayout.push_back(gl);
    }
    return outLayout;
}

void STFont::drawLayout(QImage& image, const QPointF& pos,
                const QColor& color,
                const Layout& layout, const DrawOption& options){
    if(image.isNull()) return;
    if(color.alpha()==0 &&
            (options.highlightEndIndex<=options.highlightStartIndex ||
             options.highlightColor.alpha()==0)) return;
    if(layout.isEmpty()) return;

    unsigned int length=layout.size();

    for(unsigned int i=0;i<length;i++){
        const LayoutGlyph& glyph=layout[i];
        if(glyph.glyphId==-1)continue;
        QColor col=(glyph.highlighted ||
                    (glyph.charIndex>=options.highlightStartIndex &&
                    glyph.charIndex<options.highlightEndIndex))?
                    options.highlightColor:color;

        QPointF glyphPos(pos.x()+glyph.rect.left(),
                         pos.y()+glyph.rect.top());

        drawGlyph(image, glyphPos, col, m_bitmapData+m_indexData[glyph.glyphId+1],
                  options);
    }

}

void STFont::drawString(QImage& image, const QPointF& pos,
                const QColor& color, const QString& str, const DrawOption&opt){
    drawLayout(image, pos, color, layoutString(str), opt);
}

QRectF STFont::visualBoundingRectForLayout(const Layout & layout){
    QRectF boundRect(0.f,0.f,0.f,0.f);
    foreach(const LayoutGlyph& glyph, layout){
        QRectF rt=visualBoundingRectForLayoutGlyph(glyph);
        if(rt.isEmpty()) continue;
        if(boundRect.height()==0.f){
            boundRect=rt;
        }else{
            boundRect=boundRect.united(rt);
        }
    }
    return boundRect;
}

QRectF STFont::visualBoundingRectForString(const QString &str){
    return visualBoundingRectForLayout(layoutString(str));
}

QRectF STFont::boundingRectForLayout(const Layout & layout){
    QRectF boundRect(0.f,0.f,0.f,0.f);
    foreach(const LayoutGlyph& glyph, layout){
        if(glyph.combining)continue;
        if(boundRect.height()==0.f){
            boundRect=glyph.rect;
        }else{
            boundRect=boundRect.united(glyph.rect);
        }
    }
    return boundRect;
}

QRectF STFont::boundingRectForString(const QString &str){
    return boundingRectForLayout(layoutString(str));
}

QRectF STFont::visualBoundingRectForLayoutGlyph(const LayoutGlyph &glyph){
    if(glyph.glyphId==-1){
        return QRectF(glyph.rect.x(), glyph.rect.y()+11.f, 0.f, 0.f);
    }else{
        uint32_t addr=m_indexData[glyph.glyphId+1];
        float widthInt=m_bitmapData[addr];
        float glyphMinY=m_bitmapData[addr+1]&0xf;
        float glyphMaxY=(m_bitmapData[addr+1])>>4;
        return QRectF(glyph.rect.x(), glyph.rect.y()+glyphMinY,
                      (float)widthInt*0.5f,glyphMaxY-glyphMinY);
    }
}

static bool compareLayoutGlyphWithLine(const STFont::LayoutGlyph& a,
                                       const STFont::LayoutGlyph& b){
    return a.line<b.line;
}

static STFont::LayoutGlyph layoutGlyphWithLine(int line){
    STFont::LayoutGlyph glyph;
    glyph.line=line;
    return glyph;
}

int STFont::cursorPosAtPoint(const Layout& layout , const QPointF& pt){
    int ptLine=(pt.y()+(m_lineHeight-12)/2)/m_lineHeight;
    int minLine=layout[0].line;
    int maxLine=layout.last().line;
    if(ptLine<minLine)ptLine=minLine;
    if(ptLine>maxLine)ptLine=maxLine;

    int index=qLowerBound(layout.begin(), layout.end(), layoutGlyphWithLine(ptLine),
                               compareLayoutGlyphWithLine)-layout.begin();

    if(pt.x()<0.f) return layout[index].charIndex;

    float lastX=0.f;
    int lastInd=index;

    while(index<layout.size()){
        const LayoutGlyph& glyph=layout[index];
        if(glyph.combining){
            index++;
            continue;
        }
        if((int)glyph.line>ptLine){
            index--;
            break;
        }

        if(pt.x()<glyph.rect.left()){
            float middle=(lastX+glyph.rect.left())*.5f;
            if(pt.x()<middle){
                return layout[lastInd].charIndex;
            }else{
                return glyph.charIndex;
            }
        }

        lastX=glyph.rect.left();
        lastInd=index;
        index++;
    }


    if(index==layout.size()){
        return layout[layout.size()-1].charIndex;
    }else{
        return layout[index].charIndex;
    }

}

struct STFontColor{
    uint8_t r,g,b,a;
    STFontColor(){}
    STFontColor(const QColor& col){
        r=col.red();
        g=col.green();
        b=col.blue();
        a=col.alpha();
    }
};

#define STFontBytesPerColumn 6

template<QImage::Format ImageFormat>
static void drawGrayscalePixel(uchar *imagePixel,
                      const STFontColor& color,
                      unsigned int alphaMul){
    alphaMul=(alphaMul*color.a)>>8;
    alphaMul+=alphaMul>>7; // 0-255 -> 0-256
    if(ImageFormat==QImage::Format_ARGB32){
        unsigned int imageRed=imagePixel[2];
        unsigned int imageGreen=imagePixel[1];
        unsigned int imageBlue=imagePixel[0];
        unsigned int imageAlpha=imagePixel[3];
        if(imageAlpha==0){
            imagePixel[0]=color.b;
            imagePixel[1]=color.g;
            imagePixel[2]=color.r;
            imagePixel[3]=(alphaMul*255)>>8;
            return;
        }else if(imageAlpha==255){
            imageRed+=(((int)color.r-(int)imageRed)*(int)alphaMul+128)>>8;
            imageGreen+=(((int)color.g-(int)imageGreen)*(int)alphaMul+128)>>8;
            imageBlue+=(((int)color.b-(int)imageBlue)*(int)alphaMul+128)>>8;
            imagePixel[0]=imageBlue;
            imagePixel[1]=imageGreen;
            imagePixel[2]=imageRed;
            return;
        }
        unsigned int alpha=255-(((255-imageAlpha)*(256-alphaMul))>>8);
        if(alpha==0){
            imagePixel[3]=0;
            return;
        }

        unsigned int blue=imageBlue*imageAlpha*(256-alphaMul);
        blue+=((unsigned int)color.b*alphaMul+128)<<8;
        blue=(blue/alpha+128)>>8;
        if(blue>255) blue=255;
        imagePixel[0]=blue;

        unsigned int green=imageGreen*imageAlpha*(256-alphaMul);
        green+=((unsigned int)color.g*alphaMul+128)<<8;
        green=(green/alpha+128)>>8;
        if(green>255) green=255;
        imagePixel[1]=green;

        unsigned int red=imageRed*imageAlpha*(256-alphaMul);
        red+=((unsigned int)color.r*alphaMul+128)<<8;
        red=(red/alpha+128)>>8;
        if(red>255) red=255;
        imagePixel[2]=red;

        imagePixel[3]=alpha;
    }else if(ImageFormat==QImage::Format_ARGB32_Premultiplied){
        unsigned int imageRed=imagePixel[2];
        unsigned int imageGreen=imagePixel[1];
        unsigned int imageBlue=imagePixel[0];
        unsigned int imageAlpha=imagePixel[3];
        if(imageAlpha==0){
            imagePixel[0]=(color.b*alphaMul)>>8;
            imagePixel[1]=(color.g*alphaMul)>>8;
            imagePixel[2]=(color.r*alphaMul)>>8;
            imagePixel[3]=(alphaMul*255)>>8;
            return;
        }else if(imageAlpha==255){
            imageRed+=(((int)color.r-(int)imageRed)*(int)alphaMul+128)>>8;
            imageGreen+=(((int)color.g-(int)imageGreen)*(int)alphaMul+128)>>8;
            imageBlue+=(((int)color.b-(int)imageBlue)*(int)alphaMul+128)>>8;
            imagePixel[0]=imageBlue;
            imagePixel[1]=imageGreen;
            imagePixel[2]=imageRed;
            return;
        }
        unsigned int alpha=255-(((255-imageAlpha)*(256-alphaMul))>>8);
        if(alpha==0){
            imagePixel[3]=0;
            return;
        }

        unsigned int blue=imageBlue*(256-alphaMul);
        blue+=((unsigned int)color.b*alphaMul);
        blue=(blue+128)>>8;
        if(blue>alpha) blue=alpha;
        imagePixel[0]=blue;

        unsigned int green=imageGreen*(256-alphaMul);
        green+=((unsigned int)color.g*alphaMul);
        green=(green+128)>>8;
        if(green>alpha) green=alpha;
        imagePixel[1]=green;

        unsigned int red=imageRed*(256-alphaMul);
        red+=((unsigned int)color.r*alphaMul);
        red=(red+128)>>8;
        if(red>alpha) red=alpha;
        imagePixel[2]=red;

        imagePixel[3]=alpha;
    }else if(ImageFormat==QImage::Format_RGB32){
        int imageRed=imagePixel[2];
        int imageGreen=imagePixel[1];
        int imageBlue=imagePixel[0];
        imageRed+=(((int)color.r-imageRed)*(int)alphaMul+128)>>8;
        imageGreen+=(((int)color.g-imageGreen)*(int)alphaMul+128)>>8;
        imageBlue+=(((int)color.b-imageBlue)*(int)alphaMul+128)>>8;
        imagePixel[0]=imageBlue;
        imagePixel[1]=imageGreen;
        imagePixel[2]=imageRed;
    }else{
        Q_ASSERT(false);
    }
}

template<QImage::Format ImageFormat,
         bool UseColumn1, bool UseColumn2, bool UseColumn3>
static void drawColumnGrayscale(uchar *imagePixel,
                                const STFontColor& color,
                                const uint8_t *column1, int blend1,
                                const uint8_t *column2, int blend2,
                                const uint8_t *column3, int blend3,
                                int imagePitch,
                                int skipLines, int drawLines){
    int shift=0;
    while(skipLines--){
        shift+=4;
        if(shift==8){
            shift=0;
            if(UseColumn1) column1++;
            if(UseColumn2) column2++;
            if(UseColumn3) column3++;
        }
    }
    while(drawLines--){
        int alpha=0;
        if(UseColumn1){
            alpha+=(int)((*(column1)>>shift)&15)*blend1;
        }
        if(UseColumn2){
            alpha+=(int)((*(column2)>>shift)&15)*blend2;
        }
        if(UseColumn3){
            alpha+=(int)((*(column3)>>shift)&15)*blend3;
        }
        alpha=(alpha*0x11 /* 00010001 */ + 128)>>8;
        drawGrayscalePixel<ImageFormat>(imagePixel, color, alpha);
        imagePixel+=imagePitch;

        shift+=4;
        if(shift==8){
            shift=0;
            if(UseColumn1) column1++;
            if(UseColumn2) column2++;
            if(UseColumn3) column3++;
        }
    }
}


template<QImage::Format ImageFormat, int bytesPerPixel>
static void drawGlyphGrayscale(QImage& image, const QPointF& pos,
                               const STFontColor& color, const uint8_t *bitmap,
                                       const STFont::DrawOption& ){
    int xPos=(int)(pos.x()*256.f);  // s8.24
    int yPos=(int)pos.y();          // s32
    int imageWidth=image.width();
    int imageHeight=image.height();
    int imagePitch=image.bytesPerLine();

    if(yPos>=imageHeight || yPos<=-12){
        return;
    }

    int glyphWidth=(int)bitmap[0]<<7;   // s8.24
    int minX=(xPos>>8); // floor
    int maxX=(xPos+255+glyphWidth)>>8; // ceil

    bitmap+=2;

    int actualMinX=(minX<0)?0:minX;
    int actualMaxX=(maxX>imageWidth)?imageWidth:maxX;

    if(actualMinX>=actualMaxX)
        return;

    uchar *outImage=image.bits();
    int skipLines=(yPos<0)?-yPos:0;
    int drawLines=imageHeight-yPos;
    if(drawLines>12-skipLines) drawLines=12-skipLines;

    outImage+=imagePitch*(yPos+skipLines);
    outImage+=actualMinX*bytesPerPixel;

    int srcX=(actualMinX<<8)-xPos;

    for(int x=actualMinX;x<actualMaxX;x++){
        int srcX1=srcX;
        if(glyphWidth==128){
            int srcX2=srcX+256;
            if(srcX1>=128){
                continue;
            }else if(srcX1>=0){
                drawColumnGrayscale
                        <ImageFormat, true, false, false>
                        (outImage, color,
                         bitmap, 128-srcX1, NULL, 0, NULL, 0,
                         imagePitch, skipLines, drawLines);
            }else if(srcX1>=-128){
                drawColumnGrayscale
                        <ImageFormat, true, false, false>
                        (outImage, color,
                         bitmap, 128, NULL, 0, NULL, 0,
                         imagePitch, skipLines, drawLines);
            }else if(srcX2>0){
                drawColumnGrayscale
                        <ImageFormat, true, false, false>
                        (outImage, color,
                         bitmap, srcX2, NULL, 0, NULL, 0,
                         imagePitch, skipLines, drawLines);
            }
        }else{
            if(srcX1<0){
                int srcX2=srcX+256;
                if(srcX2<=0){
                    srcX+=256;
                    continue;
                }else if(srcX2<=128){
                    drawColumnGrayscale
                            <ImageFormat, true, false, false>
                            (outImage, color,
                             bitmap, srcX2, NULL, 0, NULL, 0,
                             imagePitch, skipLines, drawLines);
                }else if(srcX2<=256){
                    drawColumnGrayscale
                            <ImageFormat, true, true, false>
                            (outImage, color,
                             bitmap, 128, bitmap+STFontBytesPerColumn, srcX2-128, NULL, 0,
                             imagePitch, skipLines, drawLines);
                }else{
                    Q_ASSERT(false);
                }
            }else if(srcX1+256>glyphWidth){
                if(srcX1>=glyphWidth){
                    srcX+=256;
                    continue;
                }else if(srcX1>=glyphWidth-128){
                    drawColumnGrayscale
                            <ImageFormat, true, false, false>
                            (outImage, color,
                             bitmap+STFontBytesPerColumn*((glyphWidth>>7)-1), glyphWidth-srcX1, NULL, 0, NULL, 0,
                             imagePitch, skipLines, drawLines);
                }else if(srcX1>=glyphWidth-256){
                    drawColumnGrayscale
                            <ImageFormat, true, true, false>
                            (outImage, color,
                             bitmap+STFontBytesPerColumn*((glyphWidth>>7)-1), 128,
                             bitmap+STFontBytesPerColumn*((glyphWidth>>7)-2), glyphWidth-srcX1-128, NULL, 0,
                             imagePitch, skipLines, drawLines);
                }else{
                    Q_ASSERT(false);
                }
            }else{
                int srcX1f=srcX1&0x7f;
                if(srcX1f==0){
                    drawColumnGrayscale
                            <ImageFormat, true, true, false>
                            (outImage, color,
                             bitmap+STFontBytesPerColumn*((srcX1>>7)), 128,
                             bitmap+STFontBytesPerColumn*((srcX1>>7)+1), 128, NULL, 0,
                             imagePitch, skipLines, drawLines);
                }else{
                    drawColumnGrayscale
                            <ImageFormat, true, true, true>
                            (outImage, color,
                             bitmap+STFontBytesPerColumn*((srcX1>>7)), 128-srcX1f,
                             bitmap+STFontBytesPerColumn*((srcX1>>7)+1), 128,
                             bitmap+STFontBytesPerColumn*((srcX1>>7)+2), srcX1f,
                             imagePitch, skipLines, drawLines);
                }
            }
        }
        srcX+=256;
        outImage+=bytesPerPixel;
    }

}



template<QImage::Format ImageFormat>
static void drawRGBPixel(uchar *imagePixel,
                      uint8_t color, uint8_t alpha,
                      unsigned int alphaMul){
    alphaMul=(alphaMul*alpha)>>8;
    alphaMul+=alphaMul>>7; // 0-255 -> 0-256
    if(ImageFormat==QImage::Format_RGB32){
        int imageBlue=imagePixel[0];
        // gamma control
        const int middleValue=160;
        if((uint8_t)imageBlue<=(uint8_t)middleValue){
            // dark-bg
            alphaMul=256-alphaMul;
            unsigned int alphaMul2=(alphaMul*alphaMul)>>8;
            unsigned int per=middleValue-imageBlue;
            alphaMul=256-((alphaMul2*per+alphaMul*(256-per))>>8);
        }else if(true){
            // bright-bg
            unsigned int alphaMul2=(alphaMul*alphaMul)>>8;
            unsigned int per=imageBlue-middleValue;
            alphaMul=((alphaMul2*per+alphaMul*(256-per))>>8);
        }

        imageBlue+=(((int)color-imageBlue)*(int)alphaMul+128)>>8;
        imagePixel[0]=imageBlue;
    }else{
        Q_ASSERT(false);
    }
}

template<QImage::Format ImageFormat,
         bool UseColumn1, bool UseColumn2, bool UseColumn3>
static void drawColumnRGB(uchar *imagePixel,
                                uint8_t color, uint8_t colAlpha,
                                const uint8_t *column1, int blend1,
                                const uint8_t *column2, int blend2,
                                const uint8_t *column3, int blend3,
                                int imagePitch,
                                int skipLines, int drawLines){
    int shift=0;
    while(skipLines--){
        shift+=4;
        if(shift==8){
            shift=0;
            if(UseColumn1) column1++;
            if(UseColumn2) column2++;
            if(UseColumn3) column3++;
        }
    }
    while(drawLines--){
        int alpha=0;
        if(UseColumn1){
            alpha+=(int)((*(column1)>>shift)&15)*blend1;
        }
        if(UseColumn2){
            alpha+=(int)((*(column2)>>shift)&15)*blend2;
        }
        if(UseColumn3){
            alpha+=(int)((*(column3)>>shift)&15)*blend3;
        }
        alpha=(alpha*0x11 /* 00010001 */ + 128)>>8;
        drawRGBPixel<ImageFormat>(imagePixel, color, colAlpha, alpha);
        imagePixel+=imagePitch;

        shift+=4;
        if(shift==8){
            shift=0;
            if(UseColumn1) column1++;
            if(UseColumn2) column2++;
            if(UseColumn3) column3++;
        }
    }
}


template<QImage::Format ImageFormat, int bytesPerPixel>
static void drawGlyphRGB(QImage& image, const QPointF& pos,
                               const STFontColor& color, const uint8_t *bitmap,
                                       const STFont::DrawOption& ){
    int xPos=(int)(pos.x()*256.f);  // s8.24
    int yPos=(int)pos.y();          // s32
    int imageWidth=image.width();
    int imageHeight=image.height();
    int imagePitch=image.bytesPerLine();

    if(yPos>=imageHeight || yPos<=-12){
        return;
    }

    int glyphWidth=(int)bitmap[0]<<7;   // s8.24
    int minX=((xPos-86)>>8); // floor
    int maxX=((xPos+86)+255+glyphWidth)>>8; // ceil

    bitmap+=2;

    int actualMinX=(minX<0)?0:minX;
    int actualMaxX=(maxX>imageWidth)?imageWidth:maxX;

    if(actualMinX>=actualMaxX)
        return;


    int skipLines=(yPos<0)?-yPos:0;
    int drawLines=imageHeight-yPos;
    if(drawLines>12-skipLines) drawLines=12-skipLines;


    for(int channel=0;channel<3;channel++){

        uchar *outImage=image.bits();
        outImage+=imagePitch*(yPos+skipLines);
        outImage+=actualMinX*bytesPerPixel;

        int srcX=(actualMinX<<8)-xPos;

        outImage+=channel;
        srcX+=86*(1-channel); //srcX+=86*(1-channel);

        uint8_t channelColor;
        switch(channel){
        case 0: channelColor=color.b; break;
        case 1: channelColor=color.g; break;
        case 2: channelColor=color.r; break;
        }

        for(int x=actualMinX;x<actualMaxX;x++){
            int srcX1=srcX;
            if(glyphWidth==128){
                int srcX2=srcX+256;
                if(srcX1>=128){
                    srcX+=256; outImage+=bytesPerPixel;
                    continue;
                }else if(srcX1>=0){
                    drawColumnRGB
                            <ImageFormat, true, false, false>
                            (outImage, channelColor, color.a,
                             bitmap, 128-srcX1, NULL, 0, NULL, 0,
                             imagePitch, skipLines, drawLines);
                }else if(srcX1>=-128){
                    drawColumnRGB
                            <ImageFormat, true, false, false>
                            (outImage, channelColor, color.a,
                             bitmap, 128, NULL, 0, NULL, 0,
                             imagePitch, skipLines, drawLines);
                }else if(srcX2>0){
                    drawColumnRGB
                            <ImageFormat, true, false, false>
                            (outImage, channelColor, color.a,
                             bitmap, srcX2, NULL, 0, NULL, 0,
                             imagePitch, skipLines, drawLines);
                }
            }else{
                if(srcX1<0){
                    int srcX2=srcX+256;
                    if(srcX2<=0){
                        srcX+=256; outImage+=bytesPerPixel;
                        continue;
                    }else if(srcX2<=128){
                        drawColumnRGB
                                <ImageFormat, true, false, false>
                                (outImage, channelColor, color.a,
                                 bitmap, srcX2, NULL, 0, NULL, 0,
                                 imagePitch, skipLines, drawLines);
                    }else if(srcX2<=256){
                        drawColumnRGB
                                <ImageFormat, true, true, false>
                                (outImage, channelColor, color.a,
                                 bitmap, 128, bitmap+STFontBytesPerColumn, srcX2-128, NULL, 0,
                                 imagePitch, skipLines, drawLines);
                    }else{
                        Q_ASSERT(false);
                    }
                }else if(srcX1+256>glyphWidth){
                    if(srcX1>=glyphWidth){
                        srcX+=256; outImage+=bytesPerPixel;
                        continue;
                    }else if(srcX1>=glyphWidth-128){
                        drawColumnRGB
                                <ImageFormat, true, false, false>
                                (outImage, channelColor, color.a,
                                 bitmap+STFontBytesPerColumn*((glyphWidth>>7)-1), glyphWidth-srcX1, NULL, 0, NULL, 0,
                                 imagePitch, skipLines, drawLines);
                    }else if(srcX1>=glyphWidth-256){
                        drawColumnRGB
                                <ImageFormat, true, true, false>
                                (outImage, channelColor, color.a,
                                 bitmap+STFontBytesPerColumn*((glyphWidth>>7)-1), 128,
                                 bitmap+STFontBytesPerColumn*((glyphWidth>>7)-2), glyphWidth-srcX1-128, NULL, 0,
                                 imagePitch, skipLines, drawLines);
                    }else{
                        Q_ASSERT(false);
                    }
                }else{
                    int srcX1f=srcX1&0x7f;
                    if(srcX1f==0){
                        drawColumnRGB
                                <ImageFormat, true, true, false>
                                (outImage, channelColor, color.a,
                                 bitmap+STFontBytesPerColumn*((srcX1>>7)), 128,
                                 bitmap+STFontBytesPerColumn*((srcX1>>7)+1), 128, NULL, 0,
                                 imagePitch, skipLines, drawLines);
                    }else{
                        drawColumnRGB
                                <ImageFormat, true, true, true>
                                (outImage, channelColor, color.a,
                                 bitmap+STFontBytesPerColumn*((srcX1>>7)), 128-srcX1f,
                                 bitmap+STFontBytesPerColumn*((srcX1>>7)+1), 128,
                                 bitmap+STFontBytesPerColumn*((srcX1>>7)+2), srcX1f,
                                 imagePitch, skipLines, drawLines);
                    }
                }
            }
            srcX+=256;
            outImage+=bytesPerPixel;
        }

    }

}



void STFont::drawGlyph(QImage& image, const QPointF& pos,
               const QColor& color, const uint8_t *bitmap,
                       const DrawOption& options){
    if(color.alpha()==0)
        return;
    switch(image.format()){
    case QImage::Format_ARGB32_Premultiplied:
        drawGlyphGrayscale<QImage::Format_ARGB32_Premultiplied, 4>
                (image, pos, color, bitmap, options);
        break;
    case QImage::Format_ARGB32:
        drawGlyphGrayscale<QImage::Format_ARGB32, 4>
                (image, pos, color, bitmap, options);
        break;
    case QImage::Format_RGB32:
        if(options.useSubpixelRendering){
            drawGlyphRGB<QImage::Format_RGB32, 4>
                    (image, pos, color, bitmap, options);
        }else{
            drawGlyphGrayscale<QImage::Format_RGB32, 4>
                    (image, pos, color, bitmap, options);
        }
        break;
    default:
        Q_ASSERT(false);
    }
}

int STFont::nextCursorPos(const QString& str, int currentCursorPos){
    if(currentCursorPos>=str.size()-1) return str.size();
    int length=str.length();
    const QChar *chars=str.data();

    currentCursorPos++;

    while(currentCursorPos<str.size()){
        uint32_t ucs=chars[currentCursorPos].unicode();
        int charLen=1;

        if(currentCursorPos<length-1){
            // is this surrogate pair?
            if(isLeadingSurrogatePair(chars[currentCursorPos].unicode()) &&
                isTrailSurrogatePair(chars[currentCursorPos+1].unicode())){
                ucs=decodeSurrogatePair(chars[currentCursorPos].unicode(),
                                        chars[currentCursorPos+1].unicode());
            }
        }
        if(ucs>=0x10000){
            charLen=2;
        }else{
            charLen=1;
        }

        if(combiningCharInfo(ucs).isCombiningChar){
            // skip
            currentCursorPos+=charLen;
        }else{
            break;
        }

    }
    return currentCursorPos;
}

int STFont::previousCursorPos(const QString& str, int currentCursorPos){
    if(currentCursorPos<=1) return 0;
    //unsigned int length=str.length();
    const QChar *chars=str.data();

    currentCursorPos--;

    while(currentCursorPos>0){
        uint32_t ucs=chars[currentCursorPos].unicode();

        if(currentCursorPos>0){
            // is this surrogate pair?
            if(isLeadingSurrogatePair(chars[currentCursorPos-1].unicode()) &&
                isTrailSurrogatePair(chars[currentCursorPos].unicode())){
                currentCursorPos--;
                ucs=decodeSurrogatePair(chars[currentCursorPos].unicode(),
                                        chars[currentCursorPos+1].unicode());
            }
        }

        if(combiningCharInfo(ucs).isCombiningChar){
            // skip...?
            if(currentCursorPos==0){
                // cannot skip
                break;
            }else if(chars[currentCursorPos-1]=='\n'){
                // this char is not working as combining character
                break;
            }
            currentCursorPos--;
        }else{
            break;
        }
    }

    return currentCursorPos;

}
