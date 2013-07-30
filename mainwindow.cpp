#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "terraintessellationscene.h"

#include <QAction>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow)
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
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setSpeedMultiplier()
{
    float multiplier = 1.0f;

    QAction* action = qobject_cast<QAction*>(this->sender());
    QString sName   = action->objectName();

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
