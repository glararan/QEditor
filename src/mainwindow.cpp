/*This file is part of QEditor.

QEditor is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

QEditor is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with QEditor.  If not, see <http://www.gnu.org/licenses/>.*/

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "ui/about.h"
#include "ui/project_settings.h"
#include "ui/mapgeneration.h"

#include "3rd-party/imgurAPI/fileupload.h"

#include "qeditor.h"

#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include <QDesktopServices>
#include <QClipboard>

#include <limits>

MainWindow::MainWindow(QWidget* parent)
: QMainWindow(parent)
, ui(new Ui::MainWindow)
, DisplayMode("action_Default")
, ToolBarItem("action_mapview_m0")
, teleportW(NULL)
, settingsW(NULL)
, texturepW(NULL)
, modelpickerW(NULL)
, heightmapW(NULL)
, colorW(NULL)
, waterW(NULL)
, cameraW(NULL)
, t_outer_radius(NULL)
, t_inner_radius(NULL)
, t_speed(NULL)
, t_flow(NULL)
, t_brush(NULL)
, t_brush_circle(NULL)
, t_brush_square(NULL)
, t_reset_transform(NULL)
, t_terrain_mode(NULL)
, t_brush_type(NULL)
, t_brush_mode(NULL)
, t_terrain_maxHeight(NULL)
, t_paint_maxAlpha(NULL)
, t_terrain_maximum_height(NULL)
, t_paint_maximum_alpha(NULL)
, t_modelmode(NULL)
, t_modelmode_insert(NULL)
, t_modelmode_delete(NULL)
, t_rotationx(NULL)
, t_rotationy(NULL)
, t_rotationz(NULL)
, t_impend(NULL)
, t_scale(NULL)
, t_terrain_mode_label(NULL)
, t_brush_label(NULL)
, t_brush_type_label(NULL)
, t_brush_mode_label(NULL)
, t_outer_radius_label(NULL)
, t_outer_radius_value_label(NULL)
, t_inner_radius_label(NULL)
, t_inner_radius_value_label(NULL)
, t_speed_label(NULL)
, t_speed_value_label(NULL)
, t_flow_label(NULL)
, t_flow_value_label(NULL)
, t_model_mode_label(NULL)
, t_rotationx_label(NULL)
, t_rotationx_value_label(NULL)
, t_rotationy_label(NULL)
, t_rotationy_value_label(NULL)
, t_rotationz_label(NULL)
, t_rotationz_value_label(NULL)
, t_impend_label(NULL)
, t_impend_value_label(NULL)
, t_scale_label(NULL)
, t_scale_value_label(NULL)
, world(NULL)
, mapView(NULL)
, mapCoords(NULL)
{
    ui->setupUi(this);

    // Initialize startup widget
    startUp = new StartUp();

    setCentralWidget(startUp);

    connect(startUp, SIGNAL(createMemoryProject(NewProjectData)), this, SLOT(createMemoryProject(NewProjectData)));
    connect(startUp, SIGNAL(openProject(ProjectFileData)),        this, SLOT(openWorld(ProjectFileData)));

    /// menu bar
    ui->menu_Project->menuAction()->setVisible(false);
    ui->menu_Tools->menuAction()->setVisible(false);

    ui->action_Show_Chunk_lines->setChecked(app().getSetting("chunkLines", false).toBool());

    // file
    connect(ui->action_New,  SIGNAL(triggered()), startUp, SLOT(showNewProject()));
    connect(ui->action_Load, SIGNAL(triggered()), startUp, SLOT(showOpenProject()));
    connect(ui->action_Exit, SIGNAL(triggered()), this,    SLOT(close()));

    // help - about
    connect(ui->action_About, SIGNAL(triggered()), this, SLOT(showAbout()));

    // tools - screenshot, fullscreen
    connect(ui->action_Screenshot,        SIGNAL(triggered()),     this, SLOT(takeScreenshot()));
    connect(ui->action_Screenshot_upload, SIGNAL(triggered()),     this, SLOT(takeScreenshotAndUpload()));
    connect(ui->action_Fullscreen,        SIGNAL(triggered(bool)), this, SLOT(setFullscreen(bool)));
}

MainWindow::~MainWindow()
{
    delete ui;

    delete teleportW;
    delete settingsW;
    delete texturepW;
    delete modelpickerW;
    delete colorW;
    delete waterW;
    delete cameraW;
    delete heightmapW;

    deleteObject(t_outer_radius);
    deleteObject(t_inner_radius);
    deleteObject(t_speed);
    deleteObject(t_flow);

    deleteObject(t_modelmode);
    deleteObject(t_modelmode_insert);
    deleteObject(t_modelmode_delete);
    deleteObject(t_rotationx);
    deleteObject(t_rotationy);
    deleteObject(t_rotationz);
    deleteObject(t_impend);
    deleteObject(t_scale);

    deleteObject(t_brush);

    deleteObject(t_brush_circle);
    deleteObject(t_brush_square);

    deleteObject(t_reset_transform);

    deleteObject(t_terrain_mode);
    deleteObject(t_brush_type);
    deleteObject(t_brush_mode);

    deleteObject(t_terrain_maxHeight);
    deleteObject(t_terrain_maximum_height);

    deleteObject(t_paint_maxAlpha);
    deleteObject(t_paint_maximum_alpha);

    deleteObject(t_terrain_mode_label);

    deleteObject(t_brush_label);
    deleteObject(t_brush_type_label);
    deleteObject(t_brush_mode_label);

    deleteObject(t_outer_radius_label);
    deleteObject(t_outer_radius_value_label);
    deleteObject(t_inner_radius_label);
    deleteObject(t_inner_radius_value_label);
    deleteObject(t_speed_label);
    deleteObject(t_speed_value_label);
    deleteObject(t_flow_label);
    deleteObject(t_flow_value_label);
    deleteObject(t_model_mode_label);
    deleteObject(t_rotationx_label);
    deleteObject(t_rotationx_value_label);
    deleteObject(t_rotationy_label);
    deleteObject(t_rotationy_value_label);
    deleteObject(t_rotationz_label);
    deleteObject(t_rotationz_value_label);
    deleteObject(t_impend_label);
    deleteObject(t_impend_value_label);
    deleteObject(t_scale_label);
    deleteObject(t_scale_value_label);

    qDebug() << tr("UI in MainWindow was destroyed!");

    deleteObject(startUp);

    if(world)
        world->deleteMe();

    deleteObject(mapView);

    if(mapCoords)
    {
        for(int i = 0; i < TILES; ++i)
            delete[] mapCoords[i];

        delete[] mapCoords;
    }

    qDebug() << tr("MainWindow was destroyed!");
}

