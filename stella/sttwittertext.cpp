#include "sttwittertext.h"
#include <QtScript>
#include <QMutex>
#include <QDebug>
#include <QMutexLocker>
#include "stfont.h"

static QMutex *g_processMutex=NULL;
static QScriptEngine *g_engine=NULL;
static QScriptValue *g_tweetLengthFunction=NULL;
static QScriptValue *g_extractMentionsFunction=NULL;

STTwitterText::STTwitterText(QObject *parent) :
    QObject(parent)
{
    if(!g_processMutex)
        g_processMutex=new QMutex();
    QString fileName = ":/stella/res/twitter-text.js";
    QFile scriptFile(fileName);
    if (!scriptFile.open(QIODevice::ReadOnly)){
        qFatal("Couldn't open twitter-text.js from resource.");
    }

     QTextStream stream(&scriptFile);
     QString contents = stream.readAll();
     scriptFile.close();

     if(!g_engine){
         g_engine=new QScriptEngine();
     }

     QScriptValue result =g_engine->evaluate(contents, fileName);

     if(g_engine->hasUncaughtException()){
         int line = g_engine->uncaughtExceptionLineNumber();
         QByteArray str=g_engine->uncaughtException().toString().toUtf8();
         qFatal("Exception while evaluating twitter-text.js at line %d: %s", line, str.data());
     }

     {
         QScriptValue result=g_engine->evaluate("function f(text, opt){return twttr.txt.getTweetLength(text, opt);} f");

         g_tweetLengthFunction=new QScriptValue(result);
         if(!g_tweetLengthFunction->isFunction()){
             int line = g_engine->uncaughtExceptionLineNumber();
             QByteArray str=g_engine->uncaughtException().toString().toUtf8();
             qFatal("Exception while linking getTweetLength at line %d: %s", line, str.data());
         }
     }
     {
         QScriptValue result=g_engine->evaluate("function f(text){return twttr.txt.extractMentions(text);} f");
         g_extractMentionsFunction=new QScriptValue(result);
         if(!g_extractMentionsFunction->isFunction()){
             int line = g_engine->uncaughtExceptionLineNumber();
             QByteArray str=g_engine->uncaughtException().toString().toUtf8();
             qFatal("Exception while linking extractMentions at line %d: %s", line, str.data());
         }
     }
}

STTwitterText *STTwitterText::sharedInstance(){
    static STTwitterText *instance=NULL;
    if(!instance){
        instance=new STTwitterText();
    }
    return instance;
}

static QString removeUni6(const QString& text){
    QString strs;
    strs.reserve(text.length());
    int i=0;
    const QChar *chrs=text.data();
    int len=text.length();
    while(i<len){
        if(i<len-1){
            if(STFont::isLeadingSurrogatePair(chrs[i].unicode()) &&
                    STFont::isTrailSurrogatePair(chrs[i+1].unicode())){
                strs+=QChar(0xe000);
                i+=2;
                continue;
            }
        }
        strs+=chrs[i];
        i++;
    }


    return strs;
}

int STTwitterText::lengthOfTweet(const QString &text){
    QMutexLocker locker(g_processMutex);
    //g_engine->globalObject().setProperty("text", );
    QString inText=removeUni6(text).trimmed().normalized(QString::NormalizationForm_C);
    QScriptValue caller();
    QScriptValueList args;
    args.append(inText);
    QScriptValue result=g_tweetLengthFunction->call(caller, args);
    if(g_engine->hasUncaughtException()){
        int line = g_engine->uncaughtExceptionLineNumber();

        QByteArray str=g_engine->uncaughtException().toString().toUtf8();//result.toString().toUtf8();

        qFatal("Exception while evaluating twttr.txt.getTweetLength at line %d: %s", line, str.data());
    }
    if(result.isUndefined()){
        qFatal("Undefined returned by getTweetLength");
    }

    int vl=result.toInteger();
    //qDebug()<<vl;
    return vl;
}

QStringList STTwitterText::extractMentions(const QString &text){
    QMutexLocker locker(g_processMutex);
    QString inText=removeUni6(text).trimmed().normalized(QString::NormalizationForm_C);
    QScriptValue caller();
    QScriptValueList args;
    args.append(inText);
    QScriptValue result=g_extractMentionsFunction->call(caller, args);
    if(g_engine->hasUncaughtException()){
        int line = g_engine->uncaughtExceptionLineNumber();

        QByteArray str=g_engine->uncaughtException().toString().toUtf8();

        qFatal("Exception while evaluating twttr.txt.extractMentions at line %d: %s", line, str.data());
    }
    if(result.isUndefined()){
        qFatal("Undefined returned by extractMentions");
    }
    if(!result.isArray()){
        qFatal("Non-array object returned by extractMentions");
    }

    int length=result.property("length").toInteger();
    QStringList res;
    for(int i=0;i<length;i++){
        res<<result.property(i).toString();
    }

    return res;
}
