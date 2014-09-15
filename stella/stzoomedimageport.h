#ifndef STZOOMEDIMAGEPORT_H
#define STZOOMEDIMAGEPORT_H

#include "ststdafx.h"
#include <QGraphicsView>

class STZoomedImageView;

class STZoomedImagePort : public QGraphicsView
{
    Q_OBJECT

    STZoomedImageView *m_view;

public:
    explicit STZoomedImagePort(QWidget *parent = 0);
    
    virtual void resizeEvent(QResizeEvent *event);

    void setImage(QUrl, QPixmap);
    void setProgressValue(float);
signals:
    
public slots:
    
};

#endif // STZOOMEDIMAGEPORT_H
