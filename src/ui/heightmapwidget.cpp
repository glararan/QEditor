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

#include "heightmapwidget.h"
#include "ui_heightmapwidget.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QHostAddress>
#include <QNetworkInterface>

#include "mathhelper.h"

HeightmapWidget::HeightmapWidget(QWidget* parent)
: QDialog(parent)
, ui(new Ui::HeightmapWidget)
//, server(NULL)
{
    ui->setupUi(this);

    foreach(const QHostAddress& address, QNetworkInterface::allAddresses())
    {
        if(address.protocol() == QAbstractSocket::IPv4Protocol && address != QHostAddress(QHostAddress::LocalHost))
             ui->ipAddressBox->addItem(address.toString(), address.toString());
    }

    connect(ui->importBrowseButton,    SIGNAL(clicked()), this, SLOT(showImportBrowser()));
    connect(ui->exportBrowseButton,    SIGNAL(clicked()), this, SLOT(showExportBrowser()));
    connect(ui->stlExportBrowseButton, SIGNAL(clicked()), this, SLOT(showSTLExportBrowser()));

    connect(ui->importButton,    SIGNAL(clicked()), this, SLOT(doImport()));
    connect(ui->exportButton,    SIGNAL(clicked()), this, SLOT(doExport()));
    connect(ui->stlExportButton, SIGNAL(clicked()), this, SLOT(doSTLExport()));

    connect(ui->heightmapScale, SIGNAL(valueChanged(double)), this, SLOT(setScale(double)));

    connect(ui->serverButton, SIGNAL(clicked()), this, SLOT(serverSwitch()));
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
        emit stlExporting(ui->stlExportFileBox->text(), MathHelper::toFloat(ui->stlExportSurfaceBox->value()), ui->stlExportScaleHeightBox->isChecked(), ui->stlExportLowResolutionBox->isChecked(), ui->stlExportMapTileBox->isChecked() ? true : false);
}

void HeightmapWidget::setScale(double scale)
{
    emit setScale(MathHelper::toFloat(scale));
}

void HeightmapWidget::serverSwitch()
{
    /*if(ui->ipAddressBox->itemData(ui->ipAddressBox->currentIndex()) == QVariant() && ui->serverButton->text() == tr("Start"))
    {
        QMessageBox msg;
        msg.setWindowTitle(tr("Error"));
        msg.setText(tr("There is no available IP Address for host."));
        msg.setIcon(QMessageBox::Critical);

        msg.exec();

        return;
    }

    if(ui->serverButton->text() != tr("Start"))
    {
        server->close();

        delete server;

        server = NULL;

        ui->serverButton->setText(tr("Start"));
    }
    else
    {
        ui->serverButton->setText(tr("Stop"));

        if(server)
        {
            server->close();

            delete server;

            server = NULL;
        }

        server = new PhoneDataServer(ui->portBox->text().toInt());
    }*/
}

/*PhoneDataServer::PhoneDataServer(quint16 port, QObject* parent)
: QTcpServer(parent)
{
    listen(QHostAddress::Any, port);
}

void PhoneDataServer::incomingConnection(qintptr socketDescriptor)
{
    PhoneDataSocket* socket = new PhoneDataSocket(this);
    socket->setSocketDescriptor(socketDescriptor);

    emit newConnection(socket);
}
*/
