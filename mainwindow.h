#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

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

signals:
    void setSpeedMultiplier(float multiplier);
    void setDisplayMode(int mode);
    void setModeEditing(int option);

private slots:
    void setSpeedMultiplier();
    void setDisplayMode();
    void setToolBarItem();

    void showTeleport();
};

#endif // MAINWINDOW_H
