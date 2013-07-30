#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

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

    QString ToolBarItem;

signals:
    void setSpeedMultiplier(float multiplier);
    void setModeEditing(int option);

private slots:
    void setSpeedMultiplier();
    void setToolBarItem();
};

#endif // MAINWINDOW_H
