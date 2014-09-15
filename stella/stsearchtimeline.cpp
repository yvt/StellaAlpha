#include "stsearchtimeline.h"

STSearchTimeline::STSearchTimeline(QString idenifier, QString key, QObject *parent) :
    STTimeline(idenifier, parent)
{
    QUrl url("https://api.twitter.com/1.1/search/tweets.json");
    url.addQueryItem("q", key);
    setFetchEndpoint(url);
}
