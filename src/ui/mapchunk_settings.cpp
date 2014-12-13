#include "mapchunk_settings.h"
#include "ui_mapchunk_settings.h"

#include "mapchunk.h"

MapChunk_Settings::MapChunk_Settings(QWidget* parent)
: QDialog(parent)
, ui(new Ui::MapChunk_Settings)
, textureScaleFarSlider0(new QDSlider())
, textureScaleNearSlider0(new QDSlider())
, textureScaleFarSlider1(new QDSlider())
, textureScaleNearSlider1(new QDSlider())
, textureScaleFarSlider2(new QDSlider())
, textureScaleNearSlider2(new QDSlider())
, textureScaleFarSlider3(new QDSlider())
, textureScaleNearSlider3(new QDSlider())
{
    ui->setupUi(this);

    // Texture scale
    ui->textureScaleOptions0->setItemData(0, 0);
    ui->textureScaleOptions0->setItemData(1, 1);
    ui->textureScaleOptions0->setItemData(2, 2);
    ui->textureScaleOptions0->setItemData(3, 3);

    ui->textureScaleOptions1->setItemData(0, 0);
    ui->textureScaleOptions1->setItemData(1, 1);
    ui->textureScaleOptions1->setItemData(2, 2);
    ui->textureScaleOptions1->setItemData(3, 3);

    ui->textureScaleOptions2->setItemData(0, 0);
    ui->textureScaleOptions2->setItemData(1, 1);
    ui->textureScaleOptions2->setItemData(2, 2);
    ui->textureScaleOptions2->setItemData(3, 3);

    ui->textureScaleOptions3->setItemData(0, 0);
    ui->textureScaleOptions3->setItemData(1, 1);
    ui->textureScaleOptions3->setItemData(2, 2);
    ui->textureScaleOptions3->setItemData(3, 3);

    textureScaleFarSlider0->setMinimum(0.1);
    textureScaleFarSlider0->setMaximum(0.9);
    textureScaleFarSlider0->setObjectName("textureScaleFarSlider0");

    textureScaleFarSlider1->setMinimum(0.1);
    textureScaleFarSlider1->setMaximum(0.9);
    textureScaleFarSlider1->setObjectName("textureScaleFarSlider1");

    textureScaleFarSlider2->setMinimum(0.1);
    textureScaleFarSlider2->setMaximum(0.9);
    textureScaleFarSlider2->setObjectName("textureScaleFarSlider2");

    textureScaleFarSlider3->setMinimum(0.1);
    textureScaleFarSlider3->setMaximum(0.9);
    textureScaleFarSlider3->setObjectName("textureScaleFarSlider3");

    textureScaleNearSlider0->setMinimum(0.1);
    textureScaleNearSlider0->setMaximum(0.9);
    textureScaleNearSlider0->setObjectName("textureScaleNearSlider0");

    textureScaleNearSlider1->setMinimum(0.1);
    textureScaleNearSlider1->setMaximum(0.9);
    textureScaleNearSlider1->setObjectName("textureScaleNearSlider1");

    textureScaleNearSlider2->setMinimum(0.1);
    textureScaleNearSlider2->setMaximum(0.9);
    textureScaleNearSlider2->setObjectName("textureScaleNearSlider2");

    textureScaleNearSlider3->setMinimum(0.1);
    textureScaleNearSlider3->setMaximum(0.9);
    textureScaleNearSlider3->setObjectName("textureScaleNearSlider3");

    // set to grid
    ui->gridLayout_ts0->addWidget(textureScaleFarSlider0,  1, 1, 1, 2);
    ui->gridLayout_ts0->addWidget(textureScaleNearSlider0, 2, 1, 1, 2);

    ui->gridLayout_ts1->addWidget(textureScaleFarSlider1,  1, 1, 1, 2);
    ui->gridLayout_ts1->addWidget(textureScaleNearSlider1, 2, 1, 1, 2);

    ui->gridLayout_ts2->addWidget(textureScaleFarSlider2,  1, 1, 1, 2);
    ui->gridLayout_ts2->addWidget(textureScaleNearSlider2, 2, 1, 1, 2);

    ui->gridLayout_ts3->addWidget(textureScaleFarSlider3,  1, 1, 1, 2);
    ui->gridLayout_ts3->addWidget(textureScaleNearSlider3, 2, 1, 1, 2);

    // disable tab for texture scales, no chunk is selected
    ui->textureScaleTabWidget->setEnabled(false);

    // connects
    connect(ui->textureScaleOptions0, SIGNAL(currentIndexChanged(int)), this, SLOT(setTextureScaleOpt(int)));
    connect(ui->textureScaleOptions1, SIGNAL(currentIndexChanged(int)), this, SLOT(setTextureScaleOpt(int)));
    connect(ui->textureScaleOptions2, SIGNAL(currentIndexChanged(int)), this, SLOT(setTextureScaleOpt(int)));
    connect(ui->textureScaleOptions3, SIGNAL(currentIndexChanged(int)), this, SLOT(setTextureScaleOpt(int)));

    connect(textureScaleFarSlider0, SIGNAL(valueChanged(double)), this, SLOT(setTextureScaleFar(double)));
    connect(textureScaleFarSlider1, SIGNAL(valueChanged(double)), this, SLOT(setTextureScaleFar(double)));
    connect(textureScaleFarSlider2, SIGNAL(valueChanged(double)), this, SLOT(setTextureScaleFar(double)));
    connect(textureScaleFarSlider3, SIGNAL(valueChanged(double)), this, SLOT(setTextureScaleFar(double)));

    connect(textureScaleNearSlider0, SIGNAL(valueChanged(double)), this, SLOT(setTextureScaleNear(double)));
    connect(textureScaleNearSlider1, SIGNAL(valueChanged(double)), this, SLOT(setTextureScaleNear(double)));
    connect(textureScaleNearSlider2, SIGNAL(valueChanged(double)), this, SLOT(setTextureScaleNear(double)));
    connect(textureScaleNearSlider3, SIGNAL(valueChanged(double)), this, SLOT(setTextureScaleNear(double)));

    connect(ui->automaticTexture1, SIGNAL(stateChanged(int)), this, SLOT(setAutomaticStartEndEnabled(int)));
    connect(ui->automaticTexture2, SIGNAL(stateChanged(int)), this, SLOT(setAutomaticStartEndEnabled(int)));
    connect(ui->automaticTexture3, SIGNAL(stateChanged(int)), this, SLOT(setAutomaticStartEndEnabled(int)));

    connect(ui->startHeight1, SIGNAL(valueChanged(double)), this, SLOT(setAutomaticStartEndCorrection(double)));
    connect(ui->startHeight2, SIGNAL(valueChanged(double)), this, SLOT(setAutomaticStartEndCorrection(double)));
    connect(ui->startHeight3, SIGNAL(valueChanged(double)), this, SLOT(setAutomaticStartEndCorrection(double)));

    connect(ui->endHeight1, SIGNAL(valueChanged(double)), this, SLOT(setAutomaticEnd(double)));
    connect(ui->endHeight2, SIGNAL(valueChanged(double)), this, SLOT(setAutomaticEnd(double)));
    connect(ui->endHeight3, SIGNAL(valueChanged(double)), this, SLOT(setAutomaticEnd(double)));
}

