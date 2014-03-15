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
#include "model/imodelmanager.h"
#include "ui/tableview.h"
#include "ui/toolbox.h"

namespace Ui
{
    class ModelPicker;
}

class ModelPicker : public QWidget
{
    Q_OBJECT

public:
    explicit ModelPicker(QWidget *parent = 0);
    ~ModelPicker();
    void loadPicker(IModelManager *manager);
    void clear();

private:
    Ui::ModelPicker *ui;
    QString currentModelLocation;
    ToolBox *box;
    QVector<TextureWell*> items;
    IModelManager *manager;

private slots:
    void modelSelected(int, int);
    void modelviewSelected(int, int);
};

#endif // MODELPICKER_H