void MainWindow::openWorld(ProjectFileData projectData)
{
    // world constructor
    world = new World(projectData);

    // map view constructor
    mapView = new MapView(world, this);

    setCentralWidget(mapView);

    delete startUp;

    startUp = NULL;

    setCursor(Qt::ArrowCursor);

    ui->menu_Project->menuAction()->setVisible(true);
    ui->menu_Tools->menuAction()->setVisible(true);

    // menu dialogs constructors
    teleportW    = new TeleportWidget();
    settingsW    = new MapView_Settings();
    texturepW    = new TexturePicker();
    modelpickerW = new ModelPicker();
    waterW       = new WaterWidget();
    cameraW      = new CameraWidget(world->getCamera());
    heightmapW   = new HeightmapWidget();

    // post initialize world sub widgets
    connect(mapView, SIGNAL(initialized()), this, SLOT(postInitializeSubWorldWidgets()));

    // init modes
    initMode();

    /// map view
    connect(mapView, SIGNAL(statusBar(QString)),              ui->statusbar, SLOT(showMessage(QString)));
    connect(mapView, SIGNAL(updateBrushOuterRadius(double)),  this,          SLOT(setBrushOuterRadius(double)));
    connect(mapView, SIGNAL(updateBrushInnerRadius(double)),  this,          SLOT(setBrushInnerRadius(double)));
    connect(mapView, SIGNAL(selectedMapChunk(MapChunk*)),     texturepW,     SLOT(setChunk(MapChunk*)));
    connect(mapView, SIGNAL(selectedWaterChunk(WaterChunk*)), waterW,        SLOT(setChunk(WaterChunk*)));

    connect(mapView, SIGNAL(eMModeChanged(MapView::eMouseMode&, MapView::eEditingMode&)),
            this,    SLOT(setBrushMode(MapView::eMouseMode&, MapView::eEditingMode&)));

    /// menu bar
    // file
    connect(ui->action_New,           SIGNAL(triggered()), this,    SLOT(newProject()));
    connect(ui->action_Save,          SIGNAL(triggered()), mapView, SLOT(save()));
    connect(ui->action_Close_Project, SIGNAL(triggered()), this,    SLOT(closeProject())); // not implemented memory cleaning after objects destroy but after loading new project don't make mem leaks.

    // project - settings
    connect(ui->action_Project_Settings, SIGNAL(triggered()), this, SLOT(showProjectSettings()));

    // tools - speed multiplier
    connect(ui->action_Speed_0  , SIGNAL(triggered()), this, SLOT(setSpeedMultiplier()));
    connect(ui->action_Speed_1  , SIGNAL(triggered()), this, SLOT(setSpeedMultiplier()));
    connect(ui->action_Speed_3  , SIGNAL(triggered()), this, SLOT(setSpeedMultiplier()));
    connect(ui->action_Speed_5  , SIGNAL(triggered()), this, SLOT(setSpeedMultiplier()));
    connect(ui->action_Speed_7  , SIGNAL(triggered()), this, SLOT(setSpeedMultiplier()));
    connect(ui->action_Speed_10 , SIGNAL(triggered()), this, SLOT(setSpeedMultiplier()));
    connect(ui->action_Speed_0_1, SIGNAL(triggered()), this, SLOT(setSpeedMultiplier()));
    connect(ui->action_Speed_0_2, SIGNAL(triggered()), this, SLOT(setSpeedMultiplier()));

    connect(this, SIGNAL(setSpeedMultiplier(float)), mapView, SLOT(setSpeedMultiplier(float)));

    // tools - display mode
    connect(ui->action_Default           , SIGNAL(triggered()), this, SLOT(setDisplayMode()));
    connect(ui->action_Terrain_Wireframe , SIGNAL(triggered()), this, SLOT(setDisplayMode()));
    connect(ui->action_Wireframe         , SIGNAL(triggered()), this, SLOT(setDisplayMode()));
    connect(ui->action_BaseLayer         , SIGNAL(triggered()), this, SLOT(setDisplayMode()));
    connect(ui->action_Base_Layer1       , SIGNAL(triggered()), this, SLOT(setDisplayMode()));
    connect(ui->action_Base_Layer1_Layer2, SIGNAL(triggered()), this, SLOT(setDisplayMode()));
    connect(ui->action_Wireframe_Height  , SIGNAL(triggered()), this, SLOT(setDisplayMode()));
    connect(ui->action_Colored           , SIGNAL(triggered()), this, SLOT(setDisplayMode()));
    connect(ui->action_Light_and_Shadow  , SIGNAL(triggered()), this, SLOT(setDisplayMode()));
    connect(ui->action_Hidden            , SIGNAL(triggered()), this, SLOT(setDisplayMode()));

    connect(this, SIGNAL(setDisplayMode(int)), mapView, SLOT(setDisplayMode(int)));

    // tools - show chunk lines
    connect(ui->action_Show_Chunk_lines, SIGNAL(toggled(bool)), mapView, SLOT(setTurnChunkLines(bool)));

    // tools - texture picker, settings, teleport, map generator, heightmap, reset camera, lock camera
    connect(ui->action_Texture_Picker, SIGNAL(triggered()),     this,    SLOT(showTexturePicker()));
    connect(ui->action_Settings,       SIGNAL(triggered()),     this,    SLOT(showSettings()));
    connect(ui->action_Teleport,       SIGNAL(triggered()),     this,    SLOT(showTeleport()));
    connect(ui->action_Map_Generator,  SIGNAL(triggered()),     this,    SLOT(showMapGeneration()));
    connect(ui->action_Heightmap,      SIGNAL(triggered()),     this,    SLOT(showHeightmap()));
    connect(ui->action_Reset_camera,   SIGNAL(triggered()),     mapView, SLOT(resetCamera()));
    connect(ui->action_Lock_camera,    SIGNAL(triggered(bool)), mapView, SLOT(lockCamera(bool)));

    connect(teleportW, SIGNAL(TeleportTo(QVector3D*)), mapView, SLOT(setCameraPosition(QVector3D*)));

    connect(settingsW, SIGNAL(setColorOfBrush(QColor*, bool)),     mapView, SLOT(setBrushColor(QColor*, bool)));
    connect(settingsW, SIGNAL(setColorOfWireframe(QColor*, bool)), mapView, SLOT(setWireframeColor(QColor*, bool)));
    connect(settingsW, SIGNAL(setEnvironmentDistance(float)),      mapView, SLOT(setEnvionmentDistance(float)));
    connect(settingsW, SIGNAL(setTextureScaleOption(int)),         mapView, SLOT(setTextureScaleOption_(int)));
    connect(settingsW, SIGNAL(setTextureScaleFar(float)),          mapView, SLOT(setTextureScaleFar(float)));
    connect(settingsW, SIGNAL(setTextureScaleNear(float)),         mapView, SLOT(setTextureScaleNear(float)));
    connect(settingsW, SIGNAL(setTabletMode(bool)),                mapView, SLOT(setTabletMode(bool)));

    connect(heightmapW, SIGNAL(accepted()),                this, SLOT(heightmapWidgetAccepted()));
    connect(heightmapW, SIGNAL(rejected()),                this, SLOT(heightmapWidgetRejected()));
    connect(heightmapW, SIGNAL(setScale(float)),           this, SLOT(setHeightmapScale(float)));
    connect(heightmapW, SIGNAL(importing(QString, float)), this, SLOT(importingHeightmap(QString, float)));
    connect(heightmapW, SIGNAL(exporting(QString, float)), this, SLOT(exportingHeightmap(QString, float)));

    // tools - test, stereoscopic
    connect(ui->action_3D_Stereoscopic, SIGNAL(triggered(bool)), mapView, SLOT(set3DStreoscopic(bool)));
    connect(ui->action_Test, SIGNAL(triggered()), mapView, SLOT(doTest()));

    // tools - development
    connect(ui->action_Undo,          SIGNAL(triggered()), this, SLOT(actionIsInDevelopment()));
    connect(ui->action_Redo,          SIGNAL(triggered()), this, SLOT(actionIsInDevelopment()));
    //connect(ui->action_New,           SIGNAL(triggered()), this, SLOT(actionIsInDevelopment()));
    connect(ui->action_Load,          SIGNAL(triggered()), this, SLOT(actionIsInDevelopment()));
    //connect(ui->action_Close_Project, SIGNAL(triggered()), this, SLOT(actionIsInDevelopment()));
    connect(ui->action_Backup_Maps,   SIGNAL(triggered()), this, SLOT(actionIsInDevelopment()));
    connect(ui->action_Project_Maps,  SIGNAL(triggered()), this, SLOT(actionIsInDevelopment()));

    /// toolbar
    connect(ui->action_mapview_m0, SIGNAL(triggered()), this, SLOT(setToolBarItem()));
    connect(ui->action_mapview_m1, SIGNAL(triggered()), this, SLOT(setToolBarItem()));
    connect(ui->action_mapview_m2, SIGNAL(triggered()), this, SLOT(setToolBarItem()));
    connect(ui->action_mapview_m3, SIGNAL(triggered()), this, SLOT(setToolBarItem()));
    connect(ui->action_mapview_m4, SIGNAL(triggered()), this, SLOT(setToolBarItem()));
    connect(ui->action_mapview_m5, SIGNAL(triggered()), this, SLOT(setToolBarItem()));
    connect(ui->action_mapview_m6, SIGNAL(triggered()), this, SLOT(setToolBarItem()));

    connect(this, SIGNAL(setModeEditing(int)), mapView, SLOT(setModeEditing(int)));

    // toolbar 2
    connect(ui->action_Switch_Vertex_Shading, SIGNAL(toggled(bool)), this, SLOT(setVertexShadingSwitch(bool)));
    connect(ui->action_Switch_Skybox,         SIGNAL(toggled(bool)), this, SLOT(setSkyboxSwitch(bool)));

    /// toolbar3
    connect(t_terrain_mode, SIGNAL(currentIndexChanged(int)), this   , SLOT(setTerrain_Mode(int)));
    connect(this,           SIGNAL(setTerrainMode(int))     , mapView, SLOT(setTerrainMode(int)));

    connect(t_brush,        SIGNAL(buttonToggled(int, bool)), this,    SLOT(setBrush(int, bool)));
    connect(t_brush_type  , SIGNAL(currentIndexChanged(int)), mapView, SLOT(setBrushType(int)));
    connect(t_brush_type,   SIGNAL(currentIndexChanged(int)), this,    SLOT(setBrushType(int)));
    connect(t_brush_mode,   SIGNAL(currentIndexChanged(int)), mapView, SLOT(setBrushMode(int)));

    connect(t_modelmode, SIGNAL(buttonToggled(int, bool)), this, SLOT(setModel_Mode(int, bool)));

    connect(t_reset_transform, SIGNAL(clicked()), this, SLOT(resetModelBrush()));

    connect(t_terrain_maxHeight,      SIGNAL(stateChanged(int)),    this,    SLOT(setTerrainMaximumHeightState(int)));
    connect(t_terrain_maximum_height, SIGNAL(valueChanged(double)), mapView, SLOT(setTerrainMaximumHeight(double)));

    connect(t_paint_maxAlpha,      SIGNAL(stateChanged(int)),    this,    SLOT(setPaintMaximumAlphaState(int)));
    connect(t_paint_maximum_alpha, SIGNAL(valueChanged(double)), mapView, SLOT(setPaintMaximumAlpha(double)));

    connect(t_outer_radius, SIGNAL(valueChanged(double)), t_outer_radius_value_label, SLOT(setNum(double)));
    connect(t_inner_radius, SIGNAL(valueChanged(double)), t_inner_radius_value_label, SLOT(setNum(double)));
    connect(t_speed,        SIGNAL(valueChanged(double)), t_speed_value_label,        SLOT(setNum(double)));
    connect(t_flow,         SIGNAL(valueChanged(double)), t_flow_value_label,         SLOT(setNum(double)));

    connect(t_rotationx,    SIGNAL(valueChanged(double)), t_rotationx_value_label,    SLOT(setNum(double)));
    connect(t_rotationy,    SIGNAL(valueChanged(double)), t_rotationy_value_label,    SLOT(setNum(double)));
    connect(t_rotationz,    SIGNAL(valueChanged(double)), t_rotationz_value_label,    SLOT(setNum(double)));
    connect(t_impend,       SIGNAL(valueChanged(double)), t_impend_value_label,       SLOT(setNum(double)));
    connect(t_scale,        SIGNAL(valueChanged(double)), t_scale_value_label,        SLOT(setNum(double)));

    connect(t_outer_radius, SIGNAL(valueChanged(double)), mapView, SLOT(setBrushOuterRadius(double)));
    connect(t_outer_radius, SIGNAL(valueChanged(double)), this,    SLOT(setBrushInnerRadiusMaximumValue(double)));
    connect(t_inner_radius, SIGNAL(valueChanged(double)), mapView, SLOT(setBrushInnerRadius(double)));
    connect(t_speed,        SIGNAL(valueChanged(double)), mapView, SLOT(setBrushSpeed(double)));
    connect(t_flow,         SIGNAL(valueChanged(double)), mapView, SLOT(setTexturingFlow(double)));

    connect(t_rotationx,    SIGNAL(valueChanged(double)), mapView, SLOT(setModelRotationX(double)));
    connect(t_rotationy,    SIGNAL(valueChanged(double)), mapView, SLOT(setModelRotationY(double)));
    connect(t_rotationz,    SIGNAL(valueChanged(double)), mapView, SLOT(setModelRotationZ(double)));
    connect(t_impend,       SIGNAL(valueChanged(double)), mapView, SLOT(setModelImpend(double)));
    connect(t_scale,        SIGNAL(valueChanged(double)), mapView, SLOT(setModelScale(double)));

    connect(colorW, SIGNAL(currentColorChanged(QColor)), mapView, SLOT(setVertexShading(QColor)));

    connect(cameraW, SIGNAL(showPath(bool)), mapView, SLOT(setCameraShowCurve(bool)));
}

