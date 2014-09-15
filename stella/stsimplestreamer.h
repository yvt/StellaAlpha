#ifndef STSIMPLESTREAMER_H
#define STSIMPLESTREAMER_H

#include "ststreamer.h"
#include "stobjectmanager.h"

class STAccount;
class STSimpleStreamer : public STStreamer
{
    Q_OBJECT
public:
    explicit STSimpleStreamer(STAccount *, QUrl);
protected:
    virtual bool isStatusFiltered(const STObjectManager::StatusRef&){return false;}
signals:
    void statusReceived(STObjectManager::StatusRef);
    void statusRemoved(quint64);
public slots:
    virtual void lineReceived(const QByteArray&);
};

#endif // STSIMPLESTREAMER_H
