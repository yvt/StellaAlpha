#include "stplaceview.h"
#include "stlinkedlabelview.h"
#include "stremoteimageview.h"
#include "ststaticmapimagemanager.h"
#include "stmath.h"

STPlaceView::STPlaceView(QGraphicsItem *parent) :
    QGraphicsObject(parent),m_size(0,0)
{
    setFlag(ItemHasNoContents);

    m_nameView=new STLinkedLabelView(this);
    m_coordView=new STLinkedLabelView(this);
    m_mapView=new STRemoteImageView(this);

    m_nameView->setBgColor(QColor(70,70,70));
    m_coordView->setBgColor(QColor(70,70,70));


    m_lastWidth=0;

    connect(m_coordView, SIGNAL(linkActivated(STObjectManager::EntityRange*,
                                              STObjectManager::LinkActivationInfo)),
            this, SLOT(childLinkActivated(STObjectManager::EntityRange*,
                                          STObjectManager::LinkActivationInfo)));
}

void STPlaceView::setObjects(QVariantMap coords, QVariantMap place){
    m_coords=coords;
    m_place=place;
    relayout();
}

void STPlaceView::setSize(QSize sz){
    if(sz==m_size)return;
    prepareGeometryChange();
    m_size=sz;
    relayout();
}

void STPlaceView::relayout(){
    if(m_size.isEmpty())
        return;
    if(m_coords.isEmpty() && m_place.isEmpty())
        return;

    bool loaded=m_lastWidth!=m_size.width();
    m_lastWidth=m_size.width();

    const int labelHeight=16;
    QSize mapSize(m_size.width(), m_size.height()-labelHeight);
    m_mapView->setSize(mapSize);

    if((!m_coords.isEmpty()) || (!m_place.isEmpty())){
        if((!mapSize.isEmpty()) &&  !m_mapView->ticket()){
            m_mapView->setTicket(STStaticMapImageManager::sharedManager()->requestWithObjects(m_coords,
                                                                                              m_place,
                                                                                              mapSize-QSize(4,4)));
        }
    }

    if(loaded){
        if(m_coords.isEmpty()){

        }else{
            QString txt;
            STMapCoord crd(m_coords["coordinates"]);
            if(crd.latitude>=0.){
                txt=tr(":degN, ").replace(QLatin1String(":deg"),
                                           QString::number(crd.latitude, 'f', 1));
            }else{
                txt=tr(":degS, ").replace(QLatin1String(":deg"),
                                           QString::number(-crd.latitude, 'f', 1));
            }
            if(crd.longitude>=0.){
                txt+=tr(":degE").replace(QLatin1String(":deg"),
                                          QString::number(crd.longitude, 'f', 1));
            }else{
                txt+=tr(":degW").replace(QLatin1String(":deg"),
                                          QString::number(-crd.longitude, 'f', 1));
            }

            QList<STObjectManager::EntityRange> ranges;
            STObjectManager::EntityRange range;
            range.charIndexStart=0;
            range.charIndexEnd=txt.length();
            range.entityType="st_url";
            range.entity=QString("https://maps.google.com/maps?q=%1,%2&z=5").arg(QString::number(crd.latitude, 'f'),
                                                                                 QString::number(crd.longitude, 'f'));
            ranges.append(range);
            m_coordView->setContents(STFont::defaultFont(), txt, ranges);
        }
    }

    int labelPos=m_size.height()-labelHeight+3;

    int coordsLeft=m_size.width()-(int)m_coordView->boundingRect().right();
    m_coordView->setPos(coordsLeft, labelPos);


    if(loaded){
        if(!m_place.isEmpty()){
            QString txt=m_place["name"].toString();
            m_nameView->setContents(STFont::defaultBoldFont(), txt,(float)coordsLeft+m_coordView->boundingRect().left()-3,true);

        }
    }

    m_nameView->setPos(1, labelPos);

}
void STPlaceView::childLinkActivated(STObjectManager::EntityRange *e,
                                     STObjectManager::LinkActivationInfo i){
    emit linkActivated(e,i);
}

void STPlaceView::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){

}

QRectF STPlaceView::boundingRect() const{
    return QRectF(0,0,m_size.width(),m_size.height());
}



