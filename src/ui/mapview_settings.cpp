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

#include "mapview_settings.h"
#include "ui_mapview_settings.h"

#include "qeditor.h"
#include "mathhelper.h"

MapView_Settings::MapView_Settings(QWidget* parent)
: QDialog(parent)
, ui(new Ui::MapView_Settings)
, brushColorType(Outer)
, wireframeColorType(Wireframe)
, cacheOuterBrushColor(app().getSetting("outerBrushColor", QColor(0, 255, 0)).value<QColor>())
, cacheInnerBrushColor(app().getSetting("innerBrushColor", QColor(0, 255, 0)).value<QColor>())
, cacheWireframeColor(MathHelper::toColor(app().getSetting("wireframe",        QVector4D(0.5f, 1.0f, 0.0f, 1.0f)).value<QVector4D>()))
, cacheTWireframeColor(MathHelper::toColor(app().getSetting("terrainWireframe", QVector4D(0.0f, 0.0f, 0.0f, 0.0f)).value<QVector4D>()))
, textureScaleFarSlider(new QDSlider())
, textureScaleNearSlider(new QDSlider())
{
    ui->setupUi(this);

    initializeComponents();

    // Environment Distance
    ui->EDistanceSlider->setValue(app().getSetting("environmentDistanceSlider", 1).toInt());

    // Texture scale
    ui->textureScaleOptions->insertItem(0, tr("Both multiply"),             QVariant::fromValue(0));
    ui->textureScaleOptions->insertItem(1, tr("Both divide"),               QVariant::fromValue(1));
    ui->textureScaleOptions->insertItem(2, tr("Far multiply, Near divide"), QVariant::fromValue(2));
    ui->textureScaleOptions->insertItem(3, tr("Far divide, Near multiply"), QVariant::fromValue(3));
    ui->textureScaleOptions->setCurrentIndex(app().getSetting("textureScaleOption").toInt());

    textureScaleFarSlider->setMinimum(0.1);
    textureScaleFarSlider->setMaximum(0.9);
    textureScaleFarSlider->setValue(app().getSetting("textureScaleFarSlider", 0.4).toDouble());

    textureScaleNearSlider->setMinimum(0.1);
    textureScaleNearSlider->setMaximum(0.9);
    textureScaleNearSlider->setValue(app().getSetting("textureScaleNearSlider", 0.4).toDouble());

    ui->gridLayout->addWidget(textureScaleFarSlider,  6, 1, 1, 2);
    ui->gridLayout->addWidget(textureScaleNearSlider, 7, 1, 1, 2);

    // tablet mode
    ui->tabletModeBox->setChecked(app().getSetting("tabletMode", false).toBool());

    // connects
    connect(ui->outerBrushColorButton, SIGNAL(clicked()), this, SLOT(showColorDialog()));
    connect(ui->innerBrushColorButton, SIGNAL(clicked()), this, SLOT(showColorDialog()));

    connect(ui->wireframeColorButton,        SIGNAL(clicked()), this, SLOT(showColorDialog()));
    connect(ui->terrainWireframeColorButton, SIGNAL(clicked()), this, SLOT(showColorDialog()));

    connect(colorDialog, SIGNAL(colorSelected(QColor)),       this, SLOT(setBrushCacheColor(QColor)));
    connect(colorDialog, SIGNAL(colorSelected(QColor)),       this, SLOT(setBrushColor(QColor)));
    connect(colorDialog, SIGNAL(currentColorChanged(QColor)), this, SLOT(setBrushColor(QColor)));
    connect(colorDialog, SIGNAL(rejected()),                  this, SLOT(setBrushToCacheColor()));

    connect(wireframeColorDialog, SIGNAL(colorSelected(QColor)),       this, SLOT(setWireframeCacheColor(QColor)));
    connect(wireframeColorDialog, SIGNAL(colorSelected(QColor)),       this, SLOT(setWireframeColor(QColor)));
    connect(wireframeColorDialog, SIGNAL(currentColorChanged(QColor)), this, SLOT(setWireframeColor(QColor)));
    connect(wireframeColorDialog, SIGNAL(rejected()),                  this, SLOT(setWireframeToCacheColor()));

    connect(ui->EDistanceSlider, SIGNAL(valueChanged(int)), this, SLOT(setEnvironmentDistance(int)));

    connect(ui->textureScaleOptions, SIGNAL(currentIndexChanged(int)), this, SLOT(setTextureScaleOpt(int)));
    connect(textureScaleFarSlider,   SIGNAL(valueChanged(double)),     this, SLOT(setTextureScaleFar(double)));
    connect(textureScaleNearSlider,  SIGNAL(valueChanged(double)),     this, SLOT(setTextureScaleNear(double)));

    connect(ui->tabletModeBox, SIGNAL(stateChanged(int)), this, SLOT(setTabletMode(int)));
}

