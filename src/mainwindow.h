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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDockWidget>
#include <QSlider>
#include <QButtonGroup>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QLabel>

#include "ui/qdslider.h"
#include "ui/teleportwidget.h"
#include "ui/mapview_settings.h"
#include "ui/texturepicker.h"
#include "ui/modelpicker.h"
#include "ui/startup.h"
#include "ui/waterwidget.h"
#include "ui/camerawidget.h"

#include "world.h"
#include "mapview.h"

namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget* parent = 0);
    ~MainWindow();
    
private:
    QString getActionName(QObject* object) const;

    Ui::MainWindow* ui;

    QString DisplayMode;
    QString ToolBarItem;

    TeleportWidget*   teleportW;
    MapView_Settings* settingsW;
    TexturePicker*    texturepW;
    ModelPicker*      modelpickerW;

    void addDockWindow(const QString& title, QWidget* widget, Qt::DockWidgetArea area = Qt::RightDockWidgetArea);

    template<class T>
    void deleteObject(T object)
    {
        if(object != NULL)
            delete object;
    }

    Qt::WindowStates nonFullScreenState;

    // Editing mode = 1
    QDSlider* t_outer_radius;
    QDSlider* t_inner_radius;
    QDSlider* t_speed;

    QButtonGroup* t_brush;

    QPushButton* t_brush_circle;
    QPushButton* t_brush_square;

    QComboBox* t_terrain_mode;
    QComboBox* t_brush_type;
    QComboBox* t_brush_mode;

    QCheckBox* t_terrain_maxHeight;

    QDoubleSpinBox* t_terrain_maximum_height;

    QLabel* t_terrain_mode_label;

    QLabel* t_brush_label;
    QLabel* t_brush_type_label;
    QLabel* t_brush_mode_label;

    QLabel* t_outer_radius_label;
    QLabel* t_outer_radius_value_label;
    QLabel* t_inner_radius_label;
    QLabel* t_inner_radius_value_label;
    QLabel* t_speed_label;
    QLabel* t_speed_value_label;

    QList<QPair<QString, QVariant>> t_terrain_mode_0;
    QList<QPair<QString, QVariant>> t_terrain_mode_1;

    // Editing mode = 2
    QDSlider* t_flow;

    QCheckBox* t_paint_maxAlpha;

    QDoubleSpinBox* t_paint_maximum_alpha;

    QLabel* t_flow_label;
    QLabel* t_flow_value_label;

    // Editing mode = 3
    QColorDialog* colorW;

    // Editing mode = 4
    WaterWidget* waterW;

    // Editing mode = 5
    QDSlider* t_rotationx;
    QDSlider* t_rotationy;
    QDSlider* t_rotationz;
    QDSlider* t_impend;
    QDSlider* t_scale;

    QButtonGroup* t_modelmode;

    QPushButton* t_modelmode_insert;
    QPushButton* t_modelmode_delete;
    QPushButton* t_reset_transform;

    QLabel* t_model_mode_label;
    QLabel* t_rotationx_label;
    QLabel* t_rotationx_value_label;
    QLabel* t_rotationy_label;
    QLabel* t_rotationy_value_label;
    QLabel* t_rotationz_label;
    QLabel* t_rotationz_value_label;
    QLabel* t_impend_label;
    QLabel* t_impend_value_label;
    QLabel* t_scale_label;
    QLabel* t_scale_value_label;

    // Editing mode = 6
    CameraWidget* cameraW;

    //
    QList<QString> mode0Actions;
    QList<QString> mode1Actions;
    QList<QString> mode2Actions;
    QList<QString> mode3Actions;
    QList<QString> mode4Actions;
    QList<QString> mode5Actions;
    QList<QString> mode6Actions;

    void initMode();
    void showMode(QList<QString>& parentList);

    void addToolbarAction(QWidget* widget, QList<QString>& parentList);
    void hideToolbarActions();

    //
    StartUp* startUp;

    World*   world;
    MapView* mapView;

    bool** mapCoords;

signals:
    void setSpeedMultiplier(float multiplier);
    void setDisplayMode(int mode);
    void setModeEditing(int option);
    void setTerrainMode(int mode);
    void setModelMode(int mode);

private slots:
    void setSpeedMultiplier();
    void setDisplayMode();
    void setToolBarItem();
    void setFullscreen(bool checked);

    void setTerrainMaximumHeightState(int state);
    void setPaintMaximumAlphaState(int state);

    void setBrush(int unknown, bool unknown2);
    void setBrushType(int type);
    void setBrushMode(MapView::eMouseMode& mouseMode, MapView::eEditingMode& editingMode);

    void setBrushOuterRadius(double value);
    void setBrushInnerRadius(double value);
    void setBrushInnerRadiusMaximumValue(double maximum);

    void resetModelBrush();

    void setTerrain_Mode(int index);
    void setModel_Mode(int unknown, bool unknown2);

    void setProjectData(ProjectFileData& data);

    void setVertexShadingSwitch(bool state);

    void showTeleport();
    void showSettings();
    void showTexturePicker();
    void showAbout();
    void showProjectSettings();

    void openWorld(ProjectFileData projectData);
    void postInitializeSubWorldWidgets();

    void createMemoryProject(NewProjectData projectData);

    void loadNewProjectMapTilesIntoMemory();

    void newProject();
    void closeProject();

    void takeScreenshot();
    void takeScreenshotAndUpload();

    void screenshotUploadDone(const QString& link);
    void screenshotUploadError(const QString& error);

    void actionIsInDevelopment();
};

#endif // MAINWINDOW_H