#include "newproject.h"

#include "mathhelper.h"

#include <QVBoxLayout>
#include <QTableWidget>
#include <QMessageBox>
#include <QGraphicsSceneMouseEvent>
#include <QSpacerItem>
#include <QFileDialog>

NewProject::NewProject(QWidget* parent)
: QWizard(parent)
, introPage(new IntroPage)
, projectInfoPage(new ProjectInfoPage)
, projectMapPage(new ProjectMapPage)
, finishPage(new FinishPage)
{
    addPage(introPage);
    addPage(projectInfoPage);
    addPage(projectMapPage);
    addPage(finishPage);

    setWindowTitle(tr("New Project"));
}

void NewProject::accept()
{
    NewProjectData data;
    data.projectFile = field("projectFileName").toString();
    data.projectDir  = field("projectDir").toString();
    data.projectName = field("projectName").toString();
    data.mapName     = field("mapName").toString();
    data.mapCoords   = projectMapPage->MapCoords();

    emit wizardData(data);

    QDialog::accept();
}

IntroPage::IntroPage(QWidget* parent) : QWizardPage(parent)
{
    setTitle(tr("New Project - Introduction"));

    label = new QLabel(tr("This wizard will generate a new QEditor project,"
                          "including a few settings. Also you simply need to "
                          "specify how many Map Tiles you will use in project."));
    label->setWordWrap(true);

    QVBoxLayout* layout = new QVBoxLayout();
    layout->addWidget(label);

    setLayout(layout);
}

ProjectInfoPage::ProjectInfoPage(QWidget* parent) : QWizardPage(parent)
{
    setTitle(tr("New Project - Settings"));
    setSubTitle(tr("Specify basic information about the project for which you "
                   "want to generate map files."));

    projectFileNameLineEdit = new QLineEdit();
    projectFileNameLabel    = new QLabel(tr("&Project file name:"));
    projectFileNameLabel->setBuddy(projectFileNameLineEdit);

    projectDirLineEdit = new QLineEdit();
    projectDirLabel    = new QLabel(tr("&Project root directory:"));
    projectDirLabel->setBuddy(projectDirLineEdit);

    chooseProjectDir = new QPushButton(tr("&Choose"));

    connect(chooseProjectDir, SIGNAL(clicked()), this, SLOT(projectDir()));

    projectNameLineEdit = new QLineEdit();
    projectNameLabel    = new QLabel(tr("&Project name:"));
    projectNameLabel->setBuddy(projectNameLineEdit);

    mapNameLineEdit = new QLineEdit();
    mapNameLabel    = new QLabel(tr("&Map name:"));
    mapNameLabel->setBuddy(mapNameLineEdit);

    registerField("projectFileName*", projectFileNameLineEdit);
    registerField("projectDir*",      projectDirLineEdit);
    registerField("projectName*",     projectNameLineEdit);
    registerField("mapName*",         mapNameLineEdit);

    QGridLayout* layout = new QGridLayout();
    layout->addWidget(projectFileNameLabel,    0, 0);
    layout->addWidget(projectFileNameLineEdit, 0, 1, 1, 2);
    layout->addWidget(projectDirLabel,         1, 0);
    layout->addWidget(projectDirLineEdit,      1, 1);
    layout->addWidget(chooseProjectDir,        1, 2);
    layout->addWidget(projectNameLabel,        2, 0);
    layout->addWidget(projectNameLineEdit,     2, 1, 1, 2);
    layout->addWidget(mapNameLabel,            3, 0);
    layout->addWidget(mapNameLineEdit,         3, 1, 1, 2);

    setLayout(layout);
}

void ProjectInfoPage::projectDir()
{
    projectDirLineEdit->setText(QFileDialog::getExistingDirectory(this, tr("Choose project root directory"), QString(), QFileDialog::ShowDirsOnly |
                                                                                                                        QFileDialog::DontResolveSymlinks));
}

bool ProjectInfoPage::validatePage()
{
    if(projectNameLineEdit->text().isEmpty())
    {
        QMessageBox msg;
        msg.setText(tr("You didn't enter the project name!"));
        msg.setWindowTitle("Error");

        msg.exec();

        return false;
    }

    if(mapNameLineEdit->text().isEmpty())
    {
        QMessageBox msg;
        msg.setText(tr("You didn't enter the map name!"));
        msg.setWindowTitle("Error");

        msg.exec();

        return false;
    }

    return true;
}

MapGraphicsView::MapGraphicsView() : QGraphicsView()
{

}

void MapGraphicsView::mouseMoveEvent(QMouseEvent* e)
{
    emit mousePos(e->x(), e->y());
    emit mouseMoved();
}

MapGraphicsScene::MapGraphicsScene() : QGraphicsScene(), pressed(false)
{
}

void MapGraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent* e)
{
    e->accept();

    pressed = true;
}

void MapGraphicsScene::mouseReleaseEvent(QGraphicsSceneMouseEvent* e)
{
    e->accept();

    if(pressed)
        pressed = false;
}

void MapGraphicsScene::click()
{
    if(pressed)
        emit mouseIsPressed();
}

