#ifndef STMAINWINDOW_H
#define STMAINWINDOW_H

#include <QMainWindow>
#include "stmainview.h"

namespace Ui {
class STMainWindow;
}

class STMainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit STMainWindow(QWidget *parent = 0);
    ~STMainWindow();


    virtual void closeEvent(QCloseEvent *);

private:
    Ui::STMainWindow *ui;
};

#endif // STMAINWINDOW_H