void MainWindow::postInitializeSubWorldWidgets()
{
    texturepW->initialize(world->getTextureManager());
    modelpickerW->loadPicker(world->getModelManager());
}

void MainWindow::createMemoryProject(NewProjectData projectData)
{
    ProjectFileData projectFile;
    projectFile.projectFile    = projectData.projectFile;
    projectFile.projectRootDir = projectData.projectDir;
    projectFile.projectName    = projectData.projectName;
    projectFile.mapName        = projectData.mapName;
    projectFile.mapsCount      = 0;

    mapCoords = projectData.mapCoords;

    for(int x = 0; x < TILES; ++x)
    {
        for(int y = 0; y < TILES; ++y)
        {
            int index = y * TILES + x;

            projectFile.maps[index].exists = 0;

            projectFile.maps[index].x = -1;
            projectFile.maps[index].y = -1;
        }
    }

    qDebug() << projectData.projectFile;
    qDebug() << projectData.projectDir;
    qDebug() << projectData.projectName;
    qDebug() << projectData.mapName;

    if(world && mapView) // world is already open, close it and create proj.
        closeProject();

    openWorld(projectFile);

    connect(mapView, SIGNAL(initialized()), this, SLOT(loadNewProjectMapTilesIntoMemory()));
}

void MainWindow::loadNewProjectMapTilesIntoMemory()
{
    world->loadNewProjectMapTilesIntoMemory(mapCoords, mapView->size());

    texturepW->initialize(world->getTextureManager());
}

