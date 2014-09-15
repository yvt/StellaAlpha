#include "stsingleuserstreamer.h"

STSingleUserStreamer::STSingleUserStreamer(STAccount *ac, quint64 uid) :
    STSimpleStreamer(ac, QUrl(QString("https://stream.twitter.com/1.1/statuses/filter.json?follow=%1").arg(uid))),
    m_targetUserId(uid)
{
}

bool STSingleUserStreamer::isStatusFiltered(const STObjectManager::StatusRef &ref){
    return ref->user->id!=m_targetUserId;
}
