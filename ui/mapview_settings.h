#ifndef MAPVIEW_SETTINGS_H
#define MAPVIEW_SETTINGS_H

#include <QDialog>
#include <QColorDialog>
#include <QColor>

namespace Ui
{
    class MapView_Settings;
}

class MapView_Settings : public QDialog
{
    Q_OBJECT
    
public:
    explicit MapView_Settings(QColor brushColor, QWidget* parent = 0);
    ~MapView_Settings();
    
private:
    Ui::MapView_Settings* ui;

    QColorDialog* colorDialog;

    QColor cacheBrushColor;

    void initializeComponents();

private slots:
    void showColorDialog();

    void setCacheColor(QColor color);
    void setBrushColor(QColor color);
    void setBrushToCacheColor();

    void setEnvironmentDistance(int posVal);

signals:
    void setColorOfBrush(QColor* color);
    void setEnvironmentDistance(float value);
};

#endif // MAPVIEW_SETTINGS_H
