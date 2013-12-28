#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "ui/about.h"
#include "ui/project_settings.h"

#include "qeditor.h"

#include <QAction>

MainWindow::MainWindow(QWidget* parent)
: QMainWindow(parent)
, ui(new Ui::MainWindow)
, DisplayMode("action_Default")
, ToolBarItem("action_mapview_m0")
, teleportW(NULL)
, settingsW(NULL)
, texturepW(NULL)
, t_radius(NULL)
, t_speed(NULL)
, t_brush(NULL)
, t_brush_circle(NULL)
, t_brush_square(NULL)
, t_terrain_mode(NULL)
, t_brush_type(NULL)
, t_terrain_mode_label(NULL)
, t_brush_label(NULL)
, t_brush_type_label(NULL)
, t_radius_label(NULL)
, t_radius_value_label(NULL)
, t_speed_label(NULL)
, t_speed_value_label(NULL)
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
}

MainWindow::~MainWindow()
{
    delete ui;

    delete teleportW;
    delete settingsW;
    delete texturepW;

    deleteObject(t_radius);
    deleteObject(t_speed);

    deleteObject(t_brush);

    deleteObject(t_brush_circle);
    deleteObject(t_brush_square);

    deleteObject(t_terrain_mode);
    deleteObject(t_brush_type);

    deleteObject(t_terrain_mode_label);

    deleteObject(t_brush_label);
    deleteObject(t_brush_type_label);

    deleteObject(t_radius_label);
    deleteObject(t_radius_value_label);
    deleteObject(t_speed_label);
    deleteObject(t_speed_value_label);

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
    teleportW = new TeleportWidget();
    settingsW = new MapView_Settings();
    texturepW = new TexturePicker();

    // post initialize world sub widgets
    connect(mapView, SIGNAL(initialized()), this, SLOT(postInitializeSubWorldWidgets()));

    // init modes
    initMode();

    /// map view
    connect(mapView, SIGNAL(statusBar(QString)),          ui->statusbar, SLOT(showMessage(QString)));
    connect(mapView, SIGNAL(updateShapingRadius(double)), this,          SLOT(setShapingRadius(double)));
    connect(mapView, SIGNAL(selectedMapChunk(MapChunk*)), texturepW,     SLOT(setChunk(MapChunk*)));

    /// menu bar
    // file
    connect(ui->action_Save, SIGNAL(triggered()), mapView, SLOT(save()));

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

    // tools - texture picker, settings, teleport, reset camera
    connect(ui->action_Texture_Picker, SIGNAL(triggered()), this,    SLOT(showTexturePicker()));
    connect(ui->action_Settings,       SIGNAL(triggered()), this,    SLOT(showSettings()));
    connect(ui->action_Teleport,       SIGNAL(triggered()), this,    SLOT(showTeleport()));
    connect(ui->action_Reset_camera,   SIGNAL(triggered()), mapView, SLOT(resetCamera()));

    connect(teleportW, SIGNAL(TeleportTo(QVector3D*)), mapView, SLOT(setCameraPosition(QVector3D*)));

    connect(settingsW, SIGNAL(setColorOfBrush(QColor*)),      mapView, SLOT(setBrushColor(QColor*)));
    connect(settingsW, SIGNAL(setEnvironmentDistance(float)), mapView, SLOT(setEnvionmentDistance(float)));
    connect(settingsW, SIGNAL(setTextureScaleOption(int)),    mapView, SLOT(setTextureScaleOption_(int)));
    connect(settingsW, SIGNAL(setTextureScaleFar(float)),     mapView, SLOT(setTextureScaleFar(float)));
    connect(settingsW, SIGNAL(setTextureScaleNear(float)),    mapView, SLOT(setTextureScaleNear(float)));

    // tools - test
    connect(ui->action_Test, SIGNAL(triggered()), mapView, SLOT(doTest()));

    /// toolbar
    connect(ui->action_mapview_m0, SIGNAL(triggered()), this, SLOT(setToolBarItem()));
    connect(ui->action_mapview_m1, SIGNAL(triggered()), this, SLOT(setToolBarItem()));
    connect(ui->action_mapview_m2, SIGNAL(triggered()), this, SLOT(setToolBarItem()));

    connect(this, SIGNAL(setModeEditing(int)), mapView, SLOT(setModeEditing(int)));

    /// toolbar3
    connect(t_terrain_mode, SIGNAL(currentIndexChanged(int)), this   , SLOT(setTerrain_Mode(int)));
    connect(this,           SIGNAL(setTerrainMode(int))     , mapView, SLOT(setTerrainMode(int)));

    connect(t_brush_type  , SIGNAL(currentIndexChanged(int)), mapView, SLOT(setShapingBrushType(int)));

    connect(t_radius, SIGNAL(valueChanged(double)), t_radius_value_label, SLOT(setNum(double)));
    connect(t_speed,  SIGNAL(valueChanged(double)), t_speed_value_label,  SLOT(setNum(double)));
    connect(t_flow,   SIGNAL(valueChanged(double)), t_flow_value_label,   SLOT(setNum(double)));

    connect(t_radius, SIGNAL(valueChanged(double)), mapView, SLOT(setShapingRadius(double)));
    connect(t_speed,  SIGNAL(valueChanged(double)), mapView, SLOT(setShapingSpeed(double)));
    connect(t_flow,   SIGNAL(valueChanged(double)), mapView, SLOT(setTexturingFlow(double)));
}

