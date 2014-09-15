#ifndef STSEARCHTIMELINE_H
#define STSEARCHTIMELINE_H

#include "sttimeline.h"

class STSearchTimeline : public STTimeline
{
    Q_OBJECT
public:
    explicit STSearchTimeline(QString identifier, QString key, QObject *parent = 0);
    
signals:
    
public slots:
    
};

#endif // STSEARCHTIMELINE_H
