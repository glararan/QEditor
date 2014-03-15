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

#ifndef WATERWIDGET_H
#define WATERWIDGET_H

#include <QDialog>

#include "waterchunk.h"

namespace Ui
{
    class WaterWidget;
}

class WaterWidget : public QDialog
{
    Q_OBJECT

public:
    explicit WaterWidget(QWidget* parent = 0);
    ~WaterWidget();

private:
    Ui::WaterWidget* ui;

    WaterChunk* waterChunk;

private slots:
    void setWaterStatus(int status);

    void setTopHeight(double value);
    void setRightHeight(double value);
    void setBottomHeight(double value);
    void setLeftHeight(double value);

    void updateWater();

public slots:
    void setChunk(WaterChunk* chunk);
};

#endif // WATERWIDGET_H
