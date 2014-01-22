INCLUDEPATH += src/common \
               src/helper \
               src/map \
               src/world

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
            $$PWD/ui/waterwidget.cpp \
    src/ui/waterwidget.cpp

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
    src/ui/waterwidget.h

FORMS += $$PWD/mainwindow.ui \
         $$PWD/ui/teleportwidget.ui \
         $$PWD/ui/mapview_settings.ui \
         $$PWD/ui/about.ui \
         $$PWD/ui/texturepicker.ui \
         $$PWD/ui/startup.ui \
         $$PWD/ui/project_settings.ui \
         $$PWD/ui/waterwidget.ui