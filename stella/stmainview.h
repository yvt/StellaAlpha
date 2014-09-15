#ifndef STMAINVIEW_H
#define STMAINVIEW_H

#include "ststdafx.h"
#include <QGraphicsView>

class STStandardView;
class STAuthorizeView;
class STStartupView;

class STMainView : public QGraphicsView
{
    Q_OBJECT

    QSet<QWidget *> m_openGLincompatibleWidgets;
    STStandardView *m_mainItem;
    STStartupView *m_startupView;
    bool m_useOpenGL;
    bool m_newViewportReq;



public:
    explicit STMainView(QWidget *parent = 0);
    virtual ~STMainView();
    
    void setOpenGLEnable(bool);

    void addOpenGLIncompatibleWidget(QWidget *);
    void removeOpenGLIncompatibleWidget(QWidget *);

    void startAuthorizeView(STAuthorizeView *, QPoint btnPos, QRect rt);
    void stopAuthorizeView(STAuthorizeView *);

protected:
    virtual void resizeEvent(QResizeEvent *event);
   // virtual void wheelEvent(QWheelEvent *event);
private slots:
     void setupNewViewport();
     void startInitialization();
     void startupDone();
signals:

};

#endif // STMAINVIEW_H
