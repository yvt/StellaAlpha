#include "stjsonparser.h"
#include "stmath.h"

STJsonParser::STJsonParser(QObject *parent) :
    QObject(parent)
{
}


static QString goThroughCommonStringPool(QString s){
    return STGoThroughCommonStringPool(s, true);
}

QVariant STJsonParser::parse(QByteArray data){
    int pos=0;
    return parse(data, pos);
}

QVariant STJsonParser::parse(QByteArray data, int& pos){
    skipWhitespace(data, pos);
    if(pos>=data.length()) // no data
        return QVariant();

    char c=data[pos];
    switch(c){
    case '"':   // JSON only supports double quotation mark
        return parseString(data, pos);
    case '{':
        return parseMap(data, pos);
    case '[':
        return parseList(data, pos);
    case 'f':
    case 't':
    case 'n':
        return parseKeyword(data, pos);
    case '-':
        return parseNumber(data, pos);
    }
    if(c>='0' && c<='9')
        return parseNumber(data, pos);

    // invalid character
    return QVariant();
}



QVariant STJsonParser::parseMap(QByteArray data, int &pos){
    Q_ASSERT(data[pos]=='{');
    pos++;

    QVariantMap map;

    enum MapParserState{
        ReadingName,
        ReadingNameSeparator,
        ReadingValue,
        ReadingValueSeparator
    } state=ReadingName;

    QVariant name, value;

    while(true){
        skipWhitespace(data, pos);
        if(pos>=data.length())
            break;
        char c=data[pos];
        if(c=='}'){
            pos++;
            break;
        }

        switch(state){
        case ReadingName:
            name=parse(data, pos);
            state=ReadingNameSeparator;
            break;
        case ReadingNameSeparator:
            if(c==':'){
                state=ReadingValue;
                pos++;
            }else{
                return map; // invalid char!
            }
            break;
        case ReadingValue:
            value=parse(data, pos);
            map.insert(name.toString(), value);
            state=ReadingValueSeparator;
            break;
        case ReadingValueSeparator:
            if(c==','){
                state=ReadingName;
                pos++;
            }else{
                return map; // invalid char!
            }
            break;
        default:
            Q_ASSERT(false);
        }
    }

    return map;
}

QVariant STJsonParser::parseList(QByteArray data, int &pos){
    Q_ASSERT(data[pos]=='[');
    pos++;

    QVariantList list;

    enum ListParserState{
        ReadingElement,
        ReadingSeparator
    } state=ReadingElement;

    while(true){
        skipWhitespace(data, pos);
        if(pos>=data.length())
            break;
        char c=data[pos];
        if(c==']'){
            pos++;
            break;
        }
        switch(state){
        case ReadingElement:
            list.append(parse(data, pos));
            state=ReadingSeparator;
            break;
        case ReadingSeparator:
            if(c==','){
                state=ReadingElement;
                pos++;
            }else{
                return list; // invalid char!
            }
            break;
        default:
            Q_ASSERT(false);
        }
    }

    return list;
}

static int decodeOneHex(char c){
    if(c>='0' && c<='9')
        return (int)c-'0';
    if(c>='a' && c<='f')
        return (int)c-'a'+10;
    if(c>='A' && c<='F')
        return (int)c-'A'+10;
    return -1;
}

static QChar parseFourDigitHexChar(QByteArray data, int pos, bool& ok){
    int d1, d2, d3, d4;
    Q_ASSERT(pos<data.size()-3);
    ok=false;
    d1=decodeOneHex(data[pos]);
    if(d1==-1){
        return QChar((ushort)0);
    }
    d2=decodeOneHex(data[pos+1]);
    if(d2==-1){
        return QChar((ushort)0);
    }
    d3=decodeOneHex(data[pos+2]);
    if(d3==-1){
        return QChar((ushort)0);
    }
    d4=decodeOneHex(data[pos+3]);
    if(d4==-1){
        return QChar((ushort)0);
    }
    d1=(d1<<4)+d2;
    d1=(d1<<4)+d3;
    d1=(d1<<4)+d4;
    ok=true;
    return QChar((ushort)d1);
}



