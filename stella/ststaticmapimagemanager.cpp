#include "ststaticmapimagemanager.h"
#include "stmath.h"
#include <QDebug>

STStaticMapImageManager::STStaticMapImageManager() :
    STRemoteImageManager()
{
    m_unusedTicketsLimit=1;
}


STStaticMapImageManager *STStaticMapImageManager::sharedManager(){
    static STStaticMapImageManager *manager=NULL;
    if(!manager){
        manager=new STStaticMapImageManager();
    }
    return manager;
}





STRemoteImageManager::TicketRef STStaticMapImageManager::requestWithObjects(QVariantMap coords, QVariantMap place, QSize size){
    QUrl url("http://maps.google.com/maps/api/staticmap");
    STMapCoord coord=STMapCoord(coords["coordinates"]);

    if(place.isEmpty()){
        invalidPlaceBoundingBox:
        url.addQueryItem("zoom", "3");
        url.addQueryItem("center", coord.toString());
    }else{
        QStringList bits;
        bits.append("weight:2");
        bits.append("color:red");
        bits.append("fillcolor:0x00000000");
        bits.append(""); // place holder
        bits.append(""); // place holder
        bits.append(""); // place holder

        QVariantMap bndBox=place["bounding_box"].toMap();
        if(QString::compare(bndBox["type"].toString(),
                            QLatin1String("Polygon"),
                            Qt::CaseInsensitive)){
            goto invalidPlaceBoundingBox;
        }

        QVariantList bndCoords=bndBox["coordinates"].toList();
        if(bndCoords.count()<1)
            goto invalidPlaceBoundingBox;
        bndCoords=bndCoords[0].toList();
        if(bndCoords.count()<3)
            goto invalidPlaceBoundingBox;
        QVarLengthArray<STMapCoord, 4> bndMapCoords;
        bndMapCoords.reserve(bndCoords.count());
        foreach(QVariant var, bndCoords){
            bndMapCoords.append(STMapCoord(var));
        }

        const double crossHairSize=.06;

        for(int i=0;i<bndMapCoords.count();i++){
            bits[4]=bndMapCoords[i].toString();
            bits[3]=STMapCoord::lerp(bndMapCoords[i], bndMapCoords[(i+1)%bndMapCoords.count()], crossHairSize).toString();
            bits[5]=STMapCoord::lerp(bndMapCoords[i], bndMapCoords[(i+bndMapCoords.count()-1)%bndMapCoords.count()], crossHairSize).toString();
            url.addQueryItem("path", bits.join("|"));
        }
    }

    url.addQueryItem("sensor", "false");

    if(!coords.isEmpty()){
        QStringList bits;
        bits.append("size:mid");
        bits.append("color:blue");
        bits.append(coord.toString());
        url.addQueryItem("markers", bits.join("|"));
    }

    url.addQueryItem("size", QString("%1x%2").arg(size.width()).arg(size.height()));

    //qDebug()<<url;

    return requestUrl(url);
}
