#ifndef STZOOMEDIMAGEWINDOW_H
#define STZOOMEDIMAGEWINDOW_H

#include "ststdafx.h"
#include <QDialog>

namespace Ui {
class STZoomedImageWindow;
}

class STZoomedImageWindow : public QDialog
{
    Q_OBJECT
    QUrl m_url;
public:
    explicit STZoomedImageWindow(QWidget *parent = 0);
    ~STZoomedImageWindow();

    void setImage(QUrl, QPixmap);
    void setProgressValue(float);

    virtual void keyPressEvent(QKeyEvent *);
    virtual void closeEvent(QCloseEvent *);
    
    QUrl url() const{return m_url;}

private:
    Ui::STZoomedImageWindow *ui;
signals:
    void willClose();
};

#endif // STZOOMEDIMAGEWINDOW_H