QVariant STJsonParser::parseString(QByteArray data, int &pos){
    Q_ASSERT(data[pos]=='"');
    pos++;

    if(data[pos]=='"'){
        pos++;
        return QString();
    }

    QString string;

    while(pos<data.length()){
        char c=data[pos];
        if(c=='\\' && pos<data.length()-1){
            // escaped
            pos++; c=data[pos];
            switch(c){
            case '"':
                string+='"'; pos++;
                break;
            case '\\':
                string+='\\'; pos++;
                break;
            case '/':
                string+='/'; pos++;
                break;
            case 'b':
                string+=(char)0x08; pos++;
                break;
            case 'f':
                string+=(char)0x0c; pos++;
                break;
            case 'n':
                string+=(char)0x0a; pos++;
                break;
            case 'r':
                string+=(char)0x0d; pos++;
                break;
            case 't':
                string+=(char)0x09; pos++;
                break;
            case 'u':
                pos++;
                if(pos<data.length()-3){
                    bool ok=false;
                    QChar c=parseFourDigitHexChar(data, pos, ok);
                    if(!ok){
                        goto invalidUnicodeEscape;
                    }
                    string+=c;
                    pos+=4;
                }else{
                    invalidUnicodeEscape:
                    // invalid escape
                    string+='\\';
                    string+='u';
                }
                break;
            default:
                // unrecognized
                string+='\\';
                break;
            }
        }else if(c=='"'){
            pos++;
            break;
        }else{
            // unescaped
            if((unsigned char)c&0x80){
                uint32_t ucs;
                if((c&0x40)==0){
                    goto invalidUtf8Found;
                }
                if((c&0x20)==0){
                    if(pos>data.length()-2){
                        goto invalidUtf8Found;
                    }
                    ucs= (uint32_t)(data[pos+0]&0x1f)<<6;
                    ucs|=(uint32_t)(data[pos+1]&0x3f)<<0;
                    pos+=2;
                }else if((c&0x10)==0){
                    if(pos>data.length()-3){
                        goto invalidUtf8Found;
                    }
                    ucs= (uint32_t)(data[pos+0]&0x0f)<<12;
                    ucs|=(uint32_t)(data[pos+1]&0x3f)<<6;
                    ucs|=(uint32_t)(data[pos+2]&0x3f)<<0;
                    pos+=3;
                }else if((c&0x08)==0){
                    if(pos>data.length()-4){
                        goto invalidUtf8Found;
                    }
                    ucs= (uint32_t)(data[pos+0]&0x07)<<18;
                    ucs|=(uint32_t)(data[pos+1]&0x3f)<<12;
                    ucs|=(uint32_t)(data[pos+2]&0x3f)<<6;
                    ucs|=(uint32_t)(data[pos+3]&0x3f)<<0;
                    pos+=4;
                }else if((c&0x04)==0){
                    if(pos>data.length()-5){
                        goto invalidUtf8Found;
                    }
                    ucs= (uint32_t)(data[pos+0]&0x03)<<24;
                    ucs|=(uint32_t)(data[pos+1]&0x3f)<<18;
                    ucs|=(uint32_t)(data[pos+2]&0x3f)<<12;
                    ucs|=(uint32_t)(data[pos+3]&0x3f)<<6;
                    ucs|=(uint32_t)(data[pos+4]&0x3f)<<0;
                    pos+=5;
                }else if((c&0x02)==0){
                    if(pos>data.length()-6){
                        goto invalidUtf8Found;
                    }
                    ucs= (uint32_t)(data[pos+0]&0x01)<<30;
                    ucs|=(uint32_t)(data[pos+1]&0x3f)<<24;
                    ucs|=(uint32_t)(data[pos+2]&0x3f)<<18;
                    ucs|=(uint32_t)(data[pos+3]&0x3f)<<12;
                    ucs|=(uint32_t)(data[pos+4]&0x3f)<<6;
                    ucs|=(uint32_t)(data[pos+5]&0x3f)<<0;
                    pos+=6;
                }else{
                    goto invalidUtf8Found;
                }
                if(ucs>>16){ // SMP
                    ucs-=0x10000;
                    string+=QChar((ushort)(0xd800+(ucs>>10)));
                    string+=QChar((ushort)(0xdc00+(ucs&0x3ff)));
                }else{
                    string+=QChar((ushort)(ucs));
                }
            }else{
                invalidUtf8Found:
                string+=QChar((ushort)(unsigned char)c);
                pos++;
            }
        }
    }

    return goThroughCommonStringPool(string);
}

static inline bool isKeywordChar(char c){
    return (c>='a' && c<='z') || (c>='A' && c<='Z') || (c>='0' && c<='9') || c=='_';
}

QVariant STJsonParser::parseKeyword(QByteArray data, int &pos){
    // we assume keyword is true, false, or null.
    QVariant var;
    switch(data[pos]){
    case 't':
        var=QVariant(true);
        break;
    case 'f':
        var=QVariant(false);
        break;
    case 'n':
        var=QVariant();
        break;
    default: // unknown keyword (invalid)
        var=QVariant();
    }

    while(pos<data.length()){
        if(isKeywordChar(data[pos]))
            pos++;
        else
            break;
    }

    return var;
}

static inline bool isNumberChar(char c){
    if(c>='0' && c<='9')
        return true;
    if(c=='+' || c=='-' || c=='e' || c=='.' || c=='E')
        return true;
    return false;
}

QVariant STJsonParser::parseNumber(QByteArray data, int &pos){

    int startPos=pos;
    bool real=false;
    while(pos<data.length()){
        if(!isNumberChar(data[pos]))
            break;
        if(data[pos]=='.' || data[pos]=='e')
            real=true;
        pos++;
    }

    if(pos==startPos)
        return QVariant();

    QByteArray sub=data.mid(startPos, pos-startPos);
    if(real)
        return (sub.toDouble());
    else{
        bool ok;
        qint64 i64=sub.toLongLong(&ok);
        if(ok)
            return i64;
        quint64 u64=sub.toULongLong(&ok);
        if(ok)
            return u64;
        return sub.toDouble();
    }
}


void STJsonParser::skipWhitespace(QByteArray data, int &pos){
    while(pos<data.length()){
        if(data[pos]==' ' || data[pos]=='\r' || data[pos]=='\n' || data[pos]=='\t')
            pos++;
        else if(data[pos]=='/' && pos<data.length()-1 && data[pos+1]=='*'){
            // comment start
            pos+=2;
            pos=data.indexOf("*/", pos);
            if(pos==-1){
                // no comment end!
                pos=data.length();
                break;
            }else{
                pos+=2;
            }
        }else
            break;
    }
}


