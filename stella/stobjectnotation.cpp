#include "stobjectnotation.h"
#include "stobjectmanager.h"
#include "stmath.h"
#include <QDebug>
#include <QFile>
#include <QDir>

#define STONHeaderMagic "STON"

#define STONDataTypeNull        ((char)0x00)
#define STONDataTypeMap         ((char)0x01)
#define STONDataTypeList        ((char)0x02)
#define STONDataTypeString8     ((char)0x03)
#define STONDataTypeString16    ((char)0x04)
#define STONDataTypeString24    ((char)0x05)
#define STONDataTypeString32    ((char)0x06)
#define STONDataTypeObject8     ((char)0x07)
#define STONDataTypeObject16    ((char)0x08)
#define STONDataTypeObject24    ((char)0x09)
#define STONDataTypeObject32    ((char)0x0a)
#define STONDataTypePInteger8   ((char)0x0b)
#define STONDataTypePInteger16  ((char)0x0c)
#define STONDataTypePInteger24  ((char)0x0d)
#define STONDataTypePInteger32  ((char)0x0e)
#define STONDataTypePInteger48  ((char)0x0f)
#define STONDataTypePInteger64  ((char)0x10)
#define STONDataTypeNInteger8   ((char)0x11)
#define STONDataTypeNInteger16  ((char)0x12)
#define STONDataTypeNInteger24  ((char)0x13)
#define STONDataTypeNInteger32  ((char)0x14)
#define STONDataTypeNInteger48  ((char)0x15)
#define STONDataTypeNInteger64  ((char)0x16)
#define STONDataTypeTrue        ((char)0x17)
#define STONDataTypeFalse       ((char)0x18)
#define STONDataTypeFloat       ((char)0x19)
#define STONDataTypeDouble      ((char)0x1a)
#define STONDataTypeDateTime    ((char)0x1b)
#define STONDataTypeCommonString    ((char)0x1c)
#define STONDataTypeZeroInt     ((char)0x1d)
#define STONDataTypeEmptyString    ((char)0x1e)
#define STONDataTypeEOMLMarker  ((char)0x1f) // end of list/map
#define STONDataTypeChar8  ((char)0x20)
#define STONDataTypeChar16  ((char)0x21)
#define STONDataTypeStrSInteger8  ((char)0x22)
#define STONDataTypeStrInteger8  ((char)0x23)
#define STONDataTypeStrInteger16  ((char)0x24)
#define STONDataTypeStrInteger32  ((char)0x25)
#define STONDataTypeStrInteger64 ((char)0x26)


#define STONDataTypeIntegerBase    ((char)0x80) // above this is 7-bit signed integer

#define STONObjectNone          ((char)0x00)
#define STONObjectTypeMask      ((char)0x3f)
#define STONObjectTypeRoot      ((char)0x01)
#define STONObjectTypeUser      ((char)0x02)
#define STONObjectTypeStatus    ((char)0x03)
#define STONObjectTypeMessage   ((char)0x04)
#define STONObjectTypeList      ((char)0x05)
#define STONObjectSizeMask      ((char)0xc0)
#define STONObjectSizeBits8     ((char)0x00)
#define STONObjectSizeBits16    ((char)0x40)
#define STONObjectSizeBits24    ((char)0x80)
#define STONObjectSizeBits32    ((char)0xc0)


// variable-length integer encoding
static void writeULEB64(QByteArray& bytes, quint64 value){
    forever{
        if(value>=0x80){
            bytes.append((char)(value&0x7f)|0x80);
            value>>=7;
        }else{
            bytes.append((char)value);
            break;
        }
    }
}
static quint64 readULEB64(const QByteArray& bytes, int& pos){
    quint64 outValue=0;
    int shift=0;

    uint8_t c;
    do{
        if(pos>=bytes.length())throw "unexpected EOF";
        c=(uint8_t)bytes[pos];
        outValue|=((quint64)(c&0x7f))<<shift;
        shift+=7;
        pos++;
    }while(c&0x80);
    return outValue;
}


STObjectNotation::STObjectNotation(QObject *parent) :
    QObject(parent)
{
}

struct STONSerializerContext{
    QByteArray bytes;
    STONSerializerContext(){
        bytes.append(STONHeaderMagic);
        bytes.append("xxxx"); // placeholder for data size
        bytes.append("xxxx"); // placeholder for string map address

    }
protected:

    QHash<QString, int> stringMap;
    QList<QString> strings;