MapView_Settings::~MapView_Settings()
{
    delete ui;
}

void MapView_Settings::initializeComponents()
{
    colorDialog = new QColorDialog();
    colorDialog->setCurrentColor(cacheOuterBrushColor);

    wireframeColorDialog = new QColorDialog();
    wireframeColorDialog->setCurrentColor(cacheWireframeColor);
}

void MapView_Settings::showColorDialog()
{
    QString name = qobject_cast<QPushButton*>(this->sender())->objectName();

    if(name == "outerBrushColorButton")
    {
        brushColorType = Outer;

        colorDialog->setCurrentColor(cacheOuterBrushColor);
        colorDialog->show();
    }
    else if(name == "innerBrushColorButton")
    {
        brushColorType = Inner;

        colorDialog->setCurrentColor(cacheInnerBrushColor);
        colorDialog->show();
    }
    else if(name == "wireframeColorButton")
    {
        wireframeColorType = Wireframe;

        wireframeColorDialog->setCurrentColor(cacheWireframeColor);
        wireframeColorDialog->show();
    }
    else if(name == "terrainWireframeColorButton")
    {
        wireframeColorType = TerrainWireframe;

        wireframeColorDialog->setCurrentColor(cacheTWireframeColor);
        wireframeColorDialog->show();
    }
}

void MapView_Settings::setBrushCacheColor(QColor color)
{
    switch(brushColorType)
    {
        case Outer:
            cacheOuterBrushColor = color;
            break;

        case Inner:
            cacheInnerBrushColor = color;
            break;
    }
}

void MapView_Settings::setBrushColor(QColor color)
{
    emit setColorOfBrush(&color, brushColorType == Outer ? true : false);
}

void MapView_Settings::setBrushToCacheColor()
{
    switch(brushColorType)
    {
        case Outer:
            emit setColorOfBrush(&cacheOuterBrushColor, true);
            break;

        case Inner:
            emit setColorOfBrush(&cacheInnerBrushColor, false);
            break;
    }
}

void MapView_Settings::setWireframeCacheColor(QColor color)
{
    switch(wireframeColorType)
    {
        case Wireframe:
            cacheWireframeColor = color;
            break;

        case TerrainWireframe:
            cacheTWireframeColor = color;
            break;
    }
}

void MapView_Settings::setWireframeColor(QColor color)
{
    emit setColorOfWireframe(&color, wireframeColorType == Wireframe ? false : true);
}

void MapView_Settings::setWireframeToCacheColor()
{
    switch(wireframeColorType)
    {
        case Wireframe:
            emit setColorOfWireframe(&cacheWireframeColor, false);
            break;

        case TerrainWireframe:
            emit setColorOfWireframe(&cacheTWireframeColor, true);
            break;
    }
}

void MapView_Settings::setEnvironmentDistance(int posVal)
{
    emit setEnvironmentDistance(posVal * 256.0f);

    app().setSetting("environmentDistanceSlider", posVal);
}

void MapView_Settings::setTextureScaleOpt(int option)
{
    emit setTextureScaleOption(option);

    app().setSetting("textureScaleOption", option);
}

void MapView_Settings::setTextureScaleFar(double scale)
{
    emit setTextureScaleFar(MathHelper::toFloat(scale));

    app().setSetting("textureScaleFarSlider", scale);
}

void MapView_Settings::setTextureScaleNear(double scale)
{
    emit setTextureScaleNear(MathHelper::toFloat(scale));

    app().setSetting("textureScaleNearSlider", scale);
}

void MapView_Settings::setTabletMode(int state)
{
    switch(state)
    {
        case 0: // Unchecked
            emit setTabletMode(false);
            break;

        default: // PartiallyChecked + Checked
            emit setTabletMode(true);
            break;
    }
}