void MainWindow::newProject()
{
    NewProject* newProj = new NewProject();

    connect(newProj, SIGNAL(wizardData(NewProjectData)), this, SLOT(createMemoryProject(NewProjectData)));

    newProj->show();
}

void MainWindow::closeProject()
{
    // if world is not saved
    // ....
    // ....
    // ....

    world->deleteMe();

    delete mapView;

    world   = NULL;
    mapView = NULL;

    // clean toolbars actions
    mode1Actions.clear();
    mode2Actions.clear();
    mode3Actions.clear();
    mode4Actions.clear();
    mode5Actions.clear();
    mode6Actions.clear();

    // clear toolbar
    ui->toolbar3->clear();

    // set pointer as main tool
    foreach(QAction* action, ui->toolbar->actions())
        action->setChecked(false);

    ui->action_mapview_m0->setChecked(true);

    startUp = new StartUp();

    setCentralWidget(startUp);

    connect(startUp, SIGNAL(createMemoryProject(NewProjectData)), this, SLOT(createMemoryProject(NewProjectData)));
    connect(startUp, SIGNAL(openProject(ProjectFileData)),        this, SLOT(openWorld(ProjectFileData)));
}

void MainWindow::takeScreenshot()
{
    if(centralWidget() == NULL)
        return;

    QString path = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);

    if(path == QString())
        path == QDir::homePath();

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save screenshoot"), path, tr("Image (*.png *.jpg)"));

    if(fileName == QString())
        return;

    QPixmap mainWindowPixmap = grab();

    QRect centralRect = QRect(0, 0, centralWidget()->width(), centralWidget()->height());

    QImage mapViewImage = mapView->grabFrameBuffer();

    QPainter painter;
    painter.begin(&mainWindowPixmap);
    painter.translate(QPoint(centralWidget()->x(), centralWidget()->y()));
    painter.drawImage(centralRect, mapViewImage, centralRect);
    painter.end();

    if(!mainWindowPixmap.toImage().save(fileName))
    {
        QMessageBox msg;
        msg.setWindowTitle("Error");
        msg.setIcon(QMessageBox::Critical);
        msg.setText(tr("Problem with saving screenshot! Try it again."));

        msg.exec();

        return;
    }
}

void MainWindow::takeScreenshotAndUpload()
{
    if(centralWidget() == NULL)
        return;

    QPixmap mainWindowPixmap = grab();

    QRect centralRect = QRect(0, 0, centralWidget()->width(), centralWidget()->height());

    QImage mapViewImage = mapView->grabFrameBuffer();

    QPainter painter;
    painter.begin(&mainWindowPixmap);
    painter.translate(QPoint(centralWidget()->x(), centralWidget()->y()));
    painter.drawImage(centralRect, mapViewImage, centralRect);
    painter.end();

    FileUpload* uploader = new FileUpload(this);

    connect(uploader, SIGNAL(uploadDone(QString)),  this, SLOT(screenshotUploadDone(QString)));
    connect(uploader, SIGNAL(uploadError(QString)), this, SLOT(screenshotUploadError(QString)));

    uploader->uploadImage(mainWindowPixmap.toImage());
}

void MainWindow::screenshotUploadDone(const QString& link)
{
    app().clipboard()->setText(link);

    QMessageBox msg;
    msg.setWindowTitle("Screenshot");
    msg.setText(tr("Image is uploaded on server Imgur. Link is in Clipboard (Ctrl + V)."));
    msg.setIcon(QMessageBox::Information);

    msg.exec();

    if(QObject::sender())
        qobject_cast<FileUpload*>(QObject::sender())->deleteLater();
}

void MainWindow::screenshotUploadError(const QString& error)
{
    QMessageBox msg;
    msg.setWindowTitle("Error");
    msg.setIcon(QMessageBox::Critical);
    msg.setText(error);

    msg.exec();

    if(QObject::sender())
        qobject_cast<FileUpload*>(QObject::sender())->deleteLater();
}

QString MainWindow::getActionName(QObject* object) const
{
    return qobject_cast<QAction*>(object)->objectName();
}

void MainWindow::setSpeedMultiplier()
{
    float multiplier = 1.0f;

    QString sName = getActionName(this->sender());

    if(sName == "action_Speed_0")
        multiplier = 0.0f;
    else if(sName == "action_Speed_1")
        multiplier = 1.0f;
    else if(sName == "action_Speed_3")
        multiplier = 3.0f;
    else if(sName == "action_Speed_5")
        multiplier = 5.0f;
    else if(sName == "action_Speed_7")
        multiplier = 7.0f;
    else if(sName == "action_Speed_10")
        multiplier = 10.0f;
    else if(sName == "action_Speed_0_1")
        multiplier = 0.1f;
    else if(sName == "action_Speed_0_2")
        multiplier = -0.1f;

    emit setSpeedMultiplier(multiplier);
}

void MainWindow::setDisplayMode()
{
    QString dName = getActionName(this->sender());

    if(dName == DisplayMode)
    {
        qobject_cast<QAction*>(this->sender())->setChecked(true);

        return;
    }

    foreach(QAction* action, ui->menu_Display_Mode->actions())
    {
        if(action->isChecked())
            action->setChecked(false);
    }

    qobject_cast<QAction*>(this->sender())->setChecked(true);

    DisplayMode = dName;

    if(dName == "action_Default")
        emit setDisplayMode(7);
    else if(dName == "action_Terrain_Wireframe")
        emit setDisplayMode(8);
    else if(dName == "action_Wireframe")
        emit setDisplayMode(0);
    else if(dName == "action_BaseLayer")
        emit setDisplayMode(3);
    else if(dName == "action_Base_Layer1")
        emit setDisplayMode(4);
    else if(dName == "action_Base_Layer1_Layer2")
        emit setDisplayMode(5);
    else if(dName == "action_Wireframe_Height")
        emit setDisplayMode(1);
    else if(dName == "action_Colored")
        emit setDisplayMode(2);
    else if(dName == "action_Light_and_Shadow")
        emit setDisplayMode(6);
    else if(dName == "action_Hidden")
        emit setDisplayMode(9);
}

