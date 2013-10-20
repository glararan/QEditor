#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "ui/about.h"

#include <QAction>

MainWindow::MainWindow(QWidget* parent)
: QMainWindow(parent)
, ui(new Ui::MainWindow)
, DisplayMode("action_Default")
, ToolBarItem("action_mapview_m0")
, teleportW(new TeleportWidget)
{
    ui->setupUi(this);

    // world constructor
    world = new World("test");

    // map view constructor
    mapView = new MapView(world, this);

    setCentralWidget(mapView);

    // settings constructor
    settingsW = new MapView_Settings(world->getBrush()->Color());

    // init modes
    initMode();

    /// map view
    connect(mapView, SIGNAL(statusBar(QString)), ui->statusbar, SLOT(showMessage(QString)));
    connect(mapView, SIGNAL(updateShapingRadius(double)), this, SLOT(setShapingRadius(double)));

    /// menu bar
    // file
    connect(ui->action_Save, SIGNAL(triggered()), mapView, SLOT(save()));
    connect(ui->action_Exit, SIGNAL(triggered()), this, SLOT(close()));

    // tools - speed multiplier
    connect(ui->actionSpeed_0  , SIGNAL(triggered()), this, SLOT(setSpeedMultiplier()));
    connect(ui->actionSpeed_1  , SIGNAL(triggered()), this, SLOT(setSpeedMultiplier()));
    connect(ui->actionSpeed_3  , SIGNAL(triggered()), this, SLOT(setSpeedMultiplier()));
    connect(ui->actionSpeed_5  , SIGNAL(triggered()), this, SLOT(setSpeedMultiplier()));
    connect(ui->actionSpeed_7  , SIGNAL(triggered()), this, SLOT(setSpeedMultiplier()));
    connect(ui->actionSpeed_10 , SIGNAL(triggered()), this, SLOT(setSpeedMultiplier()));
    connect(ui->actionSpeed_0_1, SIGNAL(triggered()), this, SLOT(setSpeedMultiplier()));
    connect(ui->actionSpeed_0_2, SIGNAL(triggered()), this, SLOT(setSpeedMultiplier()));

    connect(this, SIGNAL(setSpeedMultiplier(float)), mapView, SLOT(setSpeedMultiplier(float)));

    // tools - display mode
    connect(ui->action_Default          , SIGNAL(triggered()), this, SLOT(setDisplayMode()));
    connect(ui->action_Terrain_Wireframe, SIGNAL(triggered()), this, SLOT(setDisplayMode()));
    connect(ui->action_Wireframe        , SIGNAL(triggered()), this, SLOT(setDisplayMode()));
    connect(ui->action_Grass            , SIGNAL(triggered()), this, SLOT(setDisplayMode()));
    connect(ui->actionGrass_Rock        , SIGNAL(triggered()), this, SLOT(setDisplayMode()));
    connect(ui->actionGrass_Rock_Snow   , SIGNAL(triggered()), this, SLOT(setDisplayMode()));
    connect(ui->actionWireframe_Height  , SIGNAL(triggered()), this, SLOT(setDisplayMode()));
    connect(ui->action_Colored          , SIGNAL(triggered()), this, SLOT(setDisplayMode()));
    connect(ui->action_Light_and_Shadow , SIGNAL(triggered()), this, SLOT(setDisplayMode()));
    connect(ui->actionH_idden           , SIGNAL(triggered()), this, SLOT(setDisplayMode()));

    connect(this, SIGNAL(setDisplayMode(int)), mapView, SLOT(setDisplayMode(int)));

    // tools - settings, teleport, reset camera
    connect(ui->action_Settings,     SIGNAL(triggered()), this,    SLOT(showSettings()));
    connect(ui->actionTeleport,      SIGNAL(triggered()), this,    SLOT(showTeleport()));
    connect(ui->action_Reset_camera, SIGNAL(triggered()), mapView, SLOT(resetCamera()));

    connect(teleportW, SIGNAL(TeleportTo(QVector3D*)), mapView, SLOT(setCameraPosition(QVector3D*)));

    connect(settingsW, SIGNAL(setColorOfBrush(QColor*)),      mapView, SLOT(setBrushColor(QColor*)));
    connect(settingsW, SIGNAL(setEnvironmentDistance(float)), mapView, SLOT(setEnvionmentDistance(float)));

    // tools - test
    connect(ui->action_Test, SIGNAL(triggered()), mapView, SLOT(doTest()));

    // help - about
    connect(ui->action_About, SIGNAL(triggered()), this, SLOT(showAbout()));

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

    connect(t_radius, SIGNAL(valueChanged(double)), mapView, SLOT(setShapingRadius(double)));
    connect(t_speed,  SIGNAL(valueChanged(double)), mapView, SLOT(setShapingSpeed(double)));
}

MainWindow::~MainWindow()
{
    delete ui;

    delete teleportW;
    delete settingsW;

    delete t_radius;
    delete t_speed;

    delete t_brush;

    delete t_brush_circle;
    delete t_brush_square;

    delete t_terrain_mode;
    delete t_brush_type;

    delete t_terrain_mode_label;

    delete t_brush_label;
    delete t_brush_type_label;

    delete t_radius_label;
    delete t_radius_value_label;
    delete t_speed_label;
    delete t_speed_value_label;

    qDebug() << "destroyed UI in MainWindow";

    delete world;
    delete mapView;

    qDebug() << "MainWindow was destroyed!";
}

QString MainWindow::getActionName(QObject* object) const
{
    return qobject_cast<QAction*>(object)->objectName();
}

