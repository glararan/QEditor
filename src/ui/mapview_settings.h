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
    enum BrushColor
    {
        Inner = 0,
        Outer = 1
    };

    // Wireframe color
    enum WireframeColor
    {
        Wireframe        = 0,
        TerrainWireframe = 1
    };

    BrushColor     brushColorType;
    WireframeColor wireframeColorType;

    QColorDialog* colorDialog;
    QColorDialog* wireframeColorDialog;

    QColor cacheOuterBrushColor;
    QColor cacheInnerBrushColor;

    QColor cacheWireframeColor;
    QColor cacheTWireframeColor;

    // Texture scale
    QDSlider* textureScaleFarSlider;
    QDSlider* textureScaleNearSlider;

    void initializeComponents();

private slots:
    void showColorDialog();

    void setBrushCacheColor(QColor color);
    void setBrushColor(QColor color);
    void setBrushToCacheColor();

    void setWireframeCacheColor(QColor color);
    void setWireframeColor(QColor color);
    void setWireframeToCacheColor();

    void setEnvironmentDistance(int posVal);

    void setTextureScaleOpt(int option);
    void setTextureScaleFar(double scale);
    void setTextureScaleNear(double scale);

signals:
    void setColorOfBrush(QColor* color, bool outer);
    void setColorOfWireframe(QColor* color, bool terrain);

    void setEnvironmentDistance(float value);

    void setTextureScaleOption(int option);
    void setTextureScaleFar(float value);
    void setTextureScaleNear(float value);
};

#endif // MAPVIEW_SETTINGS_H