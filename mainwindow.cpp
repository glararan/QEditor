#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "mapview.h"

#include <QAction>

MainWindow::MainWindow(QWidget* parent)
: QMainWindow(parent)
, ui(new Ui::MainWindow)
, DisplayMode("action_Default")
, ToolBarItem("action_mapview_m0")
, teleportW(new TeleportWidget)
{
    ui->setupUi(this);

    // map view constructor
    MapView* mapView = new MapView(this);
    setCentralWidget(mapView);

    /// map view
    connect(mapView, SIGNAL(statusBar(QString)), ui->statusbar, SLOT(showMessage(QString)));

    /// menu bar
    // file
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

    // tools - teleport
    connect(ui->actionTeleport, SIGNAL(triggered()), this, SLOT(showTeleport()));

    connect(teleportW, SIGNAL(TeleportTo(QVector3D*)), mapView, SLOT(setCameraPosition(QVector3D*)));

    // tools - test
    connect(ui->action_Test, SIGNAL(triggered()), mapView, SLOT(doTest()));

    /// toolbar
    connect(ui->action_mapview_m0, SIGNAL(triggered()), this, SLOT(setToolBarItem()));
    connect(ui->action_mapview_m1, SIGNAL(triggered()), this, SLOT(setToolBarItem()));

    connect(this, SIGNAL(setModeEditing(int)), mapView, SLOT(setModeEditing(int)));
}

MainWindow::~MainWindow()
{
    delete ui;
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
        emit setModeEditing(0);
    else if(iName == "action_mapview_m1")
        emit setModeEditing(1);
}

void MainWindow::showTeleport()
{
    teleportW->exec();
    //teleportW->show();
    //teleportW->set
}
