#-------------------------------------------------
#
# Project created by QtCreator 2012-07-22T18:22:30
#
#-------------------------------------------------

QT       += core gui network opengl script

!macx{
    !win32{
    QT += webkit
    }
}

macx{
    LIBS+= -framework WebKit
    OBJECTIVE_SOURCES += stella/stmacwebview.m
    HEADERS += stella/stmacwebview.h

    #QT+=webkit

}

win32{
    DEFINES += ST_HAVE_ACTIVEQT
    QT+=activeqt
}


CONFIG += rtti exceptions

ICON=stella/stella.icns
RC_FILE=stella/stella.rc

TARGET = Stella
TEMPLATE = app

INCLUDEPATH += kqoauth qjson/include

PRECOMPILED_HEADER = stella/ststdafx.h

QMAKE_RESOURCE_FLAGS += -no-compress # make sure font mmap works

TRANSLATIONS += translations/stella_ja.ts
TRANSLATIONS += translations/stella_en.ts

mac{


    LIBS += -framework Cocoa
}

SOURCES += stella/main.cpp\
        stella/stmainwindow.cpp \
    kqoauth/kqoauthutils.cpp \
    kqoauth/kqoauthrequest.cpp \
    kqoauth/kqoauthrequest_xauth.cpp \
    kqoauth/kqoauthrequest_1.cpp \
    kqoauth/kqoauthmanager.cpp \
    kqoauth/kqoauthauthreplyserver.cpp \
    qjson/src/serializerrunnable.cpp \
    qjson/src/serializer.cpp \
    qjson/src/qobjecthelper.cpp \
    qjson/src/parserrunnable.cpp \
    qjson/src/parser.cpp \
    qjson/src/json_scanner.cpp \
    qjson/src/json_parser.cc \
    stella/stmainview.cpp \
    kqoauth/kqoauthstreamer.cpp \
    stella/stheaderview.cpp \
    stella/ststandardview.cpp \
    stella/stfont.cpp \
    stella/stmath.cpp \
    stella/stnewsview.cpp \
    stella/sttabview.cpp \
    stella/stbuttonview.cpp \
    stella/stbuttongraphicsview.cpp \
    stella/stnewsshadowview.cpp \
    stella/stlineeditview.cpp \
    stella/stcaretview.cpp \
    stella/stsimpleeditview.cpp \
    stella/sttweetview.cpp \
    stella/sttweetnoneditingview.cpp \
    stella/sttweetshadowview.cpp \
    stella/staccounttabview.cpp \
    stella/stbasetabpageview.cpp \
    stella/stpane.cpp \
    stella/stviewanimator.cpp \
    stella/stscrollbarview.cpp \
    stella/stscrolltrackbar.cpp \
    stella/stdynamiclistview.cpp \
    stella/stdynamicanimator.cpp \
    stella/sttestpane.cpp \
    stella/stclipview.cpp \
    stella/sttwittertext.cpp \
    stella/sttweetitemview.cpp \
    stella/stobjectmanager.cpp \
    stella/staccountsmanager.cpp \
    stella/staccount.cpp \
    stella/sttimeline.cpp \
    stella/stuserstreamer.cpp \
    stella/stsimplestreamingtimeline.cpp \
    stella/ststreamer.cpp \
    stella/stsimplestreamer.cpp \
    stella/sttimelineview.cpp \
    stella/sttimelinepane.cpp \
    stella/sttimelineitemview.cpp \
    stella/stusericonmanager.cpp \
    stella/stusericonview.cpp \
    stella/sthometimeline.cpp \
    stella/stmentionstimeline.cpp \
    stella/sttimelinefetcher.cpp \
    stella/sttimelinefetchmoreview.cpp \
    stella/ststripeanimationview.cpp \
    stella/stsingleuserstreamer.cpp \
    stella/stsingleusertimeline.cpp \
    stella/sttweetviewnetwork.cpp \
    stella/sttimelineitemtimeview.cpp \
    stella/stonesectimer.cpp \
    stella/stonesectimermanager.cpp \
    stella/sttimelineitemactionview.cpp \
    stella/stclearbuttonview.cpp \
    stella/stlinkedtextmanager.cpp \
    stella/stsearchtimeline.cpp \
    stella/stfilteredstreamer.cpp \
    stella/stuserpane.cpp \
    stella/stscrolledpane.cpp \
    stella/stuserprofileview.cpp \
    stella/stfavoritetimeline.cpp \
    stella/stnewslistview.cpp \
    stella/stnewslistitemview.cpp \
    stella/stnetworkreplyerrorparser.cpp \
    stella/stactionmanager.cpp \
    stella/stjsonparser.cpp \
    stella/ststaticlistview.cpp \
    stella/stuserlistmodel.cpp \
    stella/stuserlistmodelfetcher.cpp \
    stella/stuserlistmodelloader.cpp \
    stella/stuserlistview.cpp \
    stella/stuserlistitemview.cpp \
    stella/stuserlistfetchmoreview.cpp \
    stella/stuserlistpane.cpp \
    stella/stsimpleanimator.cpp \
    stella/stlinkedlabelview.cpp \
    stella/stuserprofilenumberview.cpp \
    stella/stfollowersuserlistmodel.cpp \
    stella/stfriendsuserlistmodel.cpp \
    stella/staccountsview.cpp \
    stella/stresizeview.cpp \
    stella/staccountitemview.cpp \
    stella/stnewaccountview.cpp \
    stella/stnewaccountstartpane.cpp \
    stella/stobjectnotation.cpp \
    stella/stwebview.cpp \
    stella/stnewaccountauthorizeview.cpp \
    stella/stauthorizecancelbutton.cpp \
    stella/stauthorizeview.cpp \
    stella/ststartupview.cpp \
    stella/stlogoview.cpp \
    stella/ststatuspane.cpp \
    stella/ststatususernameview.cpp \
    stella/ststatuscontentsview.cpp \
    stella/stdateformatter.cpp \
    stella/ststatusmenu.cpp \
    stella/stremoteimagemanager.cpp \
    stella/stremoteimageview.cpp \
    stella/stthumbnailmanager.cpp \
    stella/stzoomedimagemanager.cpp \
    stella/stzoomedimagedownloader.cpp \
    stella/stzoomedimageport.cpp \
    stella/stzoomedimageview.cpp \
    stella/stzoomedimagewindow.cpp \
    stella/stplaceview.cpp \
    stella/ststaticmapimagemanager.cpp

