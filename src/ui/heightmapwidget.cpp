#include "heightmapwidget.h"
#include "ui_heightmapwidget.h"

#include <QFileDialog>
#include <QMessageBox>

#include "mathhelper.h"

HeightmapWidget::HeightmapWidget(QWidget* parent)
: QDialog(parent)
, ui(new Ui::HeightmapWidget)
{
    ui->setupUi(this);

    connect(ui->importBrowseButton,    SIGNAL(clicked()), this, SLOT(showImportBrowser()));
    connect(ui->exportBrowseButton,    SIGNAL(clicked()), this, SLOT(showExportBrowser()));
    connect(ui->stlExportBrowseButton, SIGNAL(clicked()), this, SLOT(showSTLExportBrowser()));

    connect(ui->importButton,    SIGNAL(clicked()), this, SLOT(doImport()));
    connect(ui->exportButton,    SIGNAL(clicked()), this, SLOT(doExport()));
    connect(ui->stlExportButton, SIGNAL(clicked()), this, SLOT(doSTLExport()));

    connect(ui->heightmapScale, SIGNAL(valueChanged(double)), this, SLOT(setScale(double)));
}

HeightmapWidget::~HeightmapWidget()
{
    delete ui;
}

void HeightmapWidget::showImportBrowser()
{
    QString path = QFileDialog::getOpenFileName(0, tr("Select heightmap file"), QString(), tr("Portable Network Graphics (*.png)"));

    if(path == QString())
        return;

    ui->importFileBox->setText(path);
}

void HeightmapWidget::showExportBrowser()
{
    QString path = QFileDialog::getSaveFileName(0, tr("Choose output name"), QString(), tr("Portable Network Graphics (*.png)"));

    if(path == QString())
        return;

    ui->exportFileBox->setText(path);
}

void HeightmapWidget::showSTLExportBrowser()
{
    QString path = QFileDialog::getSaveFileName(0, tr("Choose output name"), QString(), tr("STereo Lithography File (*.stl)"));

    if(path == QString())
        return;

    ui->stlExportFileBox->setText(path);
}

void HeightmapWidget::doImport()
{
    emit importing(ui->importFileBox->text(), MathHelper::toFloat(ui->importScaleBox->value()));
}

void HeightmapWidget::doExport()
{
    emit exporting(ui->exportFileBox->text(), MathHelper::toFloat(ui->exportScaleBox->value()));
}

void HeightmapWidget::doSTLExport()
{
    if(QMessageBox::warning(this, tr("STL Export - Attention"), tr("This process can take few minutes. Also this process freeze application and use extra amount of CPU!"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
        emit stlExporting(ui->stlExportFileBox->text(), MathHelper::toFloat(ui->stlExportSurfaceBox->value()), ui->stlExportScaleHeightBox->isChecked(), ui->stlExportLowResolutionBox->isChecked());
}

void HeightmapWidget::setScale(double scale)
{
    emit setScale(MathHelper::toFloat(scale));
}
