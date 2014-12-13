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
    explicit IntroPage(QWidget* parent = 0);

private:
    QLabel* label;
};

class ProjectInfoPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit ProjectInfoPage(QWidget* parent = 0);

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
    explicit ProjectMapPage(QWidget* parent = 0);

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

    bool** mapCoords;
};

class FinishPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit FinishPage(QWidget* parent = 0);

protected:
    void initializePage();

private:
    QLabel* label;
};

class NewProject : public QWizard
{
    Q_OBJECT

public:
    explicit NewProject(QWidget* parent = 0);

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