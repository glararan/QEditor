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

#ifndef TEXTUREPICKER_H
#define TEXTUREPICKER_H

#include <QDialog>

#include "tableview.h"
#include "texturemanager.h"

#include <QVector>
#include <QPair>
#include <QImage>
#include <QTableWidgetItem>

class MapChunk;

namespace Ui
{
    class TexturePicker;
}

class TexturePicker : public QDialog
{
    Q_OBJECT
    
public:
    explicit TexturePicker(QWidget* parent = 0);
    ~TexturePicker();

    void initialize(TextureManager* manager);
    
protected:
    void resizeEvent(QResizeEvent* e);

private:
    Ui::TexturePicker* ui;

    TextureManager* textureManager;

    TextureWell* textureWell;

    MapChunk* mapChunk;

    QSize textureIconSize;
    QSize textureIconMargin;

    int columns;

private slots:
    void selectedTexture(int row, int cell);

    void moveLayerUp();
    void moveLayerDown();
    void deleteLayer();
    void loadAllTextures();

    void handleLayerGUI(QTableWidgetItem* item);

public slots:
    void setChunk(MapChunk* chunk);
};

#endif // TEXTUREPICKER_H