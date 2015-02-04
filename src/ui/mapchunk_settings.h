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

#ifndef MAPCHUNK_SETTINGS_H
#define MAPCHUNK_SETTINGS_H

#include <QDialog>

#include "ui/qdslider.h"

class MapChunk;

namespace Ui
{
    class MapChunk_Settings;
}

class MapChunk_Settings : public QDialog
{
    Q_OBJECT

public:
    explicit MapChunk_Settings(QWidget* parent = 0);
    ~MapChunk_Settings();

private:
    Ui::MapChunk_Settings* ui;

    MapChunk* chunk;

    // Texture scale
    QDSlider* textureScaleFarSlider0;
    QDSlider* textureScaleNearSlider0;

    QDSlider* textureScaleFarSlider1;
    QDSlider* textureScaleNearSlider1;

    QDSlider* textureScaleFarSlider2;
    QDSlider* textureScaleNearSlider2;

    QDSlider* textureScaleFarSlider3;
    QDSlider* textureScaleNearSlider3;

private slots:
    void setTextureScaleOpt(int option);
    void setTextureScaleFar(double scale);
    void setTextureScaleNear(double scale);

    void setAutomaticStartEndEnabled(int state);
    void setAutomaticStartEndCorrection(double value);
    void setAutomaticEnd(double value);

public slots:
    void setChunk(MapChunk* mapChunk);
};

#endif // MAPCHUNK_SETTINGS_H
