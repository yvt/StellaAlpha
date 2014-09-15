#include "stnewaccountview.h"
#include "stnewaccountstartpane.h"

STNewAccountView::STNewAccountView(QGraphicsItem *parent) :
    STBaseTabPageView(0, parent)
{
    pushPane(new STNewAccountStartPane(this));
}
