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

#include "texturepicker.h"
#include "ui_texturepicker.h"

#include "mathhelper.h"
#include "mapchunk.h"

TexturePicker::TexturePicker(QWidget* parent)
: QDialog(parent)
, ui(new Ui::TexturePicker)
, textureIconSize(QSize(78, 78))
, textureIconMargin(QSize(0, 0))
, columns(width() / (textureIconSize.width() + textureIconMargin.width()))
, textureWell(NULL)
{
    /// todo load textures from archive
    ui->setupUi(this);

    for(int i = 0; i < MAX_TEXTURES; ++i)
    {
        QTableWidgetItem* item = new QTableWidgetItem();

        ui->tableWidget->setItem(i, 0, item);
    }

    connect(ui->layerUpButton,         SIGNAL(clicked()), this, SLOT(moveLayerUp()));
    connect(ui->layerDownButton,       SIGNAL(clicked()), this, SLOT(moveLayerDown()));
    connect(ui->deleteLayerButton,     SIGNAL(clicked()), this, SLOT(deleteLayer()));
    connect(ui->loadAllTexturesButton, SIGNAL(clicked()), this, SLOT(loadAllTextures()));
    connect(ui->loadTexturesButton,    SIGNAL(clicked()), this, SLOT(addTextures()));

    connect(ui->tableWidget, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(handleLayerGUI(QTableWidgetItem*)));
}

TexturePicker::~TexturePicker()
{
    delete ui;
}

void TexturePicker::initialize(TextureManager* manager)
{
    textureManager = manager;

    QVector<QPair<QImage, QString>> textures;

    QPair<QString, TexturePtr> pair;

    foreach(pair, textureManager->getTextures())
    {
        if(pair.first.endsWith("ModelTexture"))
            continue;

        textures.append(qMakePair<QImage, QString>(QImage(pair.second->getPath()), pair.first));
    }

    if(textures.count() < columns)
        columns = textures.count(); // careful with this!!!

    if(textureWell)
    {
        delete textureWell;

        textureWell = NULL;

        columns = width() / (textureIconSize.width() + textureIconMargin.width());
    }

    textureWell = new TextureWell(this, ceil(MathHelper::toDouble(textures.count()) / MathHelper::toDouble(columns)), columns, textureIconSize, textureIconMargin);
    textureWell->insertItems(textures);

    ui->verticalLayout->addWidget(textureWell);

    connect(textureWell, SIGNAL(selected(int, int)), this, SLOT(selectedTexture(int, int)));
}

void TexturePicker::resizeEvent(QResizeEvent* e)
{
    int tColumns = width() / (textureIconSize.width() + textureIconMargin.width());

    if(columns != tColumns)
    {
        columns = tColumns;

        int count = textureManager->getTextures().count();

        if(count < columns)
            textureWell->setCols(count);
        else
            textureWell->setCols(columns);

        textureWell->setRows(ceil(MathHelper::toDouble(count) / MathHelper::toDouble(columns)));
    }

    QDialog::resizeEvent(e);
}

void TexturePicker::selectedTexture(int row, int cell)
{
    QVector<QString> textures;

    QPair<QString, TexturePtr> pair;

    foreach(pair, textureManager->getTextures())
    {
        if(pair.first.endsWith("ModelTexture"))
            continue;

        textures.append(pair.second->getPath());
    }

    textureManager->setSelectedTexture(textures.at(cell + row * columns));
}

void TexturePicker::setChunk(MapChunk* chunk)
{
    if(mapChunk == chunk)
        return;

    mapChunk = chunk;

    if(mapChunk == NULL)
        return;

    // Set label X Y
    ui->chunkIndexLabel->setText(QString("%1 %2").arg(chunk->chunkIndex() / CHUNKS).arg(chunk->chunkIndex() % CHUNKS));

    // Set table data
    for(int i = 0; i < MAX_TEXTURES; ++i)
    {
        QTableWidgetItem* item = ui->tableWidget->item(i, 0);

        if(!chunk->getTexture(i)->isNull())
            item->setData(Qt::DecorationRole, QPixmap::fromImage(QImage(chunk->getTexture(i)->getPath())).scaled(QSize(48, 48)));
    }
}