MapChunk_Settings::~MapChunk_Settings()
{
    delete ui;

    delete textureScaleFarSlider0;
    delete textureScaleNearSlider0;

    delete textureScaleFarSlider1;
    delete textureScaleNearSlider1;

    delete textureScaleFarSlider2;
    delete textureScaleNearSlider2;

    delete textureScaleFarSlider3;
    delete textureScaleNearSlider3;
}

void MapChunk_Settings::setTextureScaleOpt(int option)
{
    QString name = qobject_cast<QComboBox*>(this->sender())->objectName();

    int index = QString(name.at(name.size() - 1)).toInt();

    chunk->setTextureScaleOption(option, index);
}

void MapChunk_Settings::setTextureScaleFar(double scale)
{
    QString name = qobject_cast<QDSlider*>(this->sender())->objectName();

    int index = QString(name.at(name.size() - 1)).toInt();

    chunk->setTextureScaleFar(scale, index);
}

void MapChunk_Settings::setTextureScaleNear(double scale)
{
    QString name = qobject_cast<QDSlider*>(this->sender())->objectName();

    int index = QString(name.at(name.size() - 1)).toInt();

    chunk->setTextureScaleNear(scale, index);
}

void MapChunk_Settings::setAutomaticStartEndEnabled(int state)
{
    QString name = qobject_cast<QCheckBox*>(this->sender())->objectName();

    int index = QString(name.at(name.size() - 1)).toInt();

    bool status = false;

    if(state == 2)
        status = true;

    switch(index)
    {
        case 1:
            {
                ui->startHeight1->setEnabled(status);
                ui->endHeight1->setEnabled(status);
            }
            break;

        case 2:
            {
                ui->startHeight2->setEnabled(status);
                ui->endHeight2->setEnabled(status);
            }
            break;

        case 3:
            {
                ui->startHeight3->setEnabled(status);
                ui->endHeight3->setEnabled(status);
            }
            break;
    }

    chunk->setAutomaticTexture(status, index);
}