    QMap<quint64, int> userObjectMap;
    QMap<quint64, int> statusObjectMap;
    QMap<quint64, int> messageObjectMap;
    QMap<quint64, int> listObjectMap;
    struct SerializedObject{
        char type;
        STObjectManager::ObjectRef object;
    };
    QList<SerializedObject> objects;
    int nextSerializedObjectIndex;

    quint32 objectMapSize;



    static void writeULongLong(QByteArray& objBytes, qulonglong i, bool neg=false){
        if(i==0){
            objBytes.append(STONDataTypeZeroInt);
        }else if(i<=64){
            if(neg){
                char j=-(char)i;
                j&=127;
                j|=STONDataTypeIntegerBase;
                objBytes.append(j);
            }else if(i==64){
                objBytes.append(neg?STONDataTypeNInteger8:STONDataTypePInteger8);
                objBytes.append(64);
            }else{
                char j=(char)i;
                j|=STONDataTypeIntegerBase;
                objBytes.append(j);
            }
        }else if(i<(1ULL<<8)){
            objBytes.append(neg?STONDataTypeNInteger8:STONDataTypePInteger8);
            objBytes.append((uint8_t)(i));
        }else if(i<(1ULL<<16)){
            objBytes.append(neg?STONDataTypeNInteger16:STONDataTypePInteger16);
            objBytes.append((uint8_t)(i));
            objBytes.append((uint8_t)(i>>8));
        }else if(i<(1ULL<<24)){
            objBytes.append(neg?STONDataTypeNInteger24:STONDataTypePInteger24);
            objBytes.append((uint8_t)(i));
            objBytes.append((uint8_t)(i>>8));
            objBytes.append((uint8_t)(i>>16));
        }else if(i<(1ULL<<32)){
            objBytes.append(neg?STONDataTypeNInteger32:STONDataTypePInteger32);
            objBytes.append((uint8_t)(i));
            objBytes.append((uint8_t)(i>>8));
            objBytes.append((uint8_t)(i>>16));
            objBytes.append((uint8_t)(i>>24));
        }else if(i<(1ULL<<48)){
            objBytes.append(neg?STONDataTypeNInteger48:STONDataTypePInteger48);
            objBytes.append((uint8_t)(i));
            objBytes.append((uint8_t)(i>>8));
            objBytes.append((uint8_t)(i>>16));
            objBytes.append((uint8_t)(i>>24));
            objBytes.append((uint8_t)(i>>32));
            objBytes.append((uint8_t)(i>>40));
        }else{
            objBytes.append(neg?STONDataTypeNInteger64:STONDataTypePInteger64);
            objBytes.append((uint8_t)(i));
            objBytes.append((uint8_t)(i>>8));
            objBytes.append((uint8_t)(i>>16));
            objBytes.append((uint8_t)(i>>24));
            objBytes.append((uint8_t)(i>>32));
            objBytes.append((uint8_t)(i>>40));
            objBytes.append((uint8_t)(i>>48));
            objBytes.append((uint8_t)(i>>56));
        }
    }
    static void writeLongLong(QByteArray& objBytes, qlonglong i){
        if((qulonglong)i==0x8000000000000000ULL){
            // cannot be negated
            writeULongLong(objBytes, 0x8000000000000000ULL, true);
        }else if(i<0){
            writeULongLong(objBytes, (qulonglong)(-i), true);
        }else{
            writeULongLong(objBytes, (qulonglong)i, false);
        }
    }
    void writeString(QByteArray& objBytes, QString str){
        if(str.isEmpty()){
            objBytes.append(STONDataTypeEmptyString);
            return;
        }
        if(str.count()>=2 && str.startsWith("-")){
            int intifed;
            bool intfyOk=false;
            intifed=str.toInt(&intfyOk);
            if(intfyOk && intifed>=-128 && intifed<128){
                objBytes.append(STONDataTypeStrSInteger8);
                objBytes.append((char)intifed);
                return;
            }
        }else{
            qulonglong intfied;
            bool intfyOk=false;
            intfied=str.toULongLong(&intfyOk);
            if(intfyOk){
                if(intfied<(1ULL<<8)){
                    objBytes.append(STONDataTypeStrInteger8);
                    objBytes.append((uint8_t)(intfied));
                    return;
                }else if(intfied<(1ULL<<16)){
                    objBytes.append(STONDataTypeStrInteger16);
                    objBytes.append((uint8_t)(intfied));
                    objBytes.append((uint8_t)(intfied>>8));
                    return;
                }else if(intfied<(1ULL<<32)){
                    objBytes.append(STONDataTypeStrInteger32);
                    objBytes.append((uint8_t)(intfied));
                    objBytes.append((uint8_t)(intfied>>8));
                    objBytes.append((uint8_t)(intfied>>16));
                    objBytes.append((uint8_t)(intfied>>24));
                    return;
                }else{
                    objBytes.append(STONDataTypeStrInteger64);
                    objBytes.append((uint8_t)(intfied));
                    objBytes.append((uint8_t)(intfied>>8));
                    objBytes.append((uint8_t)(intfied>>16));
                    objBytes.append((uint8_t)(intfied>>24));
                    objBytes.append((uint8_t)(intfied>>32));
                    objBytes.append((uint8_t)(intfied>>40));
                    objBytes.append((uint8_t)(intfied>>48));
                    objBytes.append((uint8_t)(intfied>>56));
                    return;
                }
            }
        }

        if(str.count()<=1){
            uint32_t ucs=str[0].unicode();
            if(ucs<0x100){
                objBytes.append(STONDataTypeChar8);
                objBytes.append((uint8_t)ucs);
            }else{
                Q_ASSERT(ucs<0x10000);
                objBytes.append(STONDataTypeChar16);
                objBytes.append((uint8_t)ucs);
                objBytes.append((uint8_t)(ucs>>8));
            }
            return;
        }

        int ind;
        QString str2=STGoThroughCommonStringPool(str,false, &ind);
        if(ind!=-1){
            objBytes.append(STONDataTypeCommonString);
            objBytes.append((char)ind);
        }else{
            QHash<QString, int>::iterator it=stringMap.find(str);
            if(it==stringMap.end()){
                ind=strings.count();
                strings.append(str);
                stringMap.insert(str, ind);
            }else{
                ind=it.value();
            }

            if(ind<(1<<8)){
                objBytes.append(STONDataTypeString8);
                objBytes.append((uint8_t)(ind));
            }else if(ind<(1<<16)){
                objBytes.append(STONDataTypeString16);
                objBytes.append((uint8_t)(ind));
                objBytes.append((uint8_t)(ind>>8));
            }else if(ind<(1<<24)){
                objBytes.append(STONDataTypeString24);
                objBytes.append((uint8_t)(ind));
                objBytes.append((uint8_t)(ind>>8));
                objBytes.append((uint8_t)(ind>>16));
            }else{
                objBytes.append(STONDataTypeString32);
                objBytes.append((uint8_t)(ind));
                objBytes.append((uint8_t)(ind>>8));
                objBytes.append((uint8_t)(ind>>16));
                objBytes.append((uint8_t)(ind>>24));
            }
        }
    }
    void writeChildObject(QByteArray& objBytes, QVariant var){
        static int userType=QMetaType::type("STObjectManager::UserRef");
        static int statusType=QMetaType::type("STObjectManager::StatusRef");
        static int messageType=QMetaType::type("STObjectManager::MessageRef");
        static int listType=QMetaType::type("STObjectManager::ListRef");
        int type=var.userType();
        int index=0;
        QMap<quint64, int>::iterator it;
        SerializedObject ser;
        if(type==userType){
            STObjectManager::UserRef obj=var.value<STObjectManager::UserRef>();
            it=userObjectMap.find(obj->id);
            if(it==userObjectMap.end()){
                ser.object=obj;
                ser.type=STONObjectTypeUser;
                index=objects.count();
                objects.append(ser);
                userObjectMap.insert(obj->id, index);
            }else{
                index=it.value();
            }
        }else if(type==statusType){
            STObjectManager::StatusRef obj=var.value<STObjectManager::StatusRef>();
            it=statusObjectMap.find(obj->id);
            if(it==statusObjectMap.end()){
                ser.object=obj;
                ser.type=STONObjectTypeStatus;
                index=objects.count();
                objects.append(ser);
                statusObjectMap.insert(obj->id, index);
            }else{
                index=it.value();
            }
        }else if(type==messageType){
            STObjectManager::MessageRef obj=var.value<STObjectManager::MessageRef>();
            it=messageObjectMap.find(obj->id);
            if(it==messageObjectMap.end()){
                ser.object=obj;
                ser.type=STONObjectTypeMessage;
                index=objects.count();
                objects.append(ser);
                messageObjectMap.insert(obj->id, index);
            }else{
                index=it.value();
            }
        }else if(type==listType){
            STObjectManager::ListRef obj=var.value<STObjectManager::ListRef>();
            it=listObjectMap.find(obj->id);
            if(it==listObjectMap.end()){
                ser.object=obj;
                ser.type=STONObjectTypeList;
                index=objects.count();
                objects.append(ser);
                listObjectMap.insert(obj->id, index);
            }else{
                index=it.value();
            }
        }else{
            qFatal("STONSerializerContext:writeCChildObject: invalid type: %d", type);
        }

        if(index<(1<<8)){
            objBytes.append(STONDataTypeObject8);
            objBytes.append((uint8_t)(index));
        }else if(index<(1<<16)){
            objBytes.append(STONDataTypeObject16);
            objBytes.append((uint8_t)(index));
            objBytes.append((uint8_t)(index>>8));
        }else if(index<(1<<24)){
            objBytes.append(STONDataTypeObject24);
            objBytes.append((uint8_t)(index));
            objBytes.append((uint8_t)(index>>8));
            objBytes.append((uint8_t)(index>>16));
        }else{
            objBytes.append(STONDataTypeObject32);
            objBytes.append((uint8_t)(index));
            objBytes.append((uint8_t)(index>>8));
            objBytes.append((uint8_t)(index>>16));
            objBytes.append((uint8_t)(index>>24));
        }
    }
    void writeList(QByteArray& objBytes, QVariantList var){
        objBytes.append(STONDataTypeList);
        foreach(QVariant v, var){
            serialize(objBytes, v);
        }
        objBytes.append(STONDataTypeEOMLMarker);
    }
    void writeMap(QByteArray& objBytes, QVariantMap var){
        objBytes.append(STONDataTypeMap);
        for(QVariantMap::iterator it=var.begin();it!=var.end();it++){
            serialize(objBytes, it.key());
            serialize(objBytes, it.value());
        }
        objBytes.append(STONDataTypeEOMLMarker);
    }

