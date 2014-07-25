#This file is part of QEditor.

#QEditor is free software: you can redistribute it and/or modify
#it under the terms of the GNU General Public License as published by
#the Free Software Foundation, either version 3 of the License, or
#(at your option) any later version.

#QEditor is distributed in the hope that it will be useful,
#but WITHOUT ANY WARRANTY; without even the implied warranty of
#MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#GNU General Public License for more details.

#You should have received a copy of the GNU General Public License
#along with QEditor.  If not, see <http://www.gnu.org/licenses/>.

include(src/src.pri)

QT += core gui widgets opengl network xml

CONFIG += c++11 network

#win32:CONFIG(release, debug|release): QTPLUGIN += QWindowsIntegrationPlugin

#CONFIG += console

#DEFINES -= QT_NO_WARNING_OUTPUT QT_NO_DEBUG_OUTPUT

CONFIG(debug, debug|release)   { DEFINES += DEBUG_MODE }
CONFIG(release, debug|release) { DEFINES += RELEASE_MODE }

install_it.path = $$OUT_PWD
install_it.files += $$PWD/textures/grass.png
install_it.files += $$PWD/textures/rock.png
install_it.files += $$PWD/textures/snowrocks.png
install_it.files += $$PWD/textures/water.png

INSTALLS += install_it

TEMPLATE = app

INCLUDEPATH += src

RESOURCES += qeditor.qrc

RC_FILE = resources.rc

TRANSLATIONS += qeditor_cz.ts

OTHER_FILES += info.txt \
    data/shaders/qeditor.frag \
    data/shaders/qeditor.geom \
    data/shaders/qeditor.tcs \
    data/shaders/qeditor.tes \
    data/shaders/qeditor.vert \
    data/shaders/qeditor_water.frag \
    data/shaders/model.frag \
    data/shaders/model.vert \
    data/shaders/qeditor_bezier.vert \
    data/shaders/qeditor_bezier.geom \
    data/shaders/qeditor_bezier.frag \
    data/shaders/qeditor_water.tes \
    data/shaders/qeditor_skybox.vert \
    data/shaders/qeditor_skybox.frag \
    resources.rc

unix|win32: LIBS += -L$$PWD/ -lquazip

INCLUDEPATH += $$PWD/dep\include
LIBS += $$PWD/dep\lib\assimp_release-dll_x64\assimp.lib

INCLUDEPATH += $$PWD/
DEPENDPATH += $$PWD/

win32: PRE_TARGETDEPS += $$PWD/quazip.lib
# else:unix: PRE_TARGETDEPS += $$PWD/libquazip.a

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/ -lQt5PlatformSupport
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/ -lQt5PlatformSupportd
else:unix: LIBS += -L$$PWD/ -lQt5PlatformSupport

INCLUDEPATH += $$PWD/
DEPENDPATH += $$PWD/

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/libQt5PlatformSupport.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/libQt5PlatformSupportd.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/Qt5PlatformSupport.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/Qt5PlatformSupportd.lib
else:unix: PRE_TARGETDEPS += $$PWD/libQt5PlatformSupport.a