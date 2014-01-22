#include "waterwidget.h"
#include "ui_waterwidget.h"

#include "mathhelper.h"

WaterWidget::WaterWidget(QWidget* parent)
: QDialog(parent)
, ui(new Ui::WaterWidget)
, waterChunk(NULL)
{
    ui->setupUi(this);

    connect(ui->chunkWaterCheckBox, SIGNAL(stateChanged(int)),    this, SLOT(setWaterStatus(int)));
    connect(ui->topSpinBox,         SIGNAL(valueChanged(double)), this, SLOT(setTopHeight(double)));
    connect(ui->rightSpinBox,       SIGNAL(valueChanged(double)), this, SLOT(setRightHeight(double)));
    connect(ui->bottomSpinBox,      SIGNAL(valueChanged(double)), this, SLOT(setBottomHeight(double)));
    connect(ui->leftSpinBox,        SIGNAL(valueChanged(double)), this, SLOT(setLeftHeight(double)));
    connect(ui->updateButton,       SIGNAL(clicked()),            this, SLOT(updateWater()));
}

WaterWidget::~WaterWidget()
{
    delete ui;
}

void WaterWidget::setWaterStatus(int status)
{
    switch(status)
    {
        case 0: // unchecked
            {
                ui->topSpinBox->setEnabled(false);
                ui->rightSpinBox->setEnabled(false);
                ui->bottomSpinBox->setEnabled(false);
                ui->leftSpinBox->setEnabled(false);
                ui->topSpinBoxCheck->setEnabled(false);
                ui->rightSpinBoxCheck->setEnabled(false);
                ui->bottomSpinBoxCheck->setEnabled(false);
                ui->leftSpinBoxCheck->setEnabled(false);
                ui->updateButton->setEnabled(false);

                if(waterChunk)
                    waterChunk->setData(false);
            }
            break;

        case 2: // checked
            {
                ui->topSpinBox->setEnabled(true);
                ui->rightSpinBox->setEnabled(true);
                ui->bottomSpinBox->setEnabled(true);
                ui->leftSpinBox->setEnabled(true);
                ui->topSpinBoxCheck->setEnabled(true);
                ui->rightSpinBoxCheck->setEnabled(true);
                ui->bottomSpinBoxCheck->setEnabled(true);
                ui->leftSpinBoxCheck->setEnabled(true);
                ui->updateButton->setEnabled(true);

                if(waterChunk)
                    waterChunk->setData(true);
            }
            break;
    }
}

void WaterWidget::setChunk(WaterChunk* chunk)
{
    if(waterChunk == chunk)
        return;

    waterChunk = chunk;

    if(waterChunk == NULL)
    {
        ui->chunkWaterCheckBox->setEnabled(false);
        ui->chunkWaterCheckBox->setChecked(false);

        setWaterStatus(0);

        return;
    }

    // Set label X Y
    ui->chunkIndexLabel->setText(QString("%1 %2").arg(chunk->chunkIndex() / CHUNKS).arg(chunk->chunkIndex() % CHUNKS));

    // Set data
    ui->chunkWaterCheckBox->setEnabled(true);
    ui->chunkWaterCheckBox->setChecked(chunk->hasData());

    setWaterStatus(chunk->hasData() ? 2 : 0);

    ui->topSpinBox->setValue(chunk->getHeights().top);
    ui->rightSpinBox->setValue(chunk->getHeights().right);
    ui->bottomSpinBox->setValue(chunk->getHeights().bottom);
    ui->leftSpinBox->setValue(chunk->getHeights().left);

    ui->topSpinBoxCheck->setChecked(chunk->getHeights().topStatus);
    ui->rightSpinBoxCheck->setChecked(chunk->getHeights().rightStatus);
    ui->bottomSpinBoxCheck->setChecked(chunk->getHeights().bottomStatus);
    ui->leftSpinBoxCheck->setChecked(chunk->getHeights().leftStatus);
}

void WaterWidget::setTopHeight(double value)
{
    if(waterChunk)
    {
        WaterChunk::BorderHeights heights = waterChunk->getHeights();
        heights.top = MathHelper::toFloat(value);

        waterChunk->setHeights(heights);
    }
}

void WaterWidget::setRightHeight(double value)
{
    if(waterChunk)
    {
        WaterChunk::BorderHeights heights = waterChunk->getHeights();
        heights.right = MathHelper::toFloat(value);

        waterChunk->setHeights(heights);
    }
}

void WaterWidget::setBottomHeight(double value)
{
    if(waterChunk)
    {
        WaterChunk::BorderHeights heights = waterChunk->getHeights();
        heights.bottom = MathHelper::toFloat(value);

        waterChunk->setHeights(heights);
    }
}

void WaterWidget::setLeftHeight(double value)
{
    if(waterChunk)
    {
        WaterChunk::BorderHeights heights = waterChunk->getHeights();
        heights.left = MathHelper::toFloat(value);

        waterChunk->setHeights(heights);
    }
}

void WaterWidget::updateWater()
{
    if(waterChunk)
        waterChunk->updateData();
}