void MainWindow::setToolBarItem()
{
    QString iName = getActionName(this->sender());

    if(iName == ToolBarItem)
    {
        qobject_cast<QAction*>(this->sender())->setChecked(true);

        return;
    }

    foreach(QAction* action, ui->toolbar->actions())
    {
        if(action->isChecked())
            action->setChecked(false);
    }

    qobject_cast<QAction*>(this->sender())->setChecked(true);

    ToolBarItem = iName;

    if(iName == "action_mapview_m0")
    {
        emit setModeEditing(0);

        showMode(mode0Actions);
    }
    else if(iName == "action_mapview_m1")
    {
        emit setModeEditing(1);

        showMode(mode1Actions);
    }
    else if(iName == "action_mapview_m2")
    {
        emit setModeEditing(2);

        showMode(mode2Actions);
    }
    else if(iName == "action_mapview_m3")
    {
        emit setModeEditing(3);

        showMode(mode3Actions);

        addDockWindow(tr("Vertex Shading"), colorW);
    }
    else if(iName == "action_mapview_m4")
    {
        emit setModeEditing(4);

        showMode(mode4Actions);

        addDockWindow(tr("Water"), waterW);
    }
    else if(iName == "action_mapview_m5")
    {
        emit setModeEditing(5);

        showMode(mode5Actions);

        addDockWindow(tr("Model Picker"), modelpickerW);
    }
    else if(iName == "action_mapview_m6")
    {
        emit setModeEditing(6);

        showMode(mode6Actions);

        addDockWindow(tr("Camera"), cameraW);
    }
}

void MainWindow::setFullscreen(bool checked)
{
    switch(checked)
    {
        case true:
            {
                nonFullScreenState = windowState();

                setWindowState(Qt::WindowFullScreen);
            }
            break;

        case false:
            setWindowState(nonFullScreenState);
            break;
    }
}

void MainWindow::showTeleport()
{
    addDockWindow(tr("Teleport"), teleportW);
}

void MainWindow::showSettings()
{
    addDockWindow(tr("Settings"), settingsW);
}

void MainWindow::showTexturePicker()
{
    addDockWindow(tr("Texture Picker"), texturepW);
}

void MainWindow::showAbout()
{
    About* about = new About();
    about->exec();
}

void MainWindow::showProjectSettings()
{
    ProjectFileData data = world->getProjectData();

    Project_Settings* projectSettings = new Project_Settings(data);

    connect(projectSettings, SIGNAL(projectDataChanged(ProjectFileData&)), this, SLOT(setProjectData(ProjectFileData&)));

    projectSettings->exec();
}

void MainWindow::showMapGeneration()
{
    MapGeneration* mapGeneration = new MapGeneration();

    connect(mapGeneration, SIGNAL(generate(MapGenerationData&)), this, SLOT(setMapGenerationData(MapGenerationData&)));
    connect(mapGeneration, SIGNAL(accepted()),                   this, SLOT(mapGenerationDataAccepted()));
    connect(mapGeneration, SIGNAL(rejected()),                   this, SLOT(mapGenerationDataRejected()));

    mapGeneration->show();
}

void MainWindow::showHeightmap()
{
    addDockWindow(tr("Heightmap settings"), heightmapW);
}

void MainWindow::addDockWindow(const QString& title, QWidget* widget, Qt::DockWidgetArea area)
{
    // Check if is already in any dock
    QList<QDockWidget*> dWidgets = findChildren<QDockWidget*>();

    foreach(QDockWidget* dWidget, dWidgets)
    {
        if(dWidget->widget() == widget)
        {
            if(widget->isVisible())
                return;

            removeDockWidget(dWidget);
        }
    }

    // Add
    QDockWidget* dockWidget = new QDockWidget(title);
    dockWidget->setWidget(widget);
    dockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    addDockWidget(area, dockWidget);
}

