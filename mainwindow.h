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
#include "ui/startup.h"

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

    void addDockWindow(const QString& title, QWidget* widget, Qt::DockWidgetArea area = Qt::RightDockWidgetArea);

    template<class T>
    void deleteObject(T object)
    {
        if(object != NULL)
            delete object;
    }

    // Editing mode = 1
    QDSlider* t_outer_radius;
    QDSlider* t_inner_radius;
    QDSlider* t_speed;

    QButtonGroup* t_brush;

    QPushButton* t_brush_circle;
    QPushButton* t_brush_square;

    QComboBox* t_terrain_mode;
    QComboBox* t_brush_type;

    QCheckBox* t_terrain_maxHeight;

    QDoubleSpinBox* t_terrain_maximum_height;

    QLabel* t_terrain_mode_label;

    QLabel* t_brush_label;
    QLabel* t_brush_type_label;

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

    QLabel* t_flow_label;
    QLabel* t_flow_value_label;

    // Editing mode = 3
    QColorDialog* colorW;

    //
    QList<QString> mode0Actions;
    QList<QString> mode1Actions;
    QList<QString> mode2Actions;
    QList<QString> mode3Actions;

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

private slots:
    void setSpeedMultiplier();
    void setDisplayMode();
    void setToolBarItem();

    void setTerrainMaximumHeightState(int state);

    void setBrushOuterRadius(double value);
    void setBrushInnerRadius(double value);
    void setBrushInnerRadiusMaximumValue(double maximum);

    void setTerrain_Mode(int index);

    void setProjectData(ProjectFileData& data);

    void showTeleport();
    void showSettings();
    void showTexturePicker();
    void showAbout();
    void showProjectSettings();

    void openWorld(ProjectFileData projectData);
    void postInitializeSubWorldWidgets();

    void createMemoryProject(NewProjectData projectData);

    void loadNewProjectMapTilesIntoMemory();

    void takeScreenshot();
};

#endif // MAINWINDOW_H