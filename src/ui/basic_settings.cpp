#include "basic_settings.h"
#include "ui_basic_settings.h"

#include "world.h"

#include <QMessageBox>

Basic_Settings::Basic_Settings(World* mWorld, QWidget* parent)
: QWizard(parent)
, ui(new Ui::Basic_Settings)
, world(mWorld)
{
    ui->setupUi(this);

    connect(ui->openGeneration,   SIGNAL(clicked()), this, SLOT(openTerrainGenerationWidget()));
    connect(ui->openImportExport, SIGNAL(clicked()), this, SLOT(openTerrainImportExportWidget()));

    connect(button(QWizard::CancelButton), SIGNAL(clicked()), this, SLOT(cancelEvent()));
}

Basic_Settings::~Basic_Settings()
{
    delete ui;
}

void Basic_Settings::setDefaultHeightEnabled(int state)
{
    QString name = qobject_cast<QCheckBox*>(this->sender())->objectName();

    bool status = false;

    if(state == 2)
        status = true;

    if(name == "terrainHeightEnableBox")
        ui->terrainHeightBox->setEnabled(status);
    else if(name == "waterHeightEnableBox")
        ui->waterHeightEnableBox->setEnabled(status);
}

void Basic_Settings::openTerrainGenerationWidget()
{
    emit openTerrainGeneration();
}

void Basic_Settings::openTerrainImportExportWidget()
{
    emit openTerrainImportExport();
}

void Basic_Settings::cancelEvent()
{
    if(QMessageBox::question(this, tr("Quit Wizard"), tr("Wizard is not complete yet. Are you sure you want to quit Wizard?"), QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
        reject();
}

void Basic_Settings::accept()
{
    BasicSettingsData data;
    data.setTerrainHeight = ui->terrainHeightEnableBox->isChecked();
    data.setWaterHeight   = ui->waterHeightEnableBox->isChecked();
    data.setWaterDraw     = ui->waterDrawBox->isChecked();
    data.terrainHeight    = MathHelper::toFloat(ui->terrainHeightBox->value());
    data.waterHeight      = MathHelper::toFloat(ui->waterHeightBox->value());

    world->setBasicSettings(data);
}

void Basic_Settings::reject()
{

}