void MainWindow::setSpeedMultiplier()
{
    float multiplier = 1.0f;

    QString sName = getActionName(this->sender());

    if(sName == "actionSpeed_0")
        multiplier = 0.0f;
    else if(sName == "actionSpeed_1")
        multiplier = 1.0f;
    else if(sName == "actionSpeed_3")
        multiplier = 3.0f;
    else if(sName == "actionSpeed_5")
        multiplier = 5.0f;
    else if(sName == "actionSpeed_7")
        multiplier = 7.0f;
    else if(sName == "actionSpeed_10")
        multiplier = 10.0f;
    else if(sName == "actionSpeed_0_1")
        multiplier = 0.1f;
    else if(sName == "actionSpeed_0_2")
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
    else if(dName == "action_Grass")
        emit setDisplayMode(3);
    else if(dName == "actionGrass_Rock")
        emit setDisplayMode(4);
    else if(dName == "actionGrass_Rock_Snow")
        emit setDisplayMode(5);
    else if(dName == "actionWireframe_Height")
        emit setDisplayMode(1);
    else if(dName == "action_Colored")
        emit setDisplayMode(2);
    else if(dName == "action_Light_and_Shadow")
        emit setDisplayMode(6);
    else if(dName == "actionH_idden")
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
    addDockWindow("Teleport", teleportW);
}

void MainWindow::showSettings()
{
    addDockWindow("Settings", settingsW);
}

void MainWindow::showAbout()
{
    About* about = new About();
    about->exec();
}

void MainWindow::addDockWindow(const QString& title, QWidget* widget, Qt::DockWidgetArea area)
{
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

    t_brush = new QButtonGroup();
    t_brush->setObjectName("t_brush");

    t_brush_circle = new QPushButton(QIcon(":/circle_icon"), "");
    t_brush_circle->setToolTip("Circle Brush");
    t_brush_circle->setCheckable(true);
    t_brush_circle->setChecked(true);
    t_brush_circle->setIconSize(QSize(24, 24));
    t_brush_circle->setObjectName("t_brush_circle");
    t_brush->addButton(t_brush_circle);

    t_brush_square = new QPushButton(QIcon(":/square_icon"), "");
    t_brush_square->setToolTip("Square Brush");
    t_brush_square->setCheckable(true);
    t_brush_square->setIconSize(QSize(24, 24));
    t_brush_square->setObjectName("t_brush_square");
    t_brush->addButton(t_brush_square);

    t_terrain_mode = new QComboBox();
    t_terrain_mode->setToolTip("Select terrain mode");
    t_terrain_mode->setObjectName("t_terrain_mode");
    t_terrain_mode->addItem("Shaping"  , 0);
    t_terrain_mode->addItem("Smoothing", 1);

    t_terrain_mode_0.append(qMakePair<QString, QVariant>("Linear"  , 1));
    t_terrain_mode_0.append(qMakePair<QString, QVariant>("Flat"    , 2));
    t_terrain_mode_0.append(qMakePair<QString, QVariant>("Smooth"  , 3));
    t_terrain_mode_0.append(qMakePair<QString, QVariant>("Unknown1", 4));
    t_terrain_mode_0.append(qMakePair<QString, QVariant>("Unknown2", 5));

    t_terrain_mode_1.append(qMakePair<QString, QVariant>("Linear"  , 1));
    t_terrain_mode_1.append(qMakePair<QString, QVariant>("Flat"    , 2));
    t_terrain_mode_1.append(qMakePair<QString, QVariant>("Smooth"  , 3));

    t_brush_type = new QComboBox();
    t_brush_type->setToolTip("Select brush type");
    t_brush_type->setObjectName("t_brush_type");

    for(int i = 0; i < t_terrain_mode_0.count(); ++i)
        t_brush_type->addItem(t_terrain_mode_0.at(i).first, t_terrain_mode_0.at(i).second);

    t_terrain_mode_label = new QLabel("Mode:");

    t_brush_label      = new QLabel("Brushes:");
    t_brush_type_label = new QLabel("Brush type:");

    t_radius_label       = new QLabel("Radius:");
    t_radius_value_label = new QLabel(QString("%1").arg(t_radius->value()));
    t_speed_label        = new QLabel("Speed:");
    t_speed_value_label  = new QLabel(QString("%1").arg(t_speed->value()));

    t_terrain_mode_label->setObjectName("t_terrain_mode_label");
    t_brush_label->setObjectName("t_brush_label");
    t_brush_type_label->setObjectName("t_brush_type_label");
    t_radius_label->setObjectName("t_radius_label");
    t_radius_value_label->setObjectName("t_radius_value_label");
    t_speed_label->setObjectName("t_speed_label");
    t_speed_value_label->setObjectName("t_speed_value_label");

    t_terrain_mode_label->setStyleSheet("margin:-3px 5px 0 0;");

    t_brush_label->setStyleSheet("margin:-3px 5px 0 0;");
    t_brush_type_label->setStyleSheet("margin:-3px 5px 0 0;");

    t_radius_label->setStyleSheet("margin:-3px 5px 0 20px;");
    t_speed_label->setStyleSheet("margin:-3px 5px 0 20px;");

    t_radius_value_label->setStyleSheet("margin:-3px 0 0 5px;");
    t_speed_value_label->setStyleSheet("margin:-3px 0 0 5px;");

    /// mode1
    addToolbarAction(t_brush_label       , mode1Actions);
    addToolbarAction(t_brush_circle      , mode1Actions);
    addToolbarAction(t_brush_square      , mode1Actions);
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

void MainWindow::addToolbarAction(QWidget* widget, QList<QString>& parentList)
{
    ui->toolbar3->addWidget(widget);
    ui->toolbar3->actions().last()->setObjectName(widget->objectName());

    parentList.append(widget->objectName());
}

void MainWindow::hideToolbarActions()
{
    foreach(QAction* action, ui->toolbar3->actions())
        action->setVisible(false);
}
