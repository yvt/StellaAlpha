#ifndef STTWITTEROBJECTMANAGER_H
#define STTWITTEROBJECTMANAGER_H

#include <QObject>
#include <QVariant>
#include <QMap>
#include <QVariantMap>
#include <QMutex>
#include <QList>
#include <QSharedData>
#include <QDateTime>
#include <QSet>
#include <QVarLengthArray>

class STAccount;

class STObjectManager : public QObject
{
    Q_OBJECT
public:
    explicit STObjectManager(QObject *parent = 0);
    static STObjectManager *sharedManager();

    struct Object;
    struct Status;
    struct User;
    struct Message;
    struct List;



    struct ObjectPool{
        QMap<quint64, Object *> objects;
        QMutex *m_mutex;
        Object *object(quint64) const;

        QMutex& mutex(){
            if(!m_mutex){
                m_mutex=new QMutex(QMutex::Recursive);
            }
            return *m_mutex;
        }

        ObjectPool():m_mutex(NULL){}
    };

    struct Object{
        quint64 id;
        unsigned int refCount;
        QVariantMap data;
        qint64 loadedTime;
        ObjectPool *pool;
        QMutex mutex;

        Object(quint64, ObjectPool *pool);
        virtual ~Object();

        void retain();
        void release();

        virtual QVariant toSTONVariant();   // variant that contains map/list/something data
        virtual QVariant toRefVariant();       // variant contains reference
    };

    template<typename T>
    class Ref{
        T *ptr;
    public:
        typedef Ref<T> ThisType;
        Ref():ptr(NULL){}
        Ref(T *p):ptr(p){
            if(ptr)
            ptr->retain();
        }
        Ref(const ThisType& v):ptr(v.ptr){
            if(ptr)
            ptr->retain();
        }
        template<typename T2>
        Ref(const Ref<T2>& v):ptr(v?const_cast<T *>(dynamic_cast<const T *>(&*v)):NULL){
            if(ptr)
                ptr->retain();
        }

        ~Ref(){
            if(ptr){
                ptr->release();
            }
        }
        operator bool() const{
            return ptr!=NULL;
        }
        bool operator !()const{
            return ptr==NULL;
        }
        const T *operator ->() const{
            Q_ASSERT(ptr!=NULL);
            return ptr;
        }
        T *operator ->(){
            Q_ASSERT(ptr!=NULL);
            return ptr;
        }

        const T& operator *() const{
            return *ptr;
        }
        T& operator *(){
            return *ptr;
        }

        void operator =(const ThisType& v){
            if(ptr==v.ptr)return;
            if(ptr)ptr->release();
            ptr=v.ptr;
            if(ptr)ptr->retain();
        }
    };

    typedef Ref<Object> ObjectRef;

    struct EntityRange{
        int charIndexStart;
        int charIndexEnd;
        QString entityType;
        QVariant entity;
        QString displayText; // set non-null to substitute
        Ref<Object> stObject;

        bool operator =(const EntityRange& other) const{
            return charIndexStart==other.charIndexStart &&
                    charIndexEnd==other.charIndexEnd &&
                    entityType==other.entityType &&
                    entity==other.entity;
        }
    };

    struct LinkActivationInfo{
        bool needsContextMenu;
        quint64 accountToOpen;
        LinkActivationInfo(){
            needsContextMenu=false;
            accountToOpen=0;
        }
    };

    struct Status:public Object{
        Ref<User> user;
        Ref<Status> retweetedStatus;
        QDateTime createdAt;
        QString displayText;
        QList<EntityRange> entityRanges;
        struct RetweetInfo{
            quint64 accountId;
            quint64 retweetStatusId;
        };

        QVarLengthArray<quint64, 1> favoritedBy;
        QVarLengthArray<RetweetInfo, 1> retweets;

        Status(quint64 i, ObjectPool *p):Object(i,p){}
        virtual ~Status(){}

        void setFavorited(quint64 accountId, bool favorited);
        bool isFavoritedBy(quint64 accountId) const;

        void setRetweetStatusId(quint64 accountId, quint64 retweetStatusId);
        quint64 retweetStatusId(quint64 accountId) const;

        virtual QVariant toSTONVariant();
        virtual QVariant toRefVariant();

        static int refUserType();
    };

    typedef Ref<Status> StatusRef;
    typedef QList<StatusRef> StatusRefList;

    Ref<Status> status(const QVariant&, bool weak=false, quint64 accountId=0);
    Ref<Status> statusFromSTON(QVariant);

    struct User:public Object{

        QString displayDescription;
        QString displayUrl;

        QList<EntityRange> descriptionEntityRanges;
        QList<EntityRange> urlEntityRanges;

        User(quint64 i, ObjectPool *p):Object(i,p){}
        virtual ~User(){}

        virtual QVariant toSTONVariant();
        virtual QVariant toRefVariant();

        static int refUserType();
    };

    typedef Ref<User> UserRef;
    typedef QList<UserRef> UserRefList;

    Ref<User> user(const QVariant&);
    Ref<User> userFromSTON(QVariant);

    struct Message: public Object{
        Ref<User> recipient;
        Ref<User> sender;

        Message(quint64 i, ObjectPool *p):Object(i,p){}
        virtual ~Message(){}

        virtual QVariant toSTONVariant();
        virtual QVariant toRefVariant();

        static int refUserType();
    };

    typedef Ref<Message> MessageRef;

    Ref<Message> message(const QVariant&);
    Ref<Message> messageFromSTON(QVariant);

    struct List: public Object{
        Ref<User> owner;

        List(quint64 i, ObjectPool *p):Object(i,p){}
        virtual ~List(){}

        virtual QVariant toSTONVariant();
        virtual QVariant toRefVariant();

        static int refUserType();
    };

    typedef Ref<List> ListRef;

    Ref<List> list(const QVariant&);
    Ref<List> listFromSTON(QVariant);



signals:
    
public slots:
    
};

Q_DECLARE_METATYPE(STObjectManager::StatusRef);
Q_DECLARE_METATYPE(STObjectManager::UserRef);
Q_DECLARE_METATYPE(STObjectManager::MessageRef);
Q_DECLARE_METATYPE(STObjectManager::ListRef);


#endif // STTWITTEROBJECTMANAGER_H
