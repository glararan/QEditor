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

#include "modelpicker.h"
#include "ui_modelpicker.h"

ModelPicker::ModelPicker(QWidget* parent)
: QWidget(parent)
, ui(new Ui::ModelPicker)
{
    ui->setupUi(this);

    setMinimumHeight(200);

    box = new ToolBox();
    ui->horizontalLayout->insertWidget(0, box);

    hide();
}

ModelPicker::~ModelPicker()
{
    delete ui;
}

void ModelPicker::loadPicker(ModelManager* manager)
{
    this->manager = manager;

    clear();

    QVector<QString> categories = manager->getCategories();

    for(int i = 0; i < categories.size(); ++i)
    {
        TextureWell* well = new TextureWell(this, 0, 4, QSize(78, 78), QSize(0, 0));

        connect(well, SIGNAL(selected(int, int)), this, SLOT(modelSelected(int, int)));

        QVector<QString> names = manager->getNames(categories.at(i));

        for(int j = 0; j < names.size(); ++j)
        {
            ModelData* model = manager->getModel(manager->getIndex(names.at(j)));

            well->insertItem(model->image, names.at(j));
        }

        items.push_back(well);

        box->addItem(new ToolItem(categories.at(i),well));
    }

    manager->setCurrentModel(-1);
}

void ModelPicker::clear()
{
    for(int i = 0; i < items.size(); ++i)
        delete items.at(i);

    items.clear();
    box->clear();
    currentModelLocation.clear();
}

void ModelPicker::modelSelected(int row, int column)
{
    TextureWell* sender = static_cast<TextureWell*>(this->sender());

    if(sender)
    {
        currentModelLocation = sender->getToolTip(row, column);

        manager->setCurrentModel(manager->getIndex(currentModelLocation));
    }
}

void ModelPicker::modelviewSelected(int, int)
{
    TextureWell* sender = static_cast<TextureWell*>(this->sender());
}
