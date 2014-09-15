#ifndef STAUTHORIZEVIEW_H
#define STAUTHORIZEVIEW_H

#include <QWidget>
#include <QUrl>

class STAuthorizeCancelButton;
class STWebView;

class STAuthorizeView : public QWidget
{
    Q_OBJECT
    QPixmap m_bgPixmap;
    STAuthorizeCancelButton *m_cancelButton;
    STWebView *m_webView;
public:
    explicit STAuthorizeView(QWidget *parent = 0);
    void start(QPixmap, QPoint, QRect);
    virtual void paintEvent(QPaintEvent *);

    void setUrl(QUrl);

signals:
    void cancelled();
public slots:
    void cancelButtonPressed();
};

#endif // STAUTHORIZEVIEW_H
