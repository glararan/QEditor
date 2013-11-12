#include "mapview_settings.h"
#include "ui_mapview_settings.h"

#include "qeditor.h"

MapView_Settings::MapView_Settings(QWidget* parent)
: QDialog(parent)
, ui(new Ui::MapView_Settings)
, cacheBrushColor(app().getSetting("brushColor", QColor(0, 255, 0)).value<QColor>())
{
    ui->setupUi(this);

    initializeComponents();

    setBrushColor(cacheBrushColor);

    ui->EDistanceSlider->setValue(app().getSetting("environmentDistanceSlider", 1).toInt());

    connect(ui->brushColorButton, SIGNAL(clicked()), this, SLOT(showColorDialog()));

    connect(colorDialog, SIGNAL(colorSelected(QColor)),       this, SLOT(setCacheColor(QColor)));
    connect(colorDialog, SIGNAL(colorSelected(QColor)),       this, SLOT(setBrushColor(QColor)));
    connect(colorDialog, SIGNAL(currentColorChanged(QColor)), this, SLOT(setBrushColor(QColor)));
    connect(colorDialog, SIGNAL(rejected()),                  this, SLOT(setBrushToCacheColor()));

    connect(ui->EDistanceSlider, SIGNAL(valueChanged(int)), this, SLOT(setEnvironmentDistance(int)));
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
