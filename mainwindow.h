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
    Ui::MainWindow* ui;

signals:
    void setSpeedMultiplier(float multiplier);

private slots:
    void setSpeedMultiplier();
};

#endif // MAINWINDOW_H