void TexturePicker::moveLayerUp()
{
    int index = ui->tableWidget->selectedItems().first()->row();

    if(mapChunk != NULL)
        mapChunk->moveAlphaMap(index, true);

    QTableWidgetItem* temp = ui->tableWidget->takeItem(index - 1, 0);
    QTableWidgetItem* item = ui->tableWidget->takeItem(index, 0);

    ui->tableWidget->setItem(index - 1, 0, item);
    ui->tableWidget->setItem(index, 0, temp);

    ui->tableWidget->setCurrentCell(index - 1, 0);

    handleLayerGUI(ui->tableWidget->itemAt(0, index - 1));
}

void TexturePicker::moveLayerDown()
{
    int index = ui->tableWidget->selectedItems().first()->row();

    if(mapChunk != NULL)
        mapChunk->moveAlphaMap(index, false);

    QTableWidgetItem* temp = ui->tableWidget->takeItem(index + 1, 0);
    QTableWidgetItem* item = ui->tableWidget->takeItem(index, 0);

    ui->tableWidget->setItem(index + 1, 0, item);
    ui->tableWidget->setItem(index, 0, temp);

    ui->tableWidget->setCurrentCell(index + 1, 0);

    handleLayerGUI(ui->tableWidget->itemAt(0, index + 1));
}

void TexturePicker::deleteLayer()
{
    int index = ui->tableWidget->selectedItems().first()->row();

    if(mapChunk != NULL)
        mapChunk->deleteAlphaMap(index);

    ui->tableWidget->selectedItems().first()->setData(Qt::DecorationRole, QVariant::Invalid);
}

void TexturePicker::loadAllTextures()
{
    QDir textureDir("textures/");

    if(textureDir.exists())
    {
        QVector<QPair<QImage, QString>> texts;

        foreach(QString file, textureDir.entryList(QDir::Files | QDir::Readable))
        {
            QFile f(file);

            if(!textureManager->hasTexture(QFileInfo(f.fileName()).baseName() + "Texture", textureDir.filePath(f.fileName())))
            {
                textureManager->loadTexture(QFileInfo(f.fileName()).baseName() + "Texture", textureDir.filePath(f.fileName()));

                QImage img(textureDir.filePath(f.fileName()));

                texts.append(qMakePair<QImage, QString>(img, QFileInfo(f.fileName()).baseName() + "Texture"));
            }
        }

        qDebug() << texts.count();

        int rowHeight = textureWell->height() / textureWell->numRows();
        int rows      = ceil(MathHelper::toDouble(textureManager->getTextures().count()) / MathHelper::toDouble(columns));

        textureWell->insertItems(texts);
        textureWell->setRows(rows);

        textureWell->setMinimumHeight(rowHeight * rows);
    }
}

void TexturePicker::addTextures()
{
    QStringList textures = QFileDialog::getOpenFileNames(this, tr("Choose textures"), QString(), tr("Texture files (*.png)"));

    QVector<QPair<QImage, QString>> texts;

    for(int i = 0; i < textures.count(); ++i)
    {
        QFile f(textures.at(i));

        if(!f.exists())
            continue;

        if(!textureManager->hasTexture(QFileInfo(f.fileName()).baseName() + "Texture", QFileInfo(f).filePath()))
        {
            textureManager->loadTexture(QFileInfo(f.fileName()).baseName() + "Texture", QFileInfo(f).filePath());

            QImage img(QFileInfo(f).filePath());

            texts.append(qMakePair<QImage, QString>(img, QFileInfo(f.fileName()).baseName() + "Texture"));
        }
    }

    int rowHeight = textureWell->height() / textureWell->numRows();
    int rows      = ceil(MathHelper::toDouble(textureManager->getTextures().count()) / MathHelper::toDouble(columns));

    textureWell->insertItems(texts);
    textureWell->setRows(rows);

    textureWell->setMinimumHeight(rowHeight * rows);
}

void TexturePicker::handleLayerGUI(QTableWidgetItem* item)
{
    int row = item->row();

    if(row <= 0)
        ui->layerUpButton->setEnabled(false);
    else
        ui->layerUpButton->setEnabled(true);

    if(row >= ALPHAMAPS)
        ui->layerDownButton->setEnabled(false);
    else
        ui->layerDownButton->setEnabled(true);

    if(item->data(Qt::DecorationRole).isValid())
        ui->deleteLayerButton->setEnabled(true);
    else
        ui->deleteLayerButton->setEnabled(false);
}
