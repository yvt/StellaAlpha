#include <QtGui/QApplication>
#include "stmainwindow.h"
#include <QTextCodec>
#include <QTranslator>
#include <QLibraryInfo>
#include "sttwittertext.h"
#include "staccountsmanager.h"
#include "stactionmanager.h"
#include "stusericonmanager.h"
#include "stobjectmanager.h"
#include <QFont>
#include <QStringList>
#include "stjsonparser.h"
#include <QJson/Parser>
#include <QFile>
#include <QDebug>
#include <QJson/Serializer>
#include <time.h>
#include "stobjectnotation.h"
#if QT_MAC_USE_COCOA && 0
#define USE_STELLA_CSS 1
#include <QWindowsStyle>
#endif
#include <QTranslator>

#if USE_STELLA_CSS
static void applyStellaCSS(){


    app.setStyle("windows");

    app.setStyleSheet("*{font-family: 'Stella Sans'; font-size: 11pt; color: #ddd;}"
                        "QMenuBar{ background-color: silver; "
                        "background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #555, stop:1 #303030);"
                        " color: #ddd; border-bottom: 1px solid #222; padding: 2px 2px;}"
                        "QMenuBar::item{border-radius: 2px; padding: 2px 4px; border-radius: 2px;"
                        " background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop: 0 #606060 ,stop:0.05 #585858, stop:1 #303030) }"
                        "QMenuBar::item:selected{ border: 1px solid #222;  "
                        "  background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop: 0 #a0a0a0 ,stop:0.2 #404040, stop:1 #888); }"
                        "QMenuBar::item:pressed"
                        "{ background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop: 0 #777 ,stop:0.2 #222, stop:1 #777); }"

                        "QMenu"
                        "{ background-color: #444; }"
                        "QMenu::item"
                        "{ border-radius: 2px; border: 1px solid transparent; padding: 2px;}"
                        "QMenu::item:selected"
                        "{ border: 1px solid #222; color: white;"
                        "  background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop: 0 #a0a0a0 ,stop:0.2 #404040, stop:1 #aaa); }"
    /*
                        "QLineEdit, QTextEdit"
                        "{background-color: #444; border-image: url(:/stella/theme/TextEdit.png) 2 stretch; border-width: 2px;}"
                        "QLineEdit:focus"
                        "{border-image: url(:/stella/theme/TextEditFocus.png) 2 stretch; border-width: 2px;}"
    */
                        "QCheckBox {spacing: 5px;}"
                        "QCheckBox::indicator {width: 16px; height: 16px;}"
                        "QCheckBox::indicator:unchecked"
                        "{image: url(:/stella/theme/CheckOff.png);}"
                        "QCheckBox::indicator:unchecked:hover"
                        "{image: url(:/stella/theme/CheckOffHover.png);}"
                        "QCheckBox::indicator:unchecked:pressed"
                        "{image: url(:/stella/theme/CheckOffPress.png);}"
                        "QCheckBox::indicator:checked"
                        "{image: url(:/stella/theme/CheckOn.png);}"
                        "QCheckBox::indicator:checked:hover"
                        "{image: url(:/stella/theme/CheckOnHover.png);}"
                        "QCheckBox::indicator:checked:pressed"
                        "{image: url(:/stella/theme/CheckOnPress.png);}"
    /*
                        "QComboBox { padding: -1px 4px;}"
                        "QPushButton { padding: 1px 4px; } "
                        "QPushButton, QComboBox"
                        "{background-color: #444;  color: #ddd; "
                        "border-image: url(:/stella/theme/ButtonRelease.png) 3 stretch; border-width: 3px; }"
                        "QPushButton:hover, QComboBox:hover"
                        "{ border-image: url(:/stella/theme/ButtonHover.png) 3 stretch; }"
                        "QPushButton:pressed, QComboBox:open"
                        "{ border-image: url(:/stella/theme/ButtonPress.png) 3 stretch; }"
                        "QPushButton:disabled, QComboBox:disabled"
                        "{border-image: url(:/stella/theme/ButtonDisabled.png) 3 stretch; border-width: 3px; }"

                        "QToolButton {background: none; border: none;}"
                        "QToolButton:hover"
                        "{ border: 1px solid #222;"
                        "  background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop: 0 #a0a0a0 ,stop:0.2 #404040, stop:1 #aaa); }"
                        "QToolButton:pressed"
                        "{background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop: 0 #777 ,stop:0.2 #222, stop:1 #777); }"

                        "QComboBox::down-arrow{ color: silver ; background: url(:/stella/theme/ScrollDownArrow.png); width: 5px; height: 5px;}"
                        "QComboBox::drop-down {   border: 0px solid red;}"
                        "QComboBox QAbstractItemView"
                        "{ background: #333; border: 1px solid #222; color: #ddd; border-image: none;"
                        "  selection-background-color: #c08010;"
                        "  selection-color: white;"
                        "}"

                        "QToolBar{  color: #ddd; border: none;"
                        "  border-bottom: 1px solid #222; padding: 2px 2px;"
                        "  background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop: 0 #2a2a2a ,stop:0.2 #333,stop:0.8 #333, stop:1 #2a2a2a); }"


                        " QTreeWidget, QListView, QListWidget, QScrollArea, QsciScintilla, QWebView "
                        "{border-image: url(:/stella/theme/ListBox.png) 2 stretch; border-width: 2px; color: #ddd;"
                        " alternate-background-color : #202020; background-color: #282828;}"
                        "SHTreeView, SHDetailedViewer"
                        "{border-image: url(:/stella/theme/ListBox.png) 2 stretch; border-width: 2px;color: #ddd; }"
                        "QTreeView::item:selected:active{"
                        "  background-color: #c08010;"
                        "  color: white;"
                        "}"
                        "QTreeView::item:selected:!active{"
                        "  background-color: #404040;"
                        "  color: white;"
                        "}"
                        "QTreeView::item{padding: 0px 0px; min-height: 16px;}"
                        "QTreeView QLineEdit{"
                        "  background: none; border-image: url(:/stella/theme/Transparent.png) !important; "
                        "border: none !important; margin: -2px 0px; "
                        "}"

                        "QHeaderView, QHeaderView::section "
                        "{ "
                        "background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #3f3f3f, stop:1 #393939); }"
                        "QHeaderView::section {  border: 1px solid #2b2b2b; border-top: none; margin-right: -1px; padding: 2px 2px;}"

                        "QWidget#centralWidget, QDialog { background: #333; } "

                        "QSplitter::handle {background: none;}"

                        "QTabWidget { border: none; } "
                        "QTabWidget::tab-bar{left: 2px;}"
                        "QTabWidget > * { background: #444;  }"
                        "QTabWidget > QTabBar { background: #333;  }"

                        "QTabBar::close-button {"
                        "   image: url(:/stella/theme/CloseTab.png);"
                        "   subcontrol-position: left; margin-right: -2px;"
                        "}"
                        "QTabBar::close-button:hover {"
                        "   image: url(:/stella/theme/CloseTabHover.png);"
                        "}"
                        "QTabBar::close-button:pressed {"
                        "   image: url(:/stella/theme/CloseTabPress.png);"
                        "}"


                        "QTabWidget::tab-bar"
                        "{ border: none; top: 0px; }"
                        "QTabBar::tab"
                        "{ background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop: 0 #303030 , stop:1 #202020); min-width: 160px;"
                        " border: 1px solid #222; border-bottom: none; padding: 2px 3px; padding-right: 5px;margin-bottom: 0px;margin-right: -1px;  }"
                        "QTabBar::tab:selected"
                        "{ background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop: 0 #353535 , stop:1 #444);}"
    */
                        "QScrollBar::horizontal{"
                        "  border-top: 1px solid #222;"
                        "  border-bottom: 1px solid #222;"
                        "  background: #333;"
                        "  height: 14px;"
                        "  margin: 0px 14px 0px 14px;"
                        "}"
                        "QScrollBar::sub-page, QScrollBar::add-page{"
                        "  background: none;"
                        "}"
                        "QScrollBar::handle:horizontal {"
                        "  border-left: 1px solid #222;"
                        "  border-right: 1px solid #222;"
                        "  background: #444;"
                        "  min-width: 7px;"
                        "}"
                        "QScrollBar::handle:horizontal:hover {"
                        "  background: #4d4d4d;"
                        "}"
                        "QScrollBar::handle:horizontal:pressed {"
                        "  background: #3a3a3a;"
                        "}"
                        "QScrollBar::add-line:horizontal {"
                        "  border: 1px solid #222;"
                        "  background: #444;"
                        "  width: 13px;"
                        "  subcontrol-position: right;"
                        "  subcontrol-origin: margin;"
                        "}"
                        "QScrollBar::sub-line:horizontal {"
                        "  border: 1px solid #222;"
                        "  background: #444;"
                        "  width: 13px;"
                        "  subcontrol-position: left;"
                        "  subcontrol-origin: margin;"
                        "}"
                        "QScrollBar::add-line:horizontal:hover,"
                        "QScrollBar::sub-line:horizontal:hover {"
                        "  background: #4d4d4d;"
                        "}"
                        "QScrollBar::add-line:horizontal:pressed,"
                        "QScrollBar::sub-line:horizontal:pressed {"
                        "  background: #3a3a3a;"
                        "}"

                        "QScrollBar::vertical{"
                        "  border-left: 1px solid #222;"
                        "  border-right: 1px solid #222;"
                        "  background: #333;"
                        "  width: 14px;"
                        "  margin: 14px 0px 14px 0px;"
                        "}"
                        "QScrollBar::handle:vertical {"
                        "  border-top: 1px solid #222;"
                        "  border-bottom: 1px solid #222;"
                        "  background: #444;"
                        "  min-height: 7px;"
                        "}"
                        "QScrollBar::handle:vertical:hover {"
                        "  background: #4d4d4d;"
                        "}"
                        "QScrollBar::handle:vertical:pressed {"
                        "  background: #3a3a3a;"
                        "}"
                        "QScrollBar::add-line:vertical {"
                        "  border: 1px solid #222;"
                        "  background: #444;"
                        "  height: 13px;"
                        "  subcontrol-position: bottom;"
                        "  subcontrol-origin: margin;"
                        "}"
                        "QScrollBar::sub-line:vertical {"
                        "  border: 1px solid #222;"
                        "  background: #444;"
                        "  height: 13px;"
                        "  subcontrol-position: top;"
                        "  subcontrol-origin: margin;"
                        "}"
                        "QScrollBar::add-line:vertical:hover,"
                        "QScrollBar::sub-line:vertical:hover {"
                        "  background: #4d4d4d;"
                        "}"
                        "QScrollBar::add-line:vertical:pressed,"
                        "QScrollBar::sub-line:vertical:pressed {"
                        "  background: #3a3a3a;"
                        "}"

                        "QScrollBar:left-arrow {"
                        "    width: 5px;"
                        "    height: 5px;"
                        "    background: url(:/stella/theme/ScrollLeftArrow.png);"
                        "}"
                        "QScrollBar:right-arrow {"
                        "    width: 5px;"
                        "    height: 5px;"
                        "    background: url(:/stella/theme/ScrollRightArrow.png);"
                        "}"
                        "QScrollBar:up-arrow {"
                        "    width: 5px;"
                        "    height: 5px;"
                        "    background: url(:/stella/theme/ScrollUpArrow.png);"
                        "}"
                        "QScrollBar:down-arrow {"
                        "    width: 5px;"
                        "    height: 5px;"
                        "    background: url(:/stella/theme/ScrollDownArrow.png);"
                        "}"

                        "QProgressBar {"
                        "  background-color: #444; border-image: url(:/stella/theme/TextEdit.png) 2 stretch; border-width: 2px;"
                        "  height: 11px;"
                        "  font-size: 3px;"
                        "}"
                        "QProgressBar::chunk {"
                        "  background-color: #c08010;"
                        "}"
                /*
                        ""
                        "QTreeView::branch{border-image: none; background: rgba(0,0,0,0.01);}"
                        "QTreeView::branch:has-siblings:!adjoins-item{}"
                        "QTreeView::branch:has-siblings:adjoins-item{}"
                        "QTreeView::branch:!has-children:!has-siblings:adjoins-item {}"
                        "QTreeView::branch:has-children:!has-siblings:closed,"
                        "QTreeView::branch:closed:has-children:has-siblings {"
                        "  image: url(:/stella/theme/ScrollRightArrow.png); width: 10px;"
                        "}"
                        "QTreeView::branch:open:has-children:!has-siblings,"
                        "QTreeView::branch:open:has-children:has-siblings {"
                        "  image: url(:/stella/theme/ScrollDownArrow.png); width: 10px;"
                        "}"
    */
                        );
}
#endif

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    qsrand(time(0));

    QCoreApplication::setOrganizationName("Nexhawks.net");
    QCoreApplication::setOrganizationDomain("nexhawks.net");
    QCoreApplication::setApplicationName("Stella Alpha");
    QApplication::setGraphicsSystem("raster");


    static QTranslator qtTranslator;
    qtTranslator.load("qt_" + QLocale::system().name(),
             QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    app.installTranslator(&qtTranslator);

    QTranslator myappTranslator;
    myappTranslator.load("stella_" + QLocale::system().name(),
                         ":/stella/translations");
    app.installTranslator(&myappTranslator);

   // QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));

    QStringList fonts;
    fonts.push_back("Helvetica");
    fonts.push_back("Helvetica Neue");
    fonts.push_back("Nimbus Sans");
    fonts.push_back("FreeSans");
    fonts.push_back("Liberation Sans");
    fonts.push_back("Arial");
    fonts.push_back("Hiragino Kaku Gothic Pro");
    fonts.push_back("IPAexGothic");
    fonts.push_back("IPAGothic");
    fonts.push_back("MS Gothic");
    //fonts.push_back("MS UI Gothic");
    //fonts.push_back("Meiryo");
    QFont::insertSubstitutions("Stella Sans", fonts);

#if USE_STELLA_CSS
    applyStellaCSS();
#endif

    app.setQuitOnLastWindowClosed(true);

    STMainWindow *w=new STMainWindow();

    w->show();

    
    int ret= app.exec();

    delete w;

    return ret;
}
