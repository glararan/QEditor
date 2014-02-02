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
{
    /// todo load textures from archive
    ui->setupUi(this);

    for(int i = 0; i < MAX_TEXTURES; ++i)
    {
        QTableWidgetItem* item = new QTableWidgetItem();

        ui->tableWidget->setItem(i, 0, item);
    }

    connect(ui->layerUpButton,     SIGNAL(clicked()), this, SLOT(moveLayerUp()));
    connect(ui->layerDownButton,   SIGNAL(clicked()), this, SLOT(moveLayerDown()));
    connect(ui->deleteLayerButton, SIGNAL(clicked()), this, SLOT(deleteLayer()));

    connect(ui->tableWidget, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(handleLayerGUI(QTableWidgetItem*)));
}

TexturePicker::~TexturePicker()
{
    delete ui;
}

void TexturePicker::initialize(TextureManager* manager)
{
    textureManager = manager;

    QPair<QString, TexturePtr> pair;

    foreach(pair, textureManager->getTextures())
        textures.append(qMakePair<QImage, QString>(pair.second->getImage(), pair.first));

    if(textures.count() < columns)
        columns = textures.count();

<<<<<<< HEAD
    textureWell = new TextureWell(this, ceil(MathHelper::toDouble(textures.count()) / MathHelper::toDouble(columns)), columns, textures, textureIconSize, textureIconMargin);
=======
    textureWell = new TextureWell(this, 0, 4, textureIconSize, textureIconMargin);
    textureWell->insertItems(textures);
>>>>>>> origin/netix

    ui->verticalLayout->addWidget(textureWell);

    connect(textureWell, SIGNAL(selected(int, int)), this, SLOT(selectedTexture(int, int)));
}

void TexturePicker::resizeEvent(QResizeEvent* e)
{
    int tColumns = width() / (textureIconSize.width() + textureIconMargin.width());

    if(columns != tColumns)
    {
        columns = tColumns;

        if(textures.count() < columns)
            textureWell->setCols(textures.count());
        else
            textureWell->setCols(columns);

        textureWell->setRows(ceil(MathHelper::toDouble(textures.count()) / MathHelper::toDouble(columns)));
    }

    QDialog::resizeEvent(e);
}

void TexturePicker::selectedTexture(int row, int cell)
{
    textureManager->setSelectedTexture(cell + row * columns);
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
            item->setData(Qt::DecorationRole, QPixmap::fromImage(chunk->getTexture(i)->getImage()).scaled(QSize(48, 48)));
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

    ui->tableWidget->itemAt(0, index)->setData(Qt::DecorationRole, QVariant::Invalid);
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
<<<<<<< HEAD
}
=======
}
>>>>>>> origin/netix