    void serialize(QByteArray& objBytes, QVariant var){
        switch(var.type()){
        case QVariant::Invalid:
            objBytes.append(STONDataTypeNull);
            break;
        case QVariant::Bool:
            if(var.toBool()){
                objBytes.append(STONDataTypeTrue);
            }else{
                objBytes.append(STONDataTypeFalse);
            }
            break;
        case QVariant::Int:
        case QVariant::LongLong:
        case QVariant::UInt:
            writeLongLong(objBytes, var.toLongLong());
            break;
        case QVariant::ULongLong:
            writeULongLong(objBytes, var.toLongLong());
            break;
        case QVariant::String:
            writeString(objBytes, var.toString());
            break;
        case QVariant::Double:
        {
            objBytes.append(STONDataTypeDouble);

            double v=var.toDouble();
            objBytes.append((const char *)&v, 8);
        }
            break;
        case QVariant::DateTime:
        {
            objBytes.append(STONDataTypeDateTime);

            qint64 vl=var.toDateTime().toMSecsSinceEpoch();
            objBytes.append((const char *)&vl, 8);
        }
            break;
        case QVariant::List:
            writeList(objBytes, var.toList());
            break;
        case QVariant::Map:
            writeMap(objBytes, var.toMap());
            break;
        case QVariant::Char:
            writeString(objBytes, QString(var.toChar()));
            break;
        case QVariant::UserType:
            writeChildObject(objBytes, var) ;
            break;
        default:
            qFatal("STONSerializerContext::serialize: invalid variant type: %d", (int)var.userType());
        }
    }