ProjectMapPage::ProjectMapPage(QWidget* parent) : QWizardPage(parent)
{
    setTitle(tr("New Project - Map Tiles"));

    checkAll   = new QPushButton(tr("Check all"));
    uncheckAll = new QPushButton(tr("Uncheck all"));

    connect(checkAll  , SIGNAL(clicked()), this, SLOT(checkAllBoxs()));
    connect(uncheckAll, SIGNAL(clicked()), this, SLOT(uncheckAllBoxs()));

    gps = new QLabel();

    cursorPosX = 0;
    cursorPosY = 0;

    mapCoords = new bool*[TILES];

    for(int x = 0; x < TILES; ++x)
    {
        mapCoords[x] = new bool[TILES];

        for(int y = 0; y < TILES; ++y)
            mapCoords[x][y] = false;
    }

    mapGView  = new MapGraphicsView();
    mapGScene = new MapGraphicsScene();

    mapGView->setScene(mapGScene);
    mapGView->setFixedSize(TILES * sqrt((double)TILES) + 5, TILES * sqrt((double)TILES) + 5);
    mapGView->setMouseTracking(true);

    applyBrush();

    connect(mapGView,  SIGNAL(mousePos(int, int)), this,      SLOT(refreshGPS(int, int)));
    connect(mapGView,  SIGNAL(mouseMoved()),       mapGScene, SLOT(click()));
    connect(mapGScene, SIGNAL(mouseIsPressed()),   this,      SLOT(applyBrush()));

    QSpacerItem* hSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Ignored);

    QGridLayout* layout = new QGridLayout();
    layout->addWidget(checkAll,   0, 0);
    layout->addWidget(uncheckAll, 0, 1);
    layout->addItem(hSpacer,      0, 2);
    layout->addWidget(gps,        1, 0);
    layout->addWidget(mapGView,   2, 0, 2, 3);

    setLayout(layout);
}

void ProjectMapPage::checkAllBoxs()
{
    for(int y = 0; y < TILES; ++y)
    {
        for(int x = 0; x < TILES; ++x)
        {
            if(!mapCoords[x][y])
                mapCoords[x][y] = true;
        }
    }

    applyBrush();
}

void ProjectMapPage::uncheckAllBoxs()
{
    for(int y = 0; y < TILES; ++y)
    {
        for(int x = 0; x < TILES; ++x)
        {
            if(mapCoords[x][y])
                mapCoords[x][y] = false;
        }
    }

    applyBrush();
}

void ProjectMapPage::refreshGPS(int x, int y)
{
    cursorPosX = x / sqrt((double)TILES);
    cursorPosY = y / sqrt((double)TILES);

    if(cursorPosX < TILES && cursorPosX >= 0 && cursorPosY < TILES && cursorPosY >= 0)
        gps->setText(QString("X: %1, Y: %2").arg(cursorPosX).arg(cursorPosY));
}

void ProjectMapPage::applyBrush()
{
    if(cursorPosX < TILES && cursorPosX >= 0 && cursorPosY < TILES && cursorPosY >= 0)
        mapCoords[cursorPosX][cursorPosY] = !mapCoords[cursorPosX][cursorPosY];

    // draw
    mapGScene->clear();

    QPen pen(QColor(50, 50, 50), 1, Qt::SolidLine);

    QBrush brushEmpty(QColor(255, 255, 255));
    QBrush brushSelected(QColor(171, 131, 61));

    for(int x = 0; x < TILES; ++x)
    {
        for(int y = 0; y < TILES; ++y)
        {
            QRect tile(x * sqrt((double)TILES), y * sqrt((double)TILES), sqrt((double)TILES), sqrt((double)TILES));

            if(mapCoords[x][y])
                mapGScene->addRect(tile, pen, brushSelected);
            else
                mapGScene->addRect(tile, pen, brushEmpty);
        }
    }
}

void ProjectMapPage::initializePage()
{
    projectName = field("projectName").toString();
    mapName     = field("mapName").toString();

    setSubTitle(tr("Select how many Map Tiles for project '%1' you will use. "
                   "For each Map Tile will be created file '%2_x_y.map'.").arg(projectName).arg(mapName));
}

bool ProjectMapPage::validatePage()
{
    bool allow = false;

    for(int x = 0; x < TILES; ++x)
    {
        for(int y = 0; y < TILES; ++y)
        {
            if(mapCoords[x][y])
            {
                allow = true;

                break;
            }
        }

        if(allow) // make validate faster
            break;
    }

    if(!allow)
    {
        QMessageBox msg;
        msg.setText(tr("You didn't choose any coordinate on map. You must select at least one to create first Map Tile!"));
        msg.setWindowTitle("Error");

        msg.exec();
    }

    return allow;
}

FinishPage::FinishPage(QWidget* parent) : QWizardPage(parent)
{
    setWindowTitle(tr("New Project - Finish"));

    label = new QLabel();
    label->setWordWrap(true);

    QVBoxLayout* layout = new QVBoxLayout();
    layout->addWidget(label);

    setLayout(layout);
}

void FinishPage::initializePage()
{
    QString finishText = wizard()->buttonText(QWizard::FinishButton);
    finishText.remove("&");

    label->setText(tr("Click %1 to generate project files.").arg(finishText));
}
