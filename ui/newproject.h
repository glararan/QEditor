#ifndef NEWPROJECT_H
#define NEWPROJECT_H

#include <QWizard>
#include <QGraphicsView>
#include <QGraphicsScene>

#include "mapheaders.h"

#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QMouseEvent>

class IntroPage : public QWizardPage
{
    Q_OBJECT

public:
    IntroPage(QWidget* parent = 0);

private:
    QLabel* label;
};

class ProjectInfoPage : public QWizardPage
{
    Q_OBJECT

public:
    ProjectInfoPage(QWidget* parent = 0);

protected:
    bool validatePage();

private slots:
    void projectDir();

private:
    QLabel* projectFileNameLabel;
    QLabel* projectDirLabel;
    QLabel* projectNameLabel;
    QLabel* mapNameLabel;

    QLineEdit* projectFileNameLineEdit;
    QLineEdit* projectDirLineEdit;
    QLineEdit* projectNameLineEdit;
    QLineEdit* mapNameLineEdit;

    QPushButton* chooseProjectDir;
};

class MapGraphicsView : public QGraphicsView
{
    Q_OBJECT

public:
    MapGraphicsView();

signals:
    void mousePos(int x, int y);
    void mouseMoved();

protected:
    void mouseMoveEvent(QMouseEvent* e);
};

class MapGraphicsScene : public QGraphicsScene
{
    Q_OBJECT

public:
    MapGraphicsScene();

public slots:
    void click();

signals:
    void mouseIsPressed();

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* e);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* e);

private:
    bool pressed;
};

class ProjectMapPage : public QWizardPage
{
    Q_OBJECT

public:
    ProjectMapPage(QWidget* parent = 0);

    bool** MapCoords() const { return mapCoords; }

protected:
    void initializePage();

    bool validatePage();

private slots:
    void checkAllBoxs();
    void uncheckAllBoxs();

    void refreshGPS(int x, int y);
    void applyBrush();

private:
    QString projectName;
    QString mapName;

    QPushButton* checkAll;
    QPushButton* uncheckAll;

    QLabel* gps;

    MapGraphicsView*  mapGView;
    MapGraphicsScene* mapGScene;

    int cursorPosX, cursorPosY;

    //bool mapCoords[TILES][TILES];
    bool** mapCoords;
};

class FinishPage : public QWizardPage
{
    Q_OBJECT

public:
    FinishPage(QWidget* parent = 0);

protected:
    void initializePage();

private:
    QLabel* label;
};

class NewProject : public QWizard
{
    Q_OBJECT

public:
    NewProject(QWidget* parent = 0);

    void accept();

signals:
    void wizardData(NewProjectData projectData);

private:
    IntroPage*       introPage;
    ProjectInfoPage* projectInfoPage;
    ProjectMapPage*  projectMapPage;
    FinishPage*      finishPage;
};

#endif // NEWPROJECT_H