void MainWindow::initMode()
{
    t_outer_radius = new QDSlider();
    t_outer_radius->setMinimum(3.0);
    t_outer_radius->setMaximum(100.0);
    t_outer_radius->setValue(10.0);
    t_outer_radius->setMaximumWidth(this->width() / 3);
    t_outer_radius->setObjectName("t_outer_radius");

    t_inner_radius = new QDSlider();
    t_inner_radius->setMinimum(3.0);
    t_inner_radius->setMaximum(10.0);
    t_inner_radius->setValue(3.0);
    t_inner_radius->setMaximumWidth(this->width() / 3);
    t_inner_radius->setObjectName("t_inner_radius");

    t_speed = new QDSlider();
    t_speed->setMaximum(10.0);
    t_speed->setValue(1.0);
    t_speed->setMaximumWidth(this->width() / 3);
    t_speed->setObjectName("t_speed");

    t_flow = new QDSlider();
    t_flow->setMinimum(0.1);
    t_flow->setMaximum(1.0);
    t_flow->setValue(1.0);
    t_flow->setMaximumWidth(this->width() / 3);
    t_flow->setObjectName("t_flow");

    t_rotationx = new QDSlider(2,this);
    t_rotationx->setMinimum(0.0);
    t_rotationx->setMaximum(1.0);
    t_rotationx->setValue(0.0);
    t_rotationx->setMaximumWidth(this->width() / 6);
    t_rotationx->setObjectName("t_rotationx");

    t_rotationy = new QDSlider(2, this);
    t_rotationy->setMinimum(0.0);
    t_rotationy->setMaximum(1.0);
    t_rotationy->setValue(0.0);
    t_rotationy->setMaximumWidth(this->width() / 6);
    t_rotationy->setObjectName("t_rotationy");

    t_rotationz = new QDSlider(2, this);
    t_rotationz->setMinimum(0.0);
    t_rotationz->setMaximum(1.0);
    t_rotationz->setValue(0.0);
    t_rotationz->setMaximumWidth(this->width() / 6);
    t_rotationz->setObjectName("t_rotationz");

    t_impend = new QDSlider(2, this);
    t_impend->setMinimum(-1.0);
    t_impend->setMaximum(1.0);
    t_impend->setValue(0.0);
    t_impend->setMaximumWidth(this->width() / 6);
    t_impend->setObjectName("t_impend");

    t_scale = new QDSlider(2, this);
    t_scale->setMinimum(0.2);
    t_scale->setMaximum(5.0);
    t_scale->setValue(1.0);
    t_scale->setMaximumWidth(this->width() / 6);
    t_scale->setObjectName("t_scale");

    t_brush = new QButtonGroup();
    t_brush->setObjectName("t_brush");

    t_brush_circle = new QPushButton(QIcon(":/circle_icon"), "");
    t_brush_circle->setToolTip(tr("Circle Brush"));
    t_brush_circle->setCheckable(true);
    t_brush_circle->setChecked(true);
    t_brush_circle->setIconSize(QSize(24, 24));
    t_brush_circle->setObjectName("t_brush_circle");
    t_brush->addButton(t_brush_circle);

    t_brush_square = new QPushButton(QIcon(":/square_icon"), "");
    t_brush_square->setToolTip(tr("Square Brush"));
    t_brush_square->setCheckable(true);
    t_brush_square->setIconSize(QSize(24, 24));
    t_brush_square->setObjectName("t_brush_square");
    t_brush->addButton(t_brush_square);

    t_reset_transform = new QPushButton(QIcon("://reset"),"Reset");
    t_reset_transform->setToolTip(tr("Reset Transform"));
    t_reset_transform->setIconSize(QSize(24, 24));
    t_reset_transform->setObjectName("t_reset_transform");

    t_terrain_mode = new QComboBox();
    t_terrain_mode->setToolTip(tr("Select terrain mode"));
    t_terrain_mode->setObjectName("t_terrain_mode");
    t_terrain_mode->addItem(tr("Shaping")  , 0);
    t_terrain_mode->addItem(tr("Smoothing"), 1);

    t_terrain_mode_0.append(qMakePair<QString, QVariant>(tr("Linear")  , 1));
    t_terrain_mode_0.append(qMakePair<QString, QVariant>(tr("Flat")    , 2));
    t_terrain_mode_0.append(qMakePair<QString, QVariant>(tr("Smooth")  , 3));
    t_terrain_mode_0.append(qMakePair<QString, QVariant>(tr("Unknown1"), 4));
    t_terrain_mode_0.append(qMakePair<QString, QVariant>(tr("Unknown2"), 5));

    t_terrain_mode_1.append(qMakePair<QString, QVariant>(tr("Linear")  , 1));
    t_terrain_mode_1.append(qMakePair<QString, QVariant>(tr("Flat")    , 2));
    t_terrain_mode_1.append(qMakePair<QString, QVariant>(tr("Smooth")  , 3));

    t_modelmode = new QButtonGroup();
    t_modelmode->setObjectName("t_modelmode");

    t_modelmode_insert = new QPushButton(QIcon(":/insert_icon"), "");
    t_modelmode_insert->setToolTip(tr("Insert"));
    t_modelmode_insert->setCheckable(true);
    t_modelmode_insert->setChecked(true);
    t_modelmode_insert->setIconSize(QSize(24, 24));
    t_modelmode_insert->setObjectName("t_modelmode_insert");
    t_modelmode->addButton(t_modelmode_insert);

    t_modelmode_delete = new QPushButton(QIcon(":/remove_icon"), "");
    t_modelmode_delete->setToolTip(tr("Delete"));
    t_modelmode_delete->setCheckable(true);
    t_modelmode_delete->setIconSize(QSize(24, 24));
    t_modelmode_delete->setObjectName("t_modelmode_delete");
    t_modelmode->addButton(t_modelmode_delete);

    t_brush_type = new QComboBox();
    t_brush_type->setToolTip(tr("Select brush type"));
    t_brush_type->setObjectName("t_brush_type");

    for(int i = 0; i < t_terrain_mode_0.count(); ++i)
        t_brush_type->addItem(t_terrain_mode_0.at(i).first, t_terrain_mode_0.at(i).second);

    t_brush_mode = new QComboBox();
    t_brush_mode->setToolTip("Select brush mode");
    t_brush_mode->setObjectName("t_brush_mode");
    t_brush_mode->addItem(tr("Unselected"), 0);
    t_brush_mode->addItem(tr("Raising"), 1);
    t_brush_mode->addItem(tr("Lowering"), 2);

    t_terrain_maxHeight = new QCheckBox(tr("Maximum height:"));
    t_terrain_maxHeight->setObjectName("t_terrain_maxHeight");
    t_terrain_maxHeight->setStyleSheet("margin:-3px 5px 0 0;");

    t_paint_maxAlpha = new QCheckBox(tr("Maximum alpha:"));
    t_paint_maxAlpha->setObjectName("t_paint_maxAlpha");
    t_paint_maxAlpha->setStyleSheet("margin:-3px 5px 0 0;");

    t_terrain_maximum_height = new QDoubleSpinBox();
    t_terrain_maximum_height->setDecimals(2);
    t_terrain_maximum_height->setMinimum(MathHelper::toDouble(std::numeric_limits<float>::min()));
    t_terrain_maximum_height->setMaximum(MathHelper::toDouble(std::numeric_limits<float>::max()));
    t_terrain_maximum_height->setObjectName("t_terrain_maximum_height");
    t_terrain_maximum_height->setEnabled(false);

    t_paint_maximum_alpha = new QDoubleSpinBox();
    t_paint_maximum_alpha->setDecimals(2);
    t_paint_maximum_alpha->setMinimum(0.0);
    t_paint_maximum_alpha->setMaximum(1.0);
    t_paint_maximum_alpha->setSingleStep(0.1);
    t_paint_maximum_alpha->setObjectName("t_paint_maximum_alpha");
    t_paint_maximum_alpha->setEnabled(false);

    colorW = new QColorDialog();
    colorW->setOption(QColorDialog::NoButtons);
    colorW->setOption(QColorDialog::ShowAlphaChannel);
    colorW->setOption(QColorDialog::DontUseNativeDialog);

    t_terrain_mode_label = new QLabel(tr("Mode:"));

    t_brush_label      = new QLabel(tr("Brushes:"));
    t_brush_type_label = new QLabel(tr("Brush type:"));
    t_brush_mode_label = new QLabel(tr("Brush mode:"));

    t_outer_radius_label       = new QLabel(tr("Outer radius:"));
    t_outer_radius_value_label = new QLabel(QString("%1").arg(t_outer_radius->value()));
    t_inner_radius_label       = new QLabel(tr("Inner radius:"));
    t_inner_radius_value_label = new QLabel(QString("%1").arg(t_inner_radius->value()));
    t_speed_label              = new QLabel(tr("Speed:"));
    t_speed_value_label        = new QLabel(QString("%1").arg(t_speed->value()));
    t_flow_label               = new QLabel(tr("Flow:"));
    t_flow_value_label         = new QLabel(QString("%1").arg(t_flow->value()));

    t_model_mode_label         = new QLabel(tr("Mode:"));
    t_rotationx_label          = new QLabel(tr("Rotation X:"));
    t_rotationx_value_label    = new QLabel(QString("%1").arg(t_rotationx->value()));
    t_rotationy_label          = new QLabel(tr("Rotation Y:"));
    t_rotationy_value_label    = new QLabel(QString("%1").arg(t_rotationy->value()));
    t_rotationz_label          = new QLabel(tr("Rotation Z:"));
    t_rotationz_value_label    = new QLabel(QString("%1").arg(t_rotationz->value()));
    t_impend_label             = new QLabel(tr("Impend:"));
    t_impend_value_label       = new QLabel(QString("%1").arg(t_impend->value()));
    t_scale_label              = new QLabel(tr("Scale:"));
    t_scale_value_label        = new QLabel(QString("%1").arg(t_scale->value()));

    t_terrain_mode_label->setObjectName("t_terrain_mode_label");
    t_brush_label->setObjectName("t_brush_label");
    t_brush_type_label->setObjectName("t_brush_type_label");
    t_brush_mode_label->setObjectName("t_brush_mode_label");
    t_outer_radius_label->setObjectName("t_outer_radius_label");
    t_outer_radius_value_label->setObjectName("t_outer_radius_value_label");
    t_inner_radius_label->setObjectName("t_inner_radius_label");
    t_inner_radius_value_label->setObjectName("t_inner_radius_value_label");
    t_speed_label->setObjectName("t_speed_label");
    t_speed_value_label->setObjectName("t_speed_value_label");
    t_flow_label->setObjectName("t_flow_label");
    t_flow_value_label->setObjectName("t_flow_value_label");

    t_terrain_mode_label->setStyleSheet("margin:-3px 5px 0 0;");

    t_brush_label->setStyleSheet("margin:-3px 5px 0 0;");
    t_brush_type_label->setStyleSheet("margin:-3px 5px 0 0;");
    t_brush_mode_label->setStyleSheet("margin:-3px 5px 0 0;");

    t_outer_radius_label->setStyleSheet("margin:-3px 5px 0 20px;");
    t_inner_radius_label->setStyleSheet("margin:-3px 5px 0 20px;");
    t_speed_label->setStyleSheet("margin:-3px 5px 0 20px;");
    t_flow_label->setStyleSheet("margin:-3px 5px 0 20px;");

    t_model_mode_label->setStyleSheet("margin:-3px 5px 0 0;");
    t_rotationx_label->setStyleSheet("margin:-3px 5px 0 20px;");
    t_rotationy_label->setStyleSheet("margin:-3px 5px 0 20px;");
    t_rotationz_label->setStyleSheet("margin:-3px 5px 0 20px;");
    t_impend_label->setStyleSheet("margin:-3px 5px 0 20px;");
    t_scale_label->setStyleSheet("margin:-3px 5px 0 20px;");

    t_outer_radius_value_label->setStyleSheet("margin:-3px 0 0 5px;");
    t_inner_radius_value_label->setStyleSheet("margin:-3px 0 0 5px;");
    t_speed_value_label->setStyleSheet("margin:-3px 0 0 5px;");
    t_flow_value_label->setStyleSheet("margin:-3px 0 0 5px;");
    t_flow_value_label->setStyleSheet("margin:-3px 0 0 5px;");    
    t_rotationx_value_label->setStyleSheet("margin:-3px 0 0 5px;");
    t_rotationy_value_label->setStyleSheet("margin:-3px 0 0 5px;");
    t_rotationz_value_label->setStyleSheet("margin:-3px 0 0 5px;");
    t_impend_value_label->setStyleSheet("margin:-3px 0 0 5px;");
    t_scale_value_label->setStyleSheet("margin:-3px 0 0 5px;");

    /// mode1
    addToolbarAction(t_brush_label             , mode1Actions);
    addToolbarAction(t_brush_circle            , mode1Actions);
    addToolbarAction(t_brush_square            , mode1Actions);
    addToolbarAction(t_terrain_mode_label      , mode1Actions);
    addToolbarAction(t_terrain_mode            , mode1Actions);
    addToolbarAction(t_brush_type_label        , mode1Actions);
    addToolbarAction(t_brush_type              , mode1Actions);
    addToolbarAction(t_brush_mode_label        , mode1Actions);
    addToolbarAction(t_brush_mode              , mode1Actions);
    addToolbarAction(t_outer_radius_label      , mode1Actions);
    addToolbarAction(t_outer_radius            , mode1Actions);
    addToolbarAction(t_outer_radius_value_label, mode1Actions);
    addToolbarAction(t_inner_radius_label      , mode1Actions);
    addToolbarAction(t_inner_radius            , mode1Actions);
    addToolbarAction(t_inner_radius_value_label, mode1Actions);
    addToolbarAction(t_speed_label             , mode1Actions);
    addToolbarAction(t_speed                   , mode1Actions);
    addToolbarAction(t_speed_value_label       , mode1Actions);
    addToolbarAction(t_terrain_maxHeight       , mode1Actions);
    addToolbarAction(t_terrain_maximum_height  , mode1Actions);

    /// mode2
    addToolbarAction(t_brush_label             , mode2Actions);
    addToolbarAction(t_brush_circle            , mode2Actions);
    addToolbarAction(t_brush_square            , mode2Actions);
    addToolbarAction(t_outer_radius_label      , mode2Actions);
    addToolbarAction(t_outer_radius            , mode2Actions);
    addToolbarAction(t_outer_radius_value_label, mode2Actions);
    addToolbarAction(t_inner_radius_label      , mode2Actions);
    addToolbarAction(t_inner_radius            , mode2Actions);
    addToolbarAction(t_inner_radius_value_label, mode2Actions);
    addToolbarAction(t_flow_label              , mode2Actions);
    addToolbarAction(t_flow                    , mode2Actions);
    addToolbarAction(t_flow_value_label        , mode2Actions);
    addToolbarAction(t_paint_maxAlpha          , mode2Actions);
    addToolbarAction(t_paint_maximum_alpha     , mode2Actions);

    // mode3
    addToolbarAction(t_brush_label             , mode3Actions);
    addToolbarAction(t_brush_circle            , mode3Actions);
    addToolbarAction(t_brush_square            , mode3Actions);
    addToolbarAction(t_outer_radius_label      , mode3Actions);
    addToolbarAction(t_outer_radius            , mode3Actions);
    addToolbarAction(t_outer_radius_value_label, mode3Actions);
    addToolbarAction(t_inner_radius_label      , mode3Actions);
    addToolbarAction(t_inner_radius            , mode3Actions);
    addToolbarAction(t_inner_radius_value_label, mode3Actions);
    addToolbarAction(t_flow_label              , mode3Actions);
    addToolbarAction(t_flow                    , mode3Actions);
    addToolbarAction(t_flow_value_label        , mode3Actions);
    addToolbarAction(t_paint_maxAlpha          , mode3Actions);
    addToolbarAction(t_paint_maximum_alpha     , mode3Actions);

    // mode5
    addToolbarAction(t_model_mode_label        , mode5Actions);
    addToolbarAction(t_modelmode_insert        , mode5Actions);
    addToolbarAction(t_modelmode_delete        , mode5Actions);
    addToolbarAction(t_reset_transform         , mode5Actions);
    addToolbarAction(t_rotationx_label         , mode5Actions);
    addToolbarAction(t_rotationx               , mode5Actions);
    addToolbarAction(t_rotationx_value_label   , mode5Actions);
    addToolbarAction(t_rotationy_label         , mode5Actions);
    addToolbarAction(t_rotationy               , mode5Actions);
    addToolbarAction(t_rotationy_value_label   , mode5Actions);
    addToolbarAction(t_rotationz_label         , mode5Actions);
    addToolbarAction(t_rotationz               , mode5Actions);
    addToolbarAction(t_rotationz_value_label   , mode5Actions);
    addToolbarAction(t_impend_label            , mode5Actions);
    addToolbarAction(t_impend                  , mode5Actions);
    addToolbarAction(t_impend_label            , mode5Actions);
    addToolbarAction(t_scale_label             , mode5Actions);
    addToolbarAction(t_scale                   , mode5Actions);
    addToolbarAction(t_scale_value_label       , mode5Actions);

    // mode6


    hideToolbarActions();
}

