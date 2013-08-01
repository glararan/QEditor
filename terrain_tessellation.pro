include( common/common.pri )

QT += core gui widgets

#CONFIG += console

#DEFINES -= QT_NO_WARNING_OUTPUT QT_NO_DEBUG_OUTPUT

install_it.path = $$OUT_PWD
install_it.files += $$PWD/heightmap-1024x1024.png
install_it.files += $$PWD/grass.png
install_it.files += $$PWD/rock.png
install_it.files += $$PWD/snowrocks.png

INSTALLS += install_it

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
               shaders/terraintessellation.tes \
    info.txt

FORMS += \
    mainwindow.ui
