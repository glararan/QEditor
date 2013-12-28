#include "mapview_settings.h"
#include "ui_mapview_settings.h"

#include "qeditor.h"

MapView_Settings::MapView_Settings(QWidget* parent)
: QDialog(parent)
, ui(new Ui::MapView_Settings)
, cacheBrushColor(app().getSetting("brushColor", QColor(0, 255, 0)).value<QColor>())
, textureScaleFarSlider(new QDSlider())
, textureScaleNearSlider(new QDSlider())
{
    ui->setupUi(this);

    initializeComponents();

    // Brush color
    setBrushColor(cacheBrushColor);

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

    ui->gridLayout->addWidget(textureScaleFarSlider,  3, 1, 1, 2);
    ui->gridLayout->addWidget(textureScaleNearSlider, 4, 1, 1, 2);

    // connects
    connect(ui->brushColorButton, SIGNAL(clicked()), this, SLOT(showColorDialog()));

    connect(colorDialog, SIGNAL(colorSelected(QColor)),       this, SLOT(setCacheColor(QColor)));
    connect(colorDialog, SIGNAL(colorSelected(QColor)),       this, SLOT(setBrushColor(QColor)));
    connect(colorDialog, SIGNAL(currentColorChanged(QColor)), this, SLOT(setBrushColor(QColor)));
    connect(colorDialog, SIGNAL(rejected()),                  this, SLOT(setBrushToCacheColor()));

    connect(ui->EDistanceSlider, SIGNAL(valueChanged(int)), this, SLOT(setEnvironmentDistance(int)));

    connect(ui->textureScaleOptions, SIGNAL(currentIndexChanged(int)), this, SLOT(setTextureScaleOpt(int)));
    connect(textureScaleFarSlider,   SIGNAL(valueChanged(double)),     this, SLOT(setTextureScaleFar(double)));
    connect(textureScaleNearSlider,  SIGNAL(valueChanged(double)),     this, SLOT(setTextureScaleNear(double)));
}

MapView_Settings::~MapView_Settings()
{
    delete ui;
}

void MapView_Settings::initializeComponents()
{
    colorDialog = new QColorDialog();
    colorDialog->setCurrentColor(cacheBrushColor);
}

void MapView_Settings::showColorDialog()
{
    colorDialog->show();
}

void MapView_Settings::setCacheColor(QColor color)
{
    cacheBrushColor = color;
}

void MapView_Settings::setBrushColor(QColor color)
{
    emit setColorOfBrush(&color);
}

void MapView_Settings::setBrushToCacheColor()
{
    emit setColorOfBrush(&cacheBrushColor);
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