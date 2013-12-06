#ifndef MAPVIEW_SETTINGS_H
#define MAPVIEW_SETTINGS_H

#include <QDialog>
#include <QColorDialog>
#include <QColor>

#include "ui/qdslider.h"

namespace Ui
{
    class MapView_Settings;
}

class MapView_Settings : public QDialog
{
    Q_OBJECT
    
public:
    explicit MapView_Settings(QWidget* parent = 0);
    ~MapView_Settings();
    
private:
    Ui::MapView_Settings* ui;

    // Brush color
    QColorDialog* colorDialog;

    QColor cacheBrushColor;

    // Texture scale
    QDSlider* textureScaleFarSlider;
    QDSlider* textureScaleNearSlider;

    void initializeComponents();

private slots:
    void showColorDialog();

    void setCacheColor(QColor color);
    void setBrushColor(QColor color);
    void setBrushToCacheColor();

    void setEnvironmentDistance(int posVal);

    void setTextureScaleOpt(int option);
    void setTextureScaleFar(double scale);
    void setTextureScaleNear(double scale);

signals:
    void setColorOfBrush(QColor* color);
    void setEnvironmentDistance(float value);
    void setTextureScaleOption(int option);
    void setTextureScaleFar(float value);
    void setTextureScaleNear(float value);
};

#endif // MAPVIEW_SETTINGS_H