HEADERS  += stella/stmainwindow.h \
    kqoauth/QtKOAuth \
    kqoauth/kqoauthutils.h \
    kqoauth/kqoauthrequest.h \
    kqoauth/kqoauthrequest_xauth.h \
    kqoauth/kqoauthrequest_xauth_p.h \
    kqoauth/kqoauthrequest_p.h \
    kqoauth/kqoauthrequest_1.h \
    kqoauth/kqoauthmanager.h \
    kqoauth/kqoauthmanager_p.h \
    kqoauth/kqoauthglobals.h \
    kqoauth/kqoauthauthreplyserver.h \
    kqoauth/kqoauthauthreplyserver_p.h \
    qjson/include/QJson/Serializer \
    qjson/include/QJson/QObjectHelper \
    qjson/include/QJson/Parser \
    qjson/src/stack.hh \
    qjson/src/serializerrunnable.h \
    qjson/src/serializer.h \
    qjson/src/qobjecthelper.h \
    qjson/src/qjson_export.h \
    qjson/src/qjson_debug.h \
    qjson/src/position.hh \
    qjson/src/parserrunnable.h \
    qjson/src/parser.h \
    qjson/src/parser_p.h \
    qjson/src/location.hh \
    qjson/src/json_scanner.h \
    qjson/src/json_parser.hh \
    stella/stmainview.h \
    kqoauth/kqoauthstreamer.h \
    kqoauth/kqoauthstreamer_p.h \
    stella/stheaderview.h \
    stella/ststandardview.h \
    stella/stfont.h \
    stella/stmath.h \
    stella/stnewsview.h \
    stella/sttabview.h \
    stella/stbuttonview.h \
    stella/stbuttongraphicsview.h \
    stella/stnewsshadowview.h \
    stella/stlineeditview.h \
    stella/stcaretview.h \
    stella/stsimpleeditview.h \
    stella/sttweetview.h \
    stella/sttweetnoneditingview.h \
    stella/sttweetshadowview.h \
    stella/staccounttabview.h \
    stella/stbasetabpageview.h \
    stella/stpane.h \
    stella/stviewanimator.h \
    stella/stscrollbarview.h \
    stella/stscrolltrackbar.h \
    stella/stdynamiclistview.h \
    stella/stdynamicanimator.h \
    stella/sttestpane.h \
    stella/stclipview.h \
    stella/sttwittertext.h \
    stella/sttweetitemview.h \
    stella/stobjectmanager.h \
    stella/staccountsmanager.h \
    stella/staccount.h \
    stella/sttimeline.h \
    stella/stuserstreamer.h \
    stella/stsimplestreamingtimeline.h \
    stella/ststreamer.h \
    stella/stsimplestreamer.h \
    stella/sttimelineview.h \
    stella/sttimelinepane.h \
    stella/sttimelineitemview.h \
    stella/stusericonmanager.h \
    stella/stusericonview.h \
    stella/sthometimeline.h \
    stella/stmentionstimeline.h \
    stella/sttimelinefetcher.h \
    stella/sttimelinefetchmoreview.h \
    stella/ststripeanimationview.h \
    stella/stsingleuserstreamer.h \
    stella/stsingleusertimeline.h \
    stella/sttweetviewnetwork.h \
    stella/sttimelineitemtimeview.h \
    stella/stonesectimer.h \
    stella/stonesectimermanager.h \
    stella/sttimelineitemactionview.h \
    stella/stclearbuttonview.h \
    stella/stlinkedtextmanager.h \
    stella/stsearchtimeline.h \
    stella/stfilteredstreamer.h \
    stella/stuserpane.h \
    stella/stscrolledpane.h \
    stella/stuserprofileview.h \
    stella/stfavoritetimeline.h \
    stella/stnewslistview.h \
    stella/stnewslistitemview.h \
    stella/ststdafx.h \
    stella/stnetworkreplyerrorparser.h \
    stella/stactionmanager.h \
    stella/stjsonparser.h \
    stella/ststaticlistview.h \
    stella/stuserlistmodel.h \
    stella/stuserlistmodelfetcher.h \
    stella/stuserlistmodelloader.h \
    stella/stuserlistview.h \
    stella/stuserlistitemview.h \
    stella/stuserlistfetchmoreview.h \
    stella/stuserlistpane.h \
    stella/stsimpleanimator.h \
    stella/stlinkedlabelview.h \
    stella/stuserprofilenumberview.h \
    stella/stfollowersuserlistmodel.h \
    stella/stfriendsuserlistmodel.h \
    stella/staccountsview.h \
    stella/stresizeview.h \
    stella/staccountitemview.h \
    stella/stnewaccountview.h \
    stella/stnewaccountstartpane.h \
    stella/stobjectnotation.h \
    stella/stwebview.h \
    stella/stwebview_p.h \
    stella/stnewaccountauthorizeview.h \
    stella/stauthorizecancelbutton.h \
    stella/stauthorizeview.h \
    stella/ststartupview.h \
    stella/stlogoview.h \
    stella/ststatuspane.h \
    stella/ststatususernameview.h \
    stella/ststatuscontentsview.h \
    stella/stdateformatter.h \
    stella/ststatusmenu.h \
    stella/stremoteimagemanager.h \
    stella/stremoteimageview.h \
    stella/stthumbnailmanager.h \
    stella/stzoomedimagemanager.h \
    stella/stzoomedimagedownloader.h \
    stella/stzoomedimageport.h \
    stella/stzoomedimageview.h \
    stella/stzoomedimagewindow.h \
    stella/stplaceview.h \
    stella/ststaticmapimagemanager.h

FORMS    += stella/stmainwindow.ui \
    stella/stzoomedimagewindow.ui

RESOURCES += \
    stella/stella.qrc \
    stella/stellasans11.qrc \
    stella/stellasansbold11.qrc \
    translations/translations.qrc


OTHER_FILES += \
    stella/res/twitter-text.js \
    stella/res/sample2.json