void MainWindow::showMode(QList<QString>& parentList)
{
    hideToolbarActions();

    QList<QAction*> actions = ui->toolbar3->actions();

    foreach(QAction* action, actions)
    {
        for(int i = 0; i < parentList.count(); ++i)
        {
            if(action->objectName() == parentList[i])
                action->setVisible(true);
        }
    }
}

void MainWindow::setTerrainMaximumHeightState(int state)
{
    switch(state)
    {
        case 0:
        default:
            {
                t_terrain_maximum_height->setEnabled(false);

                world->setTerrainMaximumState(false);
            }
            break;

        case 2:
            {
                t_terrain_maximum_height->setEnabled(true);

                world->setTerrainMaximumState(true);
            }
            break;
    }
}

void MainWindow::setPaintMaximumAlphaState(int state)
{
    switch(state)
    {
        case 0:
        default:
            {
                t_paint_maximum_alpha->setEnabled(false);

                world->setPaintMaximumState(false);
            }
            break;

        case 2:
            {
                t_paint_maximum_alpha->setEnabled(true);

                world->setPaintMaximumState(true);
            }
            break;
    }
}

void MainWindow::setBrush(int unknown, bool unknown2)
{
    Q_UNUSED(unknown);
    Q_UNUSED(unknown2);

    if(t_brush->checkedButton() == t_brush_square)
    {
        QMessageBox msg;
        msg.setWindowTitle("Error");
        msg.setText(tr("Brush is not ready yet."));
        msg.setIcon(QMessageBox::Critical);

        msg.exec();

        t_brush->buttonClicked(t_brush_circle);

        t_brush_circle->setChecked(true);
    }
}

