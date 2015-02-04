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

#include "mapgeneration.h"
#include "ui_mapgeneration.h"

#include "mathhelper.h"

MapGeneration::MapGeneration(QWidget* parent)
: QDialog(parent)
, ui(new Ui::MapGeneration)
{
    ui->setupUi(this);

    setWindowFlags(Qt::WindowStaysOnTopHint);

    data.seed                = ui->seedSlider->value();
    data.perlinNoiseLevel    = ui->perlinNoiseLevelBox->currentText().toFloat();
    data.perlinNoiseHeight   = MathHelper::toFloat(ui->perlinNoiseHeightBox->value());
    data.perlinNoiseMultiple = ui->perlinNoiseMultipleBox->value();
    data.perturbFrequency    = ui->perturbFrequencyBox->currentText().toFloat();
    data.perturbD            = ui->perturbDBox->currentText().toFloat();
    data.erodeSmoothenLevel  = ui->erodeSmoothLevelBox->value();
    data.erode               = ui->erodeBox->currentText().toFloat();

    connect(ui->seedSlider,             SIGNAL(valueChanged(int)),            this, SLOT(setSeed(int)));
    connect(ui->perlinNoiseLevelBox,    SIGNAL(currentIndexChanged(QString)), this, SLOT(setPerlinNoiseLevel(QString)));
    connect(ui->perlinNoiseHeightBox,   SIGNAL(valueChanged(int)),            this, SLOT(setPerlinNoiseHeight(int)));
    connect(ui->perlinNoiseMultipleBox, SIGNAL(valueChanged(int)),            this, SLOT(setPerlinNoiseMultiple(int)));
    connect(ui->perturbFrequencyBox,    SIGNAL(currentIndexChanged(QString)), this, SLOT(setPerturbFrequency(QString)));
    connect(ui->perturbDBox,            SIGNAL(currentIndexChanged(QString)), this, SLOT(setPerturbD(QString)));
    connect(ui->erodeSmoothLevelBox,    SIGNAL(valueChanged(int)),            this, SLOT(setErodeSmoothLevel(int)));
    connect(ui->erodeBox,               SIGNAL(currentIndexChanged(QString)), this, SLOT(setErode(QString)));

    connect(ui->regenerateButton, SIGNAL(clicked()), this, SLOT(regenerate()));
}

MapGeneration::~MapGeneration()
{
    delete ui;
}

void MapGeneration::regenerate()
{
    emit generate(data);
}

void MapGeneration::setSeed(int value)
{
    ui->label->setText(QString("%1:").arg(value));

    data.seed = value;

    if(ui->regenerateBox->checkState() == Qt::Checked)
        regenerate();
}

void MapGeneration::setPerlinNoiseLevel(QString level)
{
    data.perlinNoiseLevel = level.toFloat();

    if(ui->regenerateBox->checkState() == Qt::Checked)
        regenerate();
}

void MapGeneration::setPerlinNoiseHeight(int height)
{
    data.perlinNoiseHeight = MathHelper::toFloat(height);

    if(ui->regenerateBox->checkState() == Qt::Checked)
        regenerate();
}

void MapGeneration::setPerlinNoiseMultiple(int multiple)
{
    data.perlinNoiseMultiple = multiple;

    if(ui->regenerateBox->checkState() == Qt::Checked)
        regenerate();
}

void MapGeneration::setPerturbFrequency(QString frequency)
{
    data.perturbFrequency = frequency.toFloat();

    if(ui->regenerateBox->checkState() == Qt::Checked)
        regenerate();
}

void MapGeneration::setPerturbD(QString d)
{
    data.perturbD = d.toFloat();

    if(ui->regenerateBox->checkState() == Qt::Checked)
        regenerate();
}

void MapGeneration::setErodeSmoothLevel(int level)
{
    data.erodeSmoothenLevel = level;

    if(ui->regenerateBox->checkState() == Qt::Checked)
        regenerate();
}

void MapGeneration::setErode(QString erode)
{
    data.erode = erode.toFloat();

    if(ui->regenerateBox->checkState() == Qt::Checked)
        regenerate();
}