    void writeObject(char type, QVariant var){
        QByteArray objBytes;
        int ocnt=bytes.count();
        serialize(objBytes, var);
        Q_ASSERT(ocnt==bytes.count());

        uint32_t size=(uint32_t)objBytes.count();
        char header=type;
        if(size<(1<<8)){
            header|=STONObjectSizeBits8;
            bytes.append(header);
            bytes.append((uint8_t)(size));
        }else if(size<(1<<16)){
            header|=STONObjectSizeBits16;
            bytes.append(header);
            bytes.append((uint8_t)(size));
            bytes.append((uint8_t)(size>>8));
        }else if(size<(1<<24)){
            header|=STONObjectSizeBits24;
            bytes.append(header);
            bytes.append((uint8_t)(size));
            bytes.append((uint8_t)(size>>8));
            bytes.append((uint8_t)(size>>16));
        }else{
            header|=STONObjectSizeBits32;
            bytes.append(header);
            bytes.append((uint8_t)(size));
            bytes.append((uint8_t)(size>>8));
            bytes.append((uint8_t)(size>>16));
            bytes.append((uint8_t)(size>>24));
        }

        bytes.append(objBytes);
    }

    void writeOneString(QString str){
        const QChar *chars=str.constData();
        unsigned int len=str.length();
        writeULEB64(bytes, len);
        for(unsigned int i=0;i<len;i++){
            writeULEB64(bytes, chars[i].unicode());
        }
    }

