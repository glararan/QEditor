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