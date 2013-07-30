include( common/common.pri )

QT += core gui widgets

#CONFIG += console

#DEFINES -= QT_NO_WARNING_OUTPUT QT_NO_DEBUG_OUTPUT

TEMPLATE = app

INCLUDEPATH += common

SOURCES  += main.cpp \
            terraintessellationscene.cpp \
    mainwindow.cpp

HEADERS  += \
            terraintessellationscene.h \
    mainwindow.h

RESOURCES += terrain_tessellation.qrc

OTHER_FILES += shaders/terraintessellation.vert \
               shaders/terraintessellation.geom \
               shaders/terraintessellation.frag \
               shaders/terraintessellation.tcs \
               shaders/terraintessellation.tes

FORMS += \
    mainwindow.ui