void MainWindow::setBrushType(int type)
{
    Q_UNUSED(type);

    t_speed->setValue(1.0);
}

void MainWindow::setBrushMode(MapView::eMouseMode& mouseMode, MapView::eEditingMode& editingMode)
{
    switch(editingMode)
    {
        case MapView::Terrain:
            {
                if((int)mouseMode < t_brush_mode->count())
                {
                    if((int)mouseMode != t_brush_mode->currentIndex())
                        t_brush_mode->setCurrentIndex((int)mouseMode);
                    else
                        t_brush_mode->setCurrentIndex(0);

                    if(t_brush_mode->currentIndex() > 0)
                        world->getCamera()->setLock(true);
                    else
                        world->getCamera()->setLock(false);
                }
                // else set nothing???
            }
            break;
    }
}

void MainWindow::setBrushOuterRadius(double value)
{
    t_outer_radius->setValue(t_outer_radius->value() + value);

    t_inner_radius->setMaximum(t_outer_radius->value());

    if(t_inner_radius->value() > t_outer_radius->value())
        t_inner_radius->setValue(t_outer_radius->value());
}

void MainWindow::setBrushInnerRadius(double value)
{
    t_inner_radius->setValue(t_inner_radius->value() + value);
}

void MainWindow::setBrushInnerRadiusMaximumValue(double maximum)
{
    Q_UNUSED(maximum);

    t_inner_radius->setMaximum(t_outer_radius->value());
}

void MainWindow::resetModelBrush()
{
    t_rotationx->setValue(0.0f);
    t_rotationy->setValue(0.0f);
    t_rotationz->setValue(0.0f);
    t_impend->setValue(0.0f);
    t_scale->setValue(1.0f);
}

void MainWindow::setTerrain_Mode(int index)
{
    switch(index)
    {
        case 0:
        default:
            {
                t_brush_type->clear();

                for(int i = 0; i < t_terrain_mode_0.count(); ++i)
                    t_brush_type->addItem(t_terrain_mode_0.at(i).first, t_terrain_mode_0.at(i).second);
            }
            break;

        case 1:
            {
                t_brush_type->clear();

                for(int i = 0; i < t_terrain_mode_1.count(); ++i)
                    t_brush_type->addItem(t_terrain_mode_1.at(i).first, t_terrain_mode_1.at(i).second);
            }
            break;
    }

    emit setTerrainMode(index);
}

void MainWindow::setModel_Mode(int unknown, bool unknown2)
{
    Q_UNUSED(unknown);
    Q_UNUSED(unknown2);

    if(t_modelmode_insert->isChecked())
        mapView->setModelMode(MapView::Insertion);
    else
        mapView->setModelMode(MapView::Removal);
}

void MainWindow::actionIsInDevelopment()
{
    QMessageBox msg;
    msg.setWindowTitle("Error");
    msg.setText(tr("This action is under development."));
    msg.setIcon(QMessageBox::Critical);

    msg.exec();
}

void MainWindow::mapGenerationDataAccepted()
{
    world->mapGenerationAccepted();

    QObject::sender()->deleteLater();
}

void MainWindow::mapGenerationDataRejected()
{
    world->mapGenerationRejected();

    QObject::sender()->deleteLater();
}

void MainWindow::setMapGenerationData(MapGenerationData& data)
{
    world->setMapGenerationData(data);
}

void MainWindow::setProjectData(ProjectFileData& data)
{
    world->setProjectData(data);
}

void MainWindow::heightmapWidgetAccepted()
{
    world->heightmapWidgetAccepted();
}

void MainWindow::heightmapWidgetRejected()
{
    world->heightmapWidgetRejected();
}

void MainWindow::importingHeightmap(QString path, float scale)
{
    world->importHeightmap(path, scale);
}

void MainWindow::exportingHeightmap(QString path, float scale)
{
    world->exportHeightmap(path, scale);
}

void MainWindow::setHeightmapScale(float scale)
{
    world->setHeightmapScale(scale);
}

void MainWindow::setVertexShadingSwitch(bool state)
{
    world->setVertexShadingSwitch(state);
}

void MainWindow::setSkyboxSwitch(bool state)
{
    world->setSkyboxSwitch(state);
}

void MainWindow::addToolbarAction(QWidget* widget, QList<QString>& parentList)
{
    bool isThere = false;

    foreach(QAction* action, ui->toolbar3->actions())
    {
        if(ui->toolbar3->widgetForAction(action) == widget)
        {
            isThere = true;

            break;
        }
    }

    if(!isThere)
    {
        ui->toolbar3->addWidget(widget);
        ui->toolbar3->actions().last()->setObjectName(widget->objectName());
    }

    parentList.append(widget->objectName());
}

void MainWindow::hideToolbarActions()
{
    foreach(QAction* action, ui->toolbar3->actions())
        action->setVisible(false);
}