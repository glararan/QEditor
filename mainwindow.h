#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDockWidget>
#include <QSlider>
#include <QButtonGroup>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>

#include "ui/qdslider.h"
#include "ui/teleportwidget.h"

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

    TeleportWidget* teleportW;

    void addDockWindow(const QString& title, QWidget* widget, Qt::DockWidgetArea area = Qt::RightDockWidgetArea);

    // Editing mode = 1
    QDSlider* t_radius;
    QDSlider* t_speed;

    QButtonGroup* t_brush;

    QPushButton* t_brush_circle;
    QPushButton* t_brush_square;

    QComboBox* t_terrain_mode;
    QComboBox* t_brush_type;

    QLabel* t_terrain_mode_label;

    QLabel* t_brush_label;
    QLabel* t_brush_type_label;

    QLabel* t_radius_label;
    QLabel* t_radius_value_label;
    QLabel* t_speed_label;
    QLabel* t_speed_value_label;

    QList<QString> mode0Actions;
    QList<QString> mode1Actions;

    QList<QPair<QString, QVariant>> t_terrain_mode_0;
    QList<QPair<QString, QVariant>> t_terrain_mode_1;

    void initMode();
    void showMode(QList<QString>& parentList);

    void addToolbarAction(QWidget* widget, QList<QString>& parentList);
    void hideToolbarActions();

signals:
    void setSpeedMultiplier(float multiplier);
    void setDisplayMode(int mode);
    void setModeEditing(int option);
    void setTerrainMode(int mode);

private slots:
    void setSpeedMultiplier();
    void setDisplayMode();
    void setToolBarItem();

    void setShapingRadius(double value);

    void setTerrain_Mode(int index);

    void showTeleport();
};

#endif // MAINWINDOW_H