    void writeStringTable(){
        foreach(QString st, strings){
            writeOneString(st);
        }
        writeULEB64(bytes, 0);
    }

public:

    void serializeRootObject(QVariant var){
        Q_ASSERT(bytes.size()==12);


        SerializedObject obj;
        obj.type=STONObjectTypeRoot;
        objects.push_back(obj);

        writeObject(STONObjectTypeRoot, var);
        nextSerializedObjectIndex=1;
        while(nextSerializedObjectIndex<objects.count()){
            SerializedObject& obj=objects[nextSerializedObjectIndex];
            writeObject(obj.type, obj.object->toSTONVariant());
            nextSerializedObjectIndex++;
        }

        bytes.append(STONObjectNone);

        uint32_t strAddr=bytes.count();
        bytes[8]=(uint8_t)(strAddr);
        bytes[9]=(uint8_t)(strAddr>>8);
        bytes[10]=(uint8_t)(strAddr>>16);
        bytes[11]=(uint8_t)(strAddr>>24);

        writeStringTable();

        uint32_t size=bytes.count();
        bytes[4]=(uint8_t)(size);
        bytes[5]=(uint8_t)(size>>8);
        bytes[6]=(uint8_t)(size>>16);
        bytes[7]=(uint8_t)(size>>24);

    }
};



QByteArray STObjectNotation::serialize(QVariant var){
    STONSerializerContext context;
    context.serializeRootObject(var);
    return context.bytes;
}

class STONParserContext{
    const QByteArray& bytes;
    int pos;
    struct STONObject{
        uint32_t addr;
        uint32_t dataAddr;
        uint32_t size;
        uint8_t objectType;
        bool isRead:1;
        bool isReading:1;
        QVariant data;
        QVariant refObject;
        STONObject(){
            isRead=false;
            isReading=false;
        }
    };
    QVector<STONObject> objects;
    QVector<QString> strings;

    uint8_t peekU8(){
        if(pos+1>bytes.length()) throw "unexpected EOF.";
        return (uint8_t)bytes[pos];
    }
    uint8_t readU8(){
        if(pos+1>bytes.length()) throw "unexpected EOF.";
        return (uint8_t)bytes[pos++];
    }
    uint16_t readU16(){
        if(pos+2>bytes.length()) throw "unexpected EOF.";
        uint16_t val=(uint8_t)bytes[pos++];
        val|=((uint16_t)(uint8_t)bytes[pos++])<<8;
        return val;
    }
    uint32_t readU24(){
        if(pos+3>bytes.length()) throw "unexpected EOF.";
        uint32_t val=(uint8_t)bytes[pos++];
        val|=((uint32_t)(uint8_t)bytes[pos++])<<8;
        val|=((uint32_t)(uint8_t)bytes[pos++])<<16;
        return val;
    }
    uint32_t readU32(){
        if(pos+4>bytes.length()) throw "unexpected EOF.";
        uint32_t val=(uint8_t)bytes[pos++];
        val|=((uint32_t)(uint8_t)bytes[pos++])<<8;
        val|=((uint32_t)(uint8_t)bytes[pos++])<<16;
        val|=((uint32_t)(uint8_t)bytes[pos++])<<24;
        return val;
    }
    uint64_t readU48(){
        if(pos+6>bytes.length()) throw "unexpected EOF.";
        uint64_t val=(uint8_t)bytes[pos++];
        val|=((uint64_t)(uint8_t)bytes[pos++])<<8;
        val|=((uint64_t)(uint8_t)bytes[pos++])<<16;
        val|=((uint64_t)(uint8_t)bytes[pos++])<<24;
        val|=((uint64_t)(uint8_t)bytes[pos++])<<32;
        val|=((uint64_t)(uint8_t)bytes[pos++])<<40;
        return val;
    }
    uint64_t readU64(){
        if(pos+8>bytes.length()) throw "unexpected EOF.";
        uint64_t val=(uint8_t)bytes[pos++];
        val|=((uint64_t)(uint8_t)bytes[pos++])<<8;
        val|=((uint64_t)(uint8_t)bytes[pos++])<<16;
        val|=((uint64_t)(uint8_t)bytes[pos++])<<24;
        val|=((uint64_t)(uint8_t)bytes[pos++])<<32;
        val|=((uint64_t)(uint8_t)bytes[pos++])<<40;
        val|=((uint64_t)(uint8_t)bytes[pos++])<<48;
        val|=((uint64_t)(uint8_t)bytes[pos++])<<56;
        return val;
    }
    uint64_t readULEB64(){
        return ::readULEB64(bytes,pos);
    }
    float readFloat(){
        if(pos+4>bytes.length()) throw "unexpected EOF.";
        float value; memcpy(&value, bytes.constData()+pos, 4);
        pos+=4;
        return value;
    }
    double readDouble(){
        if(pos+8>bytes.length()) throw "unexpected EOF.";
        double value; memcpy(&value, bytes.constData()+pos, 8);
        pos+=8;
        return value;
    }

