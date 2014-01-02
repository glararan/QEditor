#include "mapview_settings.h"
#include "ui_mapview_settings.h"

#include "qeditor.h"

MapView_Settings::MapView_Settings(QWidget* parent)
: QDialog(parent)
, ui(new Ui::MapView_Settings)
, brushColorType(Outer)
, cacheOuterBrushColor(app().getSetting("outerBrushColor", QColor(0, 255, 0)).value<QColor>())
, cacheInnerBrushColor(app().getSetting("innerBrushColor", QColor(0, 255, 0)).value<QColor>())
, textureScaleFarSlider(new QDSlider())
, textureScaleNearSlider(new QDSlider())
{
    ui->setupUi(this);

    initializeComponents();

    // Brush color
    setBrushColor(cacheOuterBrushColor);

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

    ui->gridLayout->addWidget(textureScaleFarSlider,  4, 1, 1, 2);
    ui->gridLayout->addWidget(textureScaleNearSlider, 5, 1, 1, 2);

    // connects
    connect(ui->outerBrushColorButton, SIGNAL(clicked()), this, SLOT(showColorDialog()));
    connect(ui->innerBrushColorButton, SIGNAL(clicked()), this, SLOT(showColorDialog()));

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
    colorDialog->setCurrentColor(cacheOuterBrushColor);
}

void MapView_Settings::showColorDialog()
{
    QString name = qobject_cast<QPushButton*>(this->sender())->objectName();

    if(name == "outerBrushColorButton")
    {
        brushColorType = Outer;

        colorDialog->setCurrentColor(cacheOuterBrushColor);
    }
    else
    {
        brushColorType = Inner;

        colorDialog->setCurrentColor(cacheInnerBrushColor);
    }

    colorDialog->show();
}

void MapView_Settings::setCacheColor(QColor color)
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