void MainWindow::postInitializeSubWorldWidgets()
{
    texturepW->initialize(world->getTextureManager());
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

    openWorld(projectFile);

    connect(mapView, SIGNAL(initialized()), this, SLOT(loadNewProjectMapTilesIntoMemory()));
}

void MainWindow::loadNewProjectMapTilesIntoMemory()
{
    world->loadNewProjectMapTilesIntoMemory(mapCoords);
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

void MainWindow::addDockWindow(const QString& title, QWidget* widget, Qt::DockWidgetArea area)
{
    // Check if is already in any dock
    QList<QDockWidget*> dWidgets = findChildren<QDockWidget*>();

    foreach(QDockWidget* dWidget, dWidgets)
    {
        if(dWidget->widget() == widget)
            return;
    }

    // Add
    QDockWidget* dockWidget = new QDockWidget(title);
    dockWidget->setWidget(widget);
    dockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    addDockWidget(area, dockWidget);
}

void MainWindow::initMode()
{
    t_radius = new QDSlider();
    t_radius->setMinimum(3.0);
    t_radius->setMaximum(100.0);
    t_radius->setValue(10.0);
    t_radius->setMaximumWidth(this->width() / 3);
    t_radius->setObjectName("t_radius");

    t_speed = new QDSlider();
    t_speed->setMaximum(10.0);
    t_speed->setValue(1.0);
    t_speed->setMaximumWidth(this->width() / 3);
    t_speed->setObjectName("t_speed");

    t_flow = new QDSlider();
    t_flow->setMinimum(0.1);
    t_flow->setMaximum(1.0);
    t_flow->setValue(0.5);
    t_flow->setMaximumWidth(this->width() / 3);
    t_flow->setObjectName("t_flow");

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

    t_brush_type = new QComboBox();
    t_brush_type->setToolTip(tr("Select brush type"));
    t_brush_type->setObjectName("t_brush_type");

    for(int i = 0; i < t_terrain_mode_0.count(); ++i)
        t_brush_type->addItem(t_terrain_mode_0.at(i).first, t_terrain_mode_0.at(i).second);

    t_terrain_mode_label = new QLabel(tr("Mode:"));

    t_brush_label      = new QLabel(tr("Brushes:"));
    t_brush_type_label = new QLabel(tr("Brush type:"));

    t_radius_label       = new QLabel(tr("Radius:"));
    t_radius_value_label = new QLabel(QString("%1").arg(t_radius->value()));
    t_speed_label        = new QLabel(tr("Speed:"));
    t_speed_value_label  = new QLabel(QString("%1").arg(t_speed->value()));
    t_flow_label         = new QLabel(tr("Flow:"));
    t_flow_value_label   = new QLabel(QString("%1").arg(t_flow->value()));

    t_terrain_mode_label->setObjectName("t_terrain_mode_label");
    t_brush_label->setObjectName("t_brush_label");
    t_brush_type_label->setObjectName("t_brush_type_label");
    t_radius_label->setObjectName("t_radius_label");
    t_radius_value_label->setObjectName("t_radius_value_label");
    t_speed_label->setObjectName("t_speed_label");
    t_speed_value_label->setObjectName("t_speed_value_label");
    t_flow_label->setObjectName("t_flow_label");
    t_flow_value_label->setObjectName("t_flow_value_label");

    t_terrain_mode_label->setStyleSheet("margin:-3px 5px 0 0;");

    t_brush_label->setStyleSheet("margin:-3px 5px 0 0;");
    t_brush_type_label->setStyleSheet("margin:-3px 5px 0 0;");

    t_radius_label->setStyleSheet("margin:-3px 5px 0 20px;");
    t_speed_label->setStyleSheet("margin:-3px 5px 0 20px;");
    t_flow_label->setStyleSheet("margin:-3px 5px 0 20px;");

    t_radius_value_label->setStyleSheet("margin:-3px 0 0 5px;");
    t_speed_value_label->setStyleSheet("margin:-3px 0 0 5px;");
    t_flow_value_label->setStyleSheet("margin:-3px 0 0 5px;");

    /// mode1
    addToolbarAction(t_brush_label       , mode1Actions);
    addToolbarAction(t_brush_circle      , mode1Actions);
    addToolbarAction(t_brush_square      , mode1Actions);
    addToolbarAction(t_terrain_mode_label, mode1Actions);
    addToolbarAction(t_terrain_mode      , mode1Actions);
    addToolbarAction(t_brush_type_label  , mode1Actions);
    addToolbarAction(t_brush_type        , mode1Actions);
    addToolbarAction(t_radius_label      , mode1Actions);
    addToolbarAction(t_radius            , mode1Actions);
    addToolbarAction(t_radius_value_label, mode1Actions);
    addToolbarAction(t_speed_label       , mode1Actions);
    addToolbarAction(t_speed             , mode1Actions);
    addToolbarAction(t_speed_value_label , mode1Actions);

    /// mode2
    addToolbarAction(t_brush_label       , mode2Actions);
    addToolbarAction(t_brush_circle      , mode2Actions);
    addToolbarAction(t_brush_square      , mode2Actions);
    addToolbarAction(t_radius_label      , mode2Actions);
    addToolbarAction(t_radius            , mode2Actions);
    addToolbarAction(t_radius_value_label, mode2Actions);
    addToolbarAction(t_flow_label        , mode2Actions);
    addToolbarAction(t_flow              , mode2Actions);
    addToolbarAction(t_flow_value_label  , mode2Actions);

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

void MainWindow::setShapingRadius(double value)
{
    t_radius->setValue(t_radius->value() + value);
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

void MainWindow::setProjectData(ProjectFileData& data)
{
    world->setProjectData(data);
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