    QVariantMap readMap(){
        QVariantMap map;
        uint8_t dataType;
        int startPos=pos;
        forever{
            dataType=peekU8();
            if(dataType==STONDataTypeEOMLMarker){
                pos++;
                break;
            }
            QVariant key=parse();
            if(!key.canConvert(QVariant::String)){
                throw "key is not a string";
            }
            QVariant value=parse();
            map.insert(key.toString(), value);
        }
        return map;
    }

    QVariantList readList(){
        QVariantList lst;
        uint8_t dataType;
        forever{
            dataType=peekU8();
            if(dataType==STONDataTypeEOMLMarker){
                pos++;
                break;
            }
            QVariant value=parse();
            lst.append(value);
        }
        return lst;
    }

    QString readStringInTable(uint8_t type){
        uint32_t index;
        switch(type){
        case STONDataTypeString8: index=readU8(); break;
        case STONDataTypeString16: index=readU16(); break;
        case STONDataTypeString24: index=readU24(); break;
        case STONDataTypeString32: index=readU32(); break;
        default: Q_ASSERT(false); index=0;
        }
        if(index>=(uint32_t)strings.count())
            throw "string index out of range";
        return strings[(int)index];
    }

    QVariant readObjectRef(uint8_t type){
        uint32_t index;
        switch(type){
        case STONDataTypeObject8: index=readU8(); break;
        case STONDataTypeObject16: index=readU16(); break;
        case STONDataTypeObject24: index=readU24(); break;
        case STONDataTypeObject32: index=readU32(); break;
        default: Q_ASSERT(false); index=0;
        }
        if(index>=(uint32_t)objects.count())
            throw "object index out of range";
        if(index==0)
            throw "referencing root object";
        readObject(index);
        return objects[index].refObject;
    }

    quint64 readPositiveInteger(uint8_t type){
        switch(type){
        case STONDataTypePInteger8: return readU8(); break;
        case STONDataTypePInteger16: return readU16(); break;
        case STONDataTypePInteger24: return readU24(); break;
        case STONDataTypePInteger32: return readU32(); break;
        case STONDataTypePInteger48: return readU48(); break;
        case STONDataTypePInteger64: return readU64(); break;
        default: Q_ASSERT(false); return 0;
        }
    }

    qint64 readNegativeInteger(uint8_t type){
        switch(type){
        case STONDataTypeNInteger8: return -(qint64)readU8(); break;
        case STONDataTypeNInteger16: return -(qint64)readU16(); break;
        case STONDataTypeNInteger24: return -(qint64)readU24(); break;
        case STONDataTypeNInteger32: return -(qint64)readU32(); break;
        case STONDataTypeNInteger48: return -(qint64)readU48(); break;
        case STONDataTypeNInteger64: return -(qint64)readU64(); break;
        default: Q_ASSERT(false); return 0;
        }
    }

    QString readCharString(uint8_t type){
        ushort ucs;
        switch(type){
        case STONDataTypeChar8: ucs=readU8(); break;
        case STONDataTypeChar16:ucs=readU16(); break;
        default: Q_ASSERT(false); ucs=0;
        }
        return QString(QChar(ucs));
    }

