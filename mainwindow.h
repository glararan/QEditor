#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDockWidget>
#include <QSlider>
#include <QButtonGroup>
#include <QPushButton>
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

    QLabel* t_brush_label;

    QLabel* t_radius_label;
    QLabel* t_radius_value_label;
    QLabel* t_speed_label;
    QLabel* t_speed_value_label;

    void initMode();

    void showMode0();
    void showMode1();

    void hideToolbarActions();

signals:
    void setSpeedMultiplier(float multiplier);
    void setDisplayMode(int mode);
    void setModeEditing(int option);

private slots:
    void setSpeedMultiplier();
    void setDisplayMode();
    void setToolBarItem();

    void setShapingRadius(double value);

    void showTeleport();
};

#endif // MAINWINDOW_H
