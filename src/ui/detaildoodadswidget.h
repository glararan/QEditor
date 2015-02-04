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

#ifndef DETAILDOODADSWIDGET_H
#define DETAILDOODADSWIDGET_H

#include <QDialog>
#include <QLineEdit>
#include <QMap>

class MapTile;
class TextureManager;

namespace Ui
{
    class DetailDoodadsWidget;
}

class ObjectLine : public QLineEdit
{
public:
    explicit ObjectLine(QWidget* parent);
    ~ObjectLine();

protected:
    void dragEnterEvent(QDragEnterEvent* event);
    void dragMoveEvent(QDragMoveEvent* event);
    void dropEvent(QDropEvent* event);
};

class DetailDoodadsWidget : public QDialog
{
    Q_OBJECT

public:
    explicit DetailDoodadsWidget(QWidget* parent = 0);
    ~DetailDoodadsWidget();

    void initialize(TextureManager* manager);

private:
    Ui::DetailDoodadsWidget* ui;

    ObjectLine* objectLine;

    TextureManager* textureManager;

    MapTile* mapTile;

    QMap<int, QVector<QPair<QString, float>>> data;

public slots:
    void setTile(MapTile* tile);

private slots:
    void addObject();
    void deleteObject();
    void spawnObjects();
    void clearObjects();

    void updateObjectList(int row);
};

#endif // DETAILDOODADSWIDGET_H