    QString readStrSignedInteger(){
        char value=(char)readU8();
        return QString::number((int)value);
    }

    QString readStrUnsignedInteger(uint8_t type){
        quint64 value;
        switch(type){
        case STONDataTypeStrInteger8: value=readU8(); break;
        case STONDataTypeStrInteger16: value=readU16(); break;
        case STONDataTypeStrInteger32: value=readU32(); break;
        case STONDataTypeStrInteger64: value=readU64(); break;
        default: Q_ASSERT(false); value=0;
        }
        return QString::number(value);
    }

    int readSmallInteger(uint8_t type){
        Q_ASSERT(type&STONDataTypeIntegerBase);
        char v=(char)(type<<1);
        v>>=1;
        return (int)v;
    }

    QDateTime readDateTime(){
        return QDateTime::fromMSecsSinceEpoch(readU64());
    }

    QString readCommonString(){
        int index=readU8();
        QString s=STCommonStringAtIndex(index);
        if(s.isEmpty()) throw "undefined common string";
        return s;
    }



    QVariant parse(){
        uint8_t dataType=readU8();
        switch(dataType){
        case STONDataTypeNull:
            return QVariant();
        case STONDataTypeMap:
            return readMap();
        case STONDataTypeList:
            return readList();
        case STONDataTypeString8:
        case STONDataTypeString16:
        case STONDataTypeString24:
        case STONDataTypeString32:
            return readStringInTable(dataType);
        case STONDataTypeObject8:
        case STONDataTypeObject16:
        case STONDataTypeObject24:
        case STONDataTypeObject32:
            return readObjectRef(dataType);
        case STONDataTypePInteger8:
        case STONDataTypePInteger16:
        case STONDataTypePInteger24:
        case STONDataTypePInteger32:
        case STONDataTypePInteger48:
        case STONDataTypePInteger64:
            return readPositiveInteger(dataType);
        case STONDataTypeNInteger8:
        case STONDataTypeNInteger16:
        case STONDataTypeNInteger24:
        case STONDataTypeNInteger32:
        case STONDataTypeNInteger48:
        case STONDataTypeNInteger64:
            return readNegativeInteger(dataType);
        case STONDataTypeTrue:
            return true;
        case STONDataTypeFalse:
            return false;
        case STONDataTypeFloat:
            return readFloat();
        case STONDataTypeDouble:
            return readDouble();
        case STONDataTypeDateTime:
            return readDateTime();
        case STONDataTypeCommonString:
            return readCommonString();
        case STONDataTypeZeroInt:
            return 0;
        case STONDataTypeEmptyString:
            return QString();
        case STONDataTypeEOMLMarker:
            throw "unexpected end of list/map marker";
        case STONDataTypeChar8:
        case STONDataTypeChar16:
            return readCharString(dataType);
        case STONDataTypeStrSInteger8:
            return readStrSignedInteger();
        case STONDataTypeStrInteger8:
        case STONDataTypeStrInteger16:
        case STONDataTypeStrInteger32:
        case STONDataTypeStrInteger64:
            return readStrUnsignedInteger(dataType);
        default:
            if(dataType&STONDataTypeIntegerBase){
                return readSmallInteger(dataType);
            }else{
                throw "unknown data type";
            }
        }
        Q_ASSERT(false);
        return QVariant();
    }

    void readObject(uint32_t objectId){
        if(objectId>=(uint32_t)objects.count()){
            throw "referencing object that doesn't exist";
        }
        STONObject& obj=objects[(int)objectId];
        if(obj.isRead)
            return;
        if(obj.isReading){
            throw "circular reference";
        }
        obj.isReading=true;

        int oldPos=pos;

        pos=(int)obj.dataAddr;
        obj.data=parse();
        pos++;
        switch(obj.objectType){
        case STONObjectTypeRoot:
            obj.refObject=obj.data;
            break;
        case STONObjectTypeUser:
        {
            STObjectManager::UserRef ref=STObjectManager::sharedManager()->userFromSTON(obj.data);
            if(ref)
                obj.refObject=ref->toRefVariant();
        }
            break;
        case STONObjectTypeStatus:
        {
            STObjectManager::StatusRef ref=STObjectManager::sharedManager()->statusFromSTON(obj.data);
            if(ref)
                obj.refObject=ref->toRefVariant();
        }
            break;
        case STONObjectTypeMessage:
        {
            STObjectManager::MessageRef ref=STObjectManager::sharedManager()->messageFromSTON(obj.data);
            if(ref)
                obj.refObject=ref->toRefVariant();
        }
            break;
        case STONObjectTypeList:
        {
            STObjectManager::ListRef ref=STObjectManager::sharedManager()->listFromSTON(obj.data);
            if(ref)
                obj.refObject=ref->toRefVariant();
        }
            break;
        default:
            throw "unsupported object type found";
        }
        obj.isRead=true;
        obj.isReading=false;

        pos=oldPos;
    }