void MapChunk_Settings::setAutomaticStartEndCorrection(double value)
{
    QString name = qobject_cast<QDoubleSpinBox*>(this->sender())->objectName();

    int index = QString(name.at(name.size() - 1)).toInt();

    switch(index)
    {
        case 1:
            ui->endHeight1->setMinimum(value);
            break;

        case 2:
            ui->endHeight2->setMinimum(value);
            break;

        case 3:
            ui->endHeight3->setMinimum(value);
            break;
    }

    chunk->setAutomaticTextureStart(value, index);
}

void MapChunk_Settings::setAutomaticEnd(double value)
{
    QString name = qobject_cast<QDoubleSpinBox*>(this->sender())->objectName();

    int index = QString(name.at(name.size() - 1)).toInt();

    chunk->setAutomaticTextureEnd(value, index);
}

void MapChunk_Settings::setChunk(MapChunk* mapChunk)
{
    if(chunk == mapChunk)
        return;

    chunk = mapChunk;

    if(chunk == NULL)
    {
        ui->textureScaleTabWidget->setEnabled(false);

        return;
    }

    // Set label X Y
    ui->chunkIndexLabel->setText(QString("%1 %2").arg(chunk->chunkIndex() / CHUNKS).arg(chunk->chunkIndex() % CHUNKS));

    ui->textureScaleTabWidget->setEnabled(true);

    // Set texture scale items
    ui->textureScaleOptions0->setCurrentIndex(chunk->getTextureScaleOption(0));
    ui->textureScaleOptions1->setCurrentIndex(chunk->getTextureScaleOption(1));
    ui->textureScaleOptions2->setCurrentIndex(chunk->getTextureScaleOption(2));
    ui->textureScaleOptions3->setCurrentIndex(chunk->getTextureScaleOption(3));

    textureScaleFarSlider0->setValue(MathHelper::toDouble(chunk->getTextureScaleFar(0)));
    textureScaleFarSlider1->setValue(MathHelper::toDouble(chunk->getTextureScaleFar(1)));
    textureScaleFarSlider2->setValue(MathHelper::toDouble(chunk->getTextureScaleFar(2)));
    textureScaleFarSlider3->setValue(MathHelper::toDouble(chunk->getTextureScaleFar(3)));

    textureScaleNearSlider0->setValue(MathHelper::toDouble(chunk->getTextureScaleNear(0)));
    textureScaleNearSlider1->setValue(MathHelper::toDouble(chunk->getTextureScaleNear(1)));
    textureScaleNearSlider2->setValue(MathHelper::toDouble(chunk->getTextureScaleNear(2)));
    textureScaleNearSlider3->setValue(MathHelper::toDouble(chunk->getTextureScaleNear(3)));

    if(chunk->getAutomaticTexture(1))
    {
        ui->automaticTexture1->setChecked(true);
        ui->startHeight1->setEnabled(true);
        ui->endHeight1->setEnabled(true);
    }
    else
    {
        ui->automaticTexture1->setChecked(false);
        ui->startHeight1->setEnabled(false);
        ui->endHeight1->setEnabled(false);
    }

    ui->startHeight1->setValue(chunk->getAutomaticTextureStart(1));
    ui->endHeight1->setValue(chunk->getAutomaticTextureEnd(1));

    if(chunk->getAutomaticTexture(2))
    {
        ui->automaticTexture2->setChecked(true);
        ui->startHeight2->setEnabled(true);
        ui->endHeight2->setEnabled(true);
    }
    else
    {
        ui->automaticTexture2->setChecked(false);
        ui->startHeight2->setEnabled(false);
        ui->endHeight2->setEnabled(false);
    }

    ui->startHeight2->setValue(chunk->getAutomaticTextureStart(2));
    ui->endHeight2->setValue(chunk->getAutomaticTextureEnd(2));

    if(chunk->getAutomaticTexture(3))
    {
        ui->automaticTexture3->setChecked(true);
        ui->startHeight3->setEnabled(true);
        ui->endHeight3->setEnabled(true);
    }
    else
    {
        ui->automaticTexture3->setChecked(false);
        ui->startHeight3->setEnabled(false);
        ui->endHeight3->setEnabled(false);
    }

    ui->startHeight3->setValue(chunk->getAutomaticTextureStart(3));
    ui->endHeight3->setValue(chunk->getAutomaticTextureEnd(3));
}
