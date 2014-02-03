include(src/src.pri)

QT += core gui widgets opengl network xml

CONFIG += c++11 network

#CONFIG += console

#DEFINES -= QT_NO_WARNING_OUTPUT QT_NO_DEBUG_OUTPUT

install_it.path = $$OUT_PWD
install_it.files += $$PWD/textures/grass.png
install_it.files += $$PWD/textures/rock.png
install_it.files += $$PWD/textures/snowrocks.png
install_it.files += $$PWD/textures/water.png

INSTALLS += install_it

TEMPLATE = app

INCLUDEPATH += src

RESOURCES += qeditor.qrc

OTHER_FILES += info.txt \
    data/shaders/qeditor.frag \
    data/shaders/qeditor.geom \
    data/shaders/qeditor.tcs \
    data/shaders/qeditor.tes \
    data/shaders/qeditor.vert \
    data/shaders/qeditor_world.frag \
    data/shaders/qeditor_world.vert \
    data/shaders/qeditor_water.frag \
    data/shaders/model.frag \
    data/shaders/model.vert

unix|win32: LIBS += -L$$PWD/ -lquazip

INCLUDEPATH += $$PWD/dep\include
LIBS += $$PWD/dep\lib\assimp_release-dll_x64\assimp.lib

INCLUDEPATH += $$PWD/
DEPENDPATH += $$PWD/

win32: PRE_TARGETDEPS += $$PWD/quazip.lib
# else:unix: PRE_TARGETDEPS += $$PWD/libquazip.a