    void readObjectHeaders(){
        forever{
            uint8_t objectTypeAndSizeBits=readU8();
            if(objectTypeAndSizeBits==STONObjectNone)
                break;
            STONObject obj;
            obj.addr=pos-1;
            obj.objectType=objectTypeAndSizeBits&STONObjectTypeMask;
            switch((unsigned int)(objectTypeAndSizeBits&STONObjectSizeMask)){
            case (unsigned int)(uint8_t)STONObjectSizeBits8:
                obj.size=readU8();
                break;
            case (unsigned int)(uint8_t)STONObjectSizeBits16:
                obj.size=readU16();
                break;
            case (unsigned int)(uint8_t)STONObjectSizeBits24:
                obj.size=readU24();
                break;
            case (unsigned int)(uint8_t)STONObjectSizeBits32:
                obj.size=readU32();
                break;
            default:
                Q_ASSERT(false); // unreachable since mask is 2-bit width
                obj.size=0;
            }
            obj.dataAddr=pos;
            // skip data now
            pos+=obj.size;
            objects.append(obj);
        }
    }

    void readStringTable(){
        forever{
            uint64_t length=readULEB64();
            if(length==0)
                break;
            if(bytes.length()-pos<(int)length){
                throw "unexpected EOF during reading string table";
            }

            QString str;
            str.reserve(length);

            while(length--){
                ushort ucs=readULEB64();
                str.append(QChar(ucs));
            }

            strings.append(str);
        }
    }

public:
    STONParserContext(const QByteArray& bt):
        bytes(bt), pos(0){
        if(!bt.startsWith(STONHeaderMagic)){
            throw "invalid header.";
        }
        if(bt.size()<13){
            throw "malformated (too short).";
        }
        pos=4;
        uint32_t expectedSize=readU32();
        uint32_t strAddr=readU32();

        if((uint32_t)bt.size()<expectedSize)
            throw "malformated (smaller than embedded size information)";
        if(strAddr>=expectedSize)
            throw "malformated (string table address beyond EOF)";

        readObjectHeaders();

        pos=(int)strAddr;
        readStringTable();

        if(objects.isEmpty())
            throw "root object not found";

    }

    QVariant rootObject(){
        readObject(0);
        return objects[0].refObject;
    }
};

QVariant STObjectNotation::parse(const QByteArray& bytes){
#if 0
    try{
        STONParserContext context(bytes);
        return context.rootObject();
    }catch(const char *err){
        qDebug()<<"STObjectNotation::parse: "<<err;
        return QVariant();
    }
#else
    STONParserContext context(bytes);
    return context.rootObject();
#endif
}

bool STObjectNotation::serialize(QVariant var, QString path, bool atomic){
    if(atomic && QFile::exists(path)){
        QString tmpPath=path+"-tmp";
        QFile fl(tmpPath);
        if(fl.open(QFile::WriteOnly|QFile::Truncate)){
            fl.write(serialize(var));
            if(!fl.flush()){
                fl.close();
                fl.remove();
                return false;
            }
            fl.close();
        }else{
            return false;
        }

        QString tmpPath2=path+"-old";
        if(!QFile::rename(path, tmpPath2)){
            QFile::remove(tmpPath);
            return false;
        }
        if(!QFile::rename(tmpPath, path)){
            QFile::remove(tmpPath);
            QFile::rename(tmpPath2, path);
            return false;
        }
        if(!QFile::remove(tmpPath2)){
            return false;
        }

        return true;
    }else{
        QFile fl(path);
        if(fl.open(QFile::WriteOnly|QFile::Truncate)){
            fl.write(serialize(var));
            fl.close();
            return true;
        }else{
            return false;
        }
    }
}

QVariant STObjectNotation::parse(QString path){
    QFile fl(path);
    if(!fl.open(QFile::ReadOnly)){
        return QVariant();
    }
    QByteArray data=fl.readAll();
    fl.close();
    return parse(data);
}


