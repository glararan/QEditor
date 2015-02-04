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

#ifndef MODELPICKER_H
#define MODELPICKER_H

#include <QWidget>
#include <QListWidget>

#include "model/modelmanager.h"

#include "ui/tableview.h"
#include "ui/toolbox.h"

namespace Ui
{
    class ModelPicker;
}

class FavouriteList : public QListWidget
{
public:
    explicit FavouriteList(QWidget* parent);
    ~FavouriteList();

protected:
    void dragEnterEvent(QDragEnterEvent* event);
    void dragMoveEvent(QDragMoveEvent* event);
    void dropEvent(QDropEvent* event);
};

class ModelPicker : public QWidget
{
    Q_OBJECT

public:
    explicit ModelPicker(QWidget* parent = 0);
    ~ModelPicker();

    void loadPicker(ModelManager* manager);
    void clear();

private:
    Ui::ModelPicker* ui;

    QString currentModelLocation;

    ToolBox* box;

    QVector<TextureWell*> items;

    FavouriteList* favouriteList;

    ModelManager* manager;

private slots:
    void modelSelected(int row, int column);
    void modelSelected(int row);
    void modelviewSelected(int, int);

    void deleteModel();
    void clearModels();

public slots:
    void setPreviousModel();
    void setNextModel();

signals:
    void modelIsSelected();
};

#endif // MODELPICKER_H
