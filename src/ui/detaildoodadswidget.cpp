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

#include "detaildoodadswidget.h"
#include "ui_detaildoodadswidget.h"

#include "texturemanager.h"
#include "maptile.h"

DetailDoodadsWidget::DetailDoodadsWidget(QWidget* parent)
: QDialog(parent)
, ui(new Ui::DetailDoodadsWidget)
, textureManager(NULL)
, mapTile(NULL)
, objectLine(new ObjectLine(this))
{
    ui->setupUi(this);

    objectLine->setReadOnly(true);

    ui->gridLayout->addWidget(objectLine, 1, 0, 1, 3); // add ui object line

    connect(ui->addButton,    SIGNAL(clicked()), this, SLOT(addObject()));
    connect(ui->deleteButton, SIGNAL(clicked()), this, SLOT(deleteObject()));

    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(spawnObjects()));
    connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(clearObjects()));

    connect(ui->objectList, SIGNAL(currentRowChanged(int)), this, SLOT(updateObjectList(int)));
}

DetailDoodadsWidget::~DetailDoodadsWidget()
{
    delete ui;
}

void DetailDoodadsWidget::initialize(TextureManager* manager)
{
    textureManager = manager;

    int comboBoxIndex = 0;

    for(int i = 0; i < textureManager->getTextures().count(); ++i)
    {
        QPair<QString, TexturePtr> pair = textureManager->getTextures().at(i);

        if(pair.first.endsWith("ModelTexture", Qt::CaseSensitive))
            continue;

        ui->textureBox->addItem(QIcon(pair.second->getPath()), QString("%1 : 0").arg(pair.second->getPath()), pair.second->getPath());

        data.insert(comboBoxIndex++, QVector<QPair<QString, float>>());
    }
}

void DetailDoodadsWidget::setTile(MapTile* tile)
{
    mapTile = tile;
}

void DetailDoodadsWidget::addObject()
{
    if(objectLine->text().isEmpty())
    {
        QMessageBox msg;
        msg.setWindowTitle("Error");
        msg.setIcon(QMessageBox::Critical);
        msg.setText(tr("Object path is empty! Drag and drop model to object path line."));
        msg.exec();

        return;
    }

    int textureIndex = ui->textureBox->currentIndex();

    // check if there is already exists that object
    for(int i = 0; i < data[textureIndex].count(); ++i)
    {
        QPair<QString, float> pair = data[textureIndex][i];

        if(pair.first == objectLine->text())
        {
            QMessageBox msg;
            msg.setWindowTitle("Error");
            msg.setIcon(QMessageBox::Critical);
            msg.setText("Object is already on list!");
            msg.exec();

            return;
        }
    }

    data[textureIndex].append(QPair<QString, float>(objectLine->text(), MathHelper::toFloat(ui->radiusBox->value())));

    ui->textureBox->setItemText(textureIndex, QString("%1 : %2").arg(ui->textureBox->itemData(textureIndex).toString()).arg(data[textureIndex].count()));

    updateObjectList(textureIndex);
}

void DetailDoodadsWidget::deleteObject()
{
    if(ui->objectList->currentRow() < 0)
        return;

    int textureIndex = ui->textureBox->currentIndex();

    data[textureIndex].removeAt(ui->objectList->currentRow());

    ui->textureBox->setItemText(textureIndex, QString("%1 : %2").arg(ui->textureBox->itemData(textureIndex).toString()).arg(data[textureIndex].count()));

    updateObjectList(textureIndex);
}

void DetailDoodadsWidget::spawnObjects()
{
    // tile do spawn
    QVector<QString> textures;

    for(int i = 0; i < ui->textureBox->count(); ++i)
        textures.append(ui->textureBox->itemData(i).toString());

    if(!mapTile)
    {
        QMessageBox msg;
        msg.setWindowTitle("Error");
        msg.setText(tr("Map Tile was not selected! We couldn't spawn objects."));
        msg.setIcon(QMessageBox::Critical);

        msg.exec();

        return;
    }

    mapTile->spawnDetailDoodads(textures, data);
}

void DetailDoodadsWidget::clearObjects()
{
    objectLine->clear();
    ui->radiusBox->clear();
    ui->objectList->clear();

    QList<int> keys = data.keys();

    for(int i = 0; i < keys.count(); ++i)
        data[keys.at(i)].clear();

    for(int i = 0; i < ui->textureBox->count(); ++i)
        ui->textureBox->setItemText(i, QString("%1 : 0").arg(ui->textureBox->itemData(i).toString()));
}

void DetailDoodadsWidget::updateObjectList(int row)
{
    ui->objectList->clear();

    for(int i = 0; i < data[row].count(); ++i)
        ui->objectList->addItem(QString("%1 : %2").arg(data[row][i].first).arg(data[row][i].second));
}

ObjectLine::ObjectLine(QWidget* parent) : QLineEdit(parent)
{
    setAcceptDrops(true);
    setFocusPolicy(Qt::StrongFocus);
}

ObjectLine::~ObjectLine()
{
}

void ObjectLine::dragEnterEvent(QDragEnterEvent* event)
{
    if(event->mimeData()->hasFormat("text/plain"))
        event->acceptProposedAction();
}

void ObjectLine::dragMoveEvent(QDragMoveEvent* event)
{
    event->acceptProposedAction();
}

void ObjectLine::dropEvent(QDropEvent* event)
{
    if(event->mimeData()->hasText())
    {
        const QMimeData* data = event->mimeData();

        if(data->text() != QString())
            setText(data->text());

        if(event->source() == this)
        {
            event->setDropAction(Qt::MoveAction);
            event->accept();
        }
        else
            event->acceptProposedAction();
    }
    else
        event->ignore();
}
