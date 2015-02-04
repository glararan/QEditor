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

#ifndef MAPGENERATION_H
#define MAPGENERATION_H

#include <QDialog>

#include "mapheaders.h"

namespace Ui
{
    class MapGeneration;
}

class MapGeneration : public QDialog
{
    Q_OBJECT

public:
    explicit MapGeneration(QWidget* parent = 0);
    ~MapGeneration();

private:
    Ui::MapGeneration* ui;

    MapGenerationData data;

private slots:
    void regenerate();

    void setSeed(int value);
    void setPerlinNoiseLevel(QString level);
    void setPerlinNoiseHeight(int height);
    void setPerlinNoiseMultiple(int multiple);
    void setPerturbFrequency(QString frequency);
    void setPerturbD(QString d);
    void setErodeSmoothLevel(int level);
    void setErode(QString erode);

signals:
    void generate(MapGenerationData& data);
};

#endif // MAPGENERATION_H
