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

INCLUDEPATH += src/common \
               src/helper \
               src/map \
               src/map/generation \
               src/model \
               src/world \
               src/3rd-party/imageshackAPI \
               src/3rd-party/hmstl \
               src/3rd-party/libtrix

SOURCES  += $$PWD/main.cpp \
            $$PWD/mainwindow.cpp \
            $$PWD/ui/teleportwidget.cpp \
            $$PWD/ui/qdslider.cpp \
            $$PWD/ui/mapview_settings.cpp \
            $$PWD/brush.cpp \
            $$PWD/mapview.cpp \
            $$PWD/ui/about.cpp \
            $$PWD/ui/texturepicker.cpp \
            $$PWD/ui/tableview.cpp \
            $$PWD/qeditor.cpp \
            $$PWD/ui/startup.cpp \
            $$PWD/ui/newproject.cpp \
            $$PWD/ui/project_settings.cpp \
            $$PWD/common/material.cpp \
            $$PWD/common/texture.cpp \
            $$PWD/common/sampler.cpp \
            $$PWD/common/camera.cpp \
            $$PWD/common/texturearray.cpp \
            $$PWD/common/texturemanager.cpp \
            $$PWD/world/world.cpp \
            $$PWD/map/mapchunk.cpp \
            $$PWD/map/maptile.cpp \
            $$PWD/helper/mathhelper.cpp \
            $$PWD/map/watertile.cpp \
            $$PWD/map/waterchunk.cpp \
            $$PWD/model/ianimation.cpp \
            $$PWD/model/ibone.cpp \
            $$PWD/model/ilight.cpp \
            $$PWD/model/imesh.cpp \
            $$PWD/model/imodel.cpp \
            $$PWD/model/imodelinterface.cpp \
            $$PWD/model/imodelmanager.cpp \
            $$PWD/model/ipipeline.cpp \
            $$PWD/model/itexturemanager.cpp \
            $$PWD/ui/modelpicker.cpp \
            $$PWD/ui/toolbox.cpp \
            $$PWD/ui/waterwidget.cpp \
            $$PWD/common/framebuffer.cpp \
            $$PWD/3rd-party/imgurAPI/fileupload.cpp \
            $$PWD/ui/camerawidget.cpp \
            $$PWD/beziercurve.cpp \
            $$PWD/skybox.cpp \
            $$PWD/map/generation/fractalgeneration.cpp \
            $$PWD/map/generation/perlingenerator.cpp \
            $$PWD/ui/mapgeneration.cpp \
            $$PWD/ui/heightmapwidget.cpp \
            $$PWD/map/mapcleft.cpp \
            $$PWD/ui/mapchunk_settings.cpp \
    src/ui/basic_settings.cpp \
    src/stlexport.cpp
            #$$PWD/3rd-party/libtrix/libtrix.cpp \
            #$$PWD/3rd-party/hmstl/stb_image.cpp \
            #$$PWD/3rd-party/hmstl/heightmap.cpp
            #$$PWD/3rd-party/hmstl/htmstl.cpp

HEADERS  += $$PWD/mainwindow.h \
            $$PWD/ui/teleportwidget.h \
            $$PWD/ui/qdslider.h \
            $$PWD/ui/mapview_settings.h \
            $$PWD/brush.h \
            $$PWD/mapview.h \
            $$PWD/ui/about.h \
            $$PWD/ui/texturepicker.h \
            $$PWD/ui/tableview.h \
            $$PWD/qeditor.h \
            $$PWD/ui/startup.h \
            $$PWD/ui/newproject.h \
            $$PWD/ui/project_settings.h \
            $$PWD/common/material.h \
            $$PWD/common/texture.h \
            $$PWD/common/sampler.h \
            $$PWD/common/camera.h \
            $$PWD/common/camera_p.h \
            $$PWD/common/texturearray.h \
            $$PWD/common/texturemanager.h \
            $$PWD/world/world.h \
            $$PWD/map/mapheaders.h \
            $$PWD/map/mapchunk.h \
            $$PWD/map/maptile.h \
            $$PWD/helper/globalheader.h \
            $$PWD/helper/mathhelper.h \
            $$PWD/map/watertile.h \
            $$PWD/map/waterchunk.h \
            $$PWD/ui/waterwidget.h \
            $$PWD/model/ianimation.h \
            $$PWD/model/ibone.h \
            $$PWD/model/ilight.h \
            $$PWD/model/imesh.h \
            $$PWD/model/imodel.h \
            $$PWD/model/imodelinterface.h \
            $$PWD/model/imodelmanager.h \
            $$PWD/model/ipipeline.h \
            $$PWD/model/itexturemanager.h \
            $$PWD/ui/modelpicker.h \
            $$PWD/ui/toolbox.h \
            $$PWD/ui/waterwidget.h \
            $$PWD/common/framebuffer.h \
            $$PWD/3rd-party/imgurAPI/fileupload.h \
            $$PWD/ui/camerawidget.h \
            $$PWD/beziercurve.h \
            $$PWD/skybox.h \
            $$PWD/version.h \
            $$PWD/map/generation/fractalgeneration.h \
            $$PWD/map/generation/perlingenerator.h \
            $$PWD/ui/mapgeneration.h \
            $$PWD/ui/heightmapwidget.h \
            $$PWD/map/mapcleft.h \
            $$PWD/ui/mapchunk_settings.h \
    src/ui/basic_settings.h \
    src/stlexport.h
            #$$PWD/3rd-party/libtrix/libtrix.h \
            #$$PWD/3rd-party/hmstl/stb_image.h \
            #$$PWD/3rd-party/hmstl/heightmap.h \
            #$$PWD/3rd-party/hmstl/htmstl.h

FORMS    += $$PWD/mainwindow.ui \
            $$PWD/ui/teleportwidget.ui \
            $$PWD/ui/mapview_settings.ui \
            $$PWD/ui/about.ui \
            $$PWD/ui/texturepicker.ui \
            $$PWD/ui/startup.ui \
            $$PWD/ui/project_settings.ui \
            $$PWD/ui/waterwidget.ui \
            $$PWD/ui/modelpicker.ui \
            $$PWD/ui/camerawidget.ui \
            $$PWD/ui/mapgeneration.ui \
            $$PWD/ui/heightmapwidget.ui \
            $$PWD/ui/mapchunk_settings.ui \
    src/ui/basic_settings.ui