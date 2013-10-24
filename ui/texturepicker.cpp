#include "texturepicker.h"
#include "ui_texturepicker.h"

#include <QPixmap>

#include <QDebug>
#include <QMessageBox>

TexturePicker::TexturePicker(QWidget* parent)
: QDialog(parent)
, ui(new Ui::TexturePicker)
, textureIconSize(QSize(78, 78))
, textureIconMargin(QSize(0, 0))
, columns(width() / (textureIconSize.width() + textureIconMargin.width()))
{
    /// todo load textures from archive
    ui->setupUi(this);

    QImage grass("grass.png");
    QImage rock("rock.png");
    QImage snowrock("snowrocks.png");

    textures.append(qMakePair<QImage, QString>(grass, "grass"));
    textures.append(qMakePair<QImage, QString>(rock, "rock"));
    textures.append(qMakePair<QImage, QString>(snowrock, "snowrocks"));

    if(textures.count() < columns)
        columns = textures.count();

    textureWell = new TextureWell(parent, ceil((double)textures.count() / (double)columns), columns, textures, textureIconSize, textureIconMargin);

    layout = new QGridLayout();
    layout->addWidget(textureWell, 0, 1);

    setLayout(layout);
}

TexturePicker::~TexturePicker()
{
    delete ui;
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

        textureWell->setRows(ceil((double)textures.count() / (double)columns));
    }

    QDialog::resizeEvent(e);
}

const QImage TexturePicker::getTexture() const
{
    return textures.at(columns * textureWell->selectedRow() + textureWell->selectedColumn()).first;
}