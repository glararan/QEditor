#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "mapview.h"

#include <QAction>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow), ToolBarItem("action_mapview_m0")
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
    connect(ui->actionSpeed_0, SIGNAL(triggered()), this, SLOT(setSpeedMultiplier()));
    connect(ui->actionSpeed_1, SIGNAL(triggered()), this, SLOT(setSpeedMultiplier()));
    connect(ui->actionSpeed_3, SIGNAL(triggered()), this, SLOT(setSpeedMultiplier()));
    connect(ui->actionSpeed_5, SIGNAL(triggered()), this, SLOT(setSpeedMultiplier()));
    connect(ui->actionSpeed_7, SIGNAL(triggered()), this, SLOT(setSpeedMultiplier()));
    connect(ui->actionSpeed_10, SIGNAL(triggered()), this, SLOT(setSpeedMultiplier()));
    connect(ui->actionSpeed_0_1, SIGNAL(triggered()), this, SLOT(setSpeedMultiplier()));
    connect(ui->actionSpeed_0_2, SIGNAL(triggered()), this, SLOT(setSpeedMultiplier()));

    connect(this, SIGNAL(setSpeedMultiplier(float)), mapView, SLOT(setSpeedMultiplier(float)));

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
