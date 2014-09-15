#ifndef STSINGLEUSERSTREAMER_H
#define STSINGLEUSERSTREAMER_H

#include "stsimplestreamer.h"

class STSingleUserStreamer : public STSimpleStreamer
{
    Q_OBJECT
    quint64 m_targetUserId;
public:
    explicit STSingleUserStreamer(STAccount *ac, quint64 uid);
    quint64 targetUserId() const{return m_targetUserId;}
protected:
    virtual bool isStatusFiltered(const STObjectManager::StatusRef &);
signals:
    
public slots:
    
};

#endif // STSINGLEUSERSTREAMER_H
