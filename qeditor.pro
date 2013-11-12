include(common/common.pri)

QT += core gui widgets

#CONFIG += console

#DEFINES -= QT_NO_WARNING_OUTPUT QT_NO_DEBUG_OUTPUT

install_it.path = $$OUT_PWD
install_it.files += $$PWD/grass.png
install_it.files += $$PWD/rock.png
install_it.files += $$PWD/snowrocks.png

INSTALLS += install_it

TEMPLATE = app

INCLUDEPATH += common

SOURCES  += main.cpp \
    mainwindow.cpp \
    ui/teleportwidget.cpp \
    ui/qdslider.cpp \
    ui/mapview_settings.cpp \
    world.cpp \
    maptile.cpp \
    mapchunk.cpp \
    mapview.cpp \
    mathhelper.cpp \
    brush.cpp \
    ui/about.cpp \
    ui/texturepicker.cpp \
    ui/tableview.cpp \
    qeditor.cpp \
    ui/startup.cpp \
    ui/newproject.cpp

HEADERS  += \
    mainwindow.h \
    ui/teleportwidget.h \
    ui/qdslider.h \
    ui/mapview_settings.h \
    world.h \
    maptile.h \
    mapchunk.h \
    mapheaders.h \
    mapview.h \
    mathhelper.h \
    brush.h \
    ui/about.h \
    ui/texturepicker.h \
    ui/tableview.h \
    qeditor.h \
    ui/startup.h \
    ui/newproject.h \
    globalheader.h

RESOURCES += \
    qeditor.qrc

OTHER_FILES += \
    info.txt \
    shaders/qeditor.frag \
    shaders/qeditor.geom \
    shaders/qeditor.tcs \
    shaders/qeditor.tes \
    shaders/qeditor.vert

FORMS += \
    mainwindow.ui \
    ui/teleportwidget.ui \
    ui/mapview_settings.ui \
    ui/about.ui \
    ui/texturepicker.ui \
    ui/startup.ui

unix|win32: LIBS += -L$$PWD/ -lquazip

INCLUDEPATH += $$PWD/
DEPENDPATH += $$PWD/

win32: PRE_TARGETDEPS += $$PWD/quazip.lib
# else:unix: PRE_TARGETDEPS += $$PWD/libquazip.a