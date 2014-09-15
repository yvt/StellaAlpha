#ifndef STWEBVIEW_H
#define STWEBVIEW_H

#include "ststdafx.h"
#include <QWidget>

class STWebViewPrivate;
class STWebView : public QWidget
{
    Q_OBJECT
    QSize m_size;
public:
    explicit STWebView(QWidget *parent = 0);
    virtual ~STWebView();

signals:
    
public slots:
    void setUrl(const QUrl&);


protected:
    STWebViewPrivate *d_ptr;
    virtual void resizeEvent(QResizeEvent *);
private:
    Q_DECLARE_PRIVATE(STWebView);
};

#endif // STWEBVIEW_H
