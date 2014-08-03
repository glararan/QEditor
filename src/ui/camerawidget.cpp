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

#include "camerawidget.h"
#include "ui_camerawidget.h"

#include "beziercurve.h"

#include <QTableWidgetItem>
#include <QMessageBox>

CameraWidget::CameraWidget(Camera* cam, QWidget* parent)
: QDialog(parent)
, ui(new Ui::CameraWidget)
, camera(cam)
, playing(false)
{
    ui->setupUi(this);

    ui->tableWidget->setHorizontalHeaderLabels(QStringList() << tr("Index") << "X" << "Y" << "Z" << "X orientation" << "Y orientation");
    ui->tableWidget->setColumnCount(Count);

    connect(ui->tableWidget, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(itemChanged(QTableWidgetItem*)));

    connect(ui->showCurveBox, SIGNAL(stateChanged(int)), this, SLOT(setShowPath(int)));

    connect(ui->addButton,    SIGNAL(clicked()), this, SLOT(addPosition()));
    connect(ui->setButton,    SIGNAL(clicked()), this, SLOT(setPosition()));
    connect(ui->deleteButton, SIGNAL(clicked()), this, SLOT(deletePosition()));

    connect(ui->playButton,  SIGNAL(clicked()), this, SLOT(play()));
    connect(ui->stopButton,  SIGNAL(clicked()), this, SLOT(stop()));
    connect(ui->clearButton, SIGNAL(clicked()), this, SLOT(clear()));
}

CameraWidget::~CameraWidget()
{
    delete ui;
}

void CameraWidget::selectPoint(const QVector3D& position)
{
    QTableWidget* table = ui->tableWidget;

    for(int i = 0; i < table->rowCount(); ++i)
    {
        if(position.x() - 1.0f < table->item(i, PositionX)->data(Qt::DisplayRole).toFloat()
        && position.x() + 1.0f > table->item(i, PositionX)->data(Qt::DisplayRole).toFloat()
        && position.y() - 1.0f < table->item(i, PositionZ)->data(Qt::DisplayRole).toFloat()
        && position.y() + 1.0f > table->item(i, PositionZ)->data(Qt::DisplayRole).toFloat()
        && position.z() - 1.0f < table->item(i, PositionY)->data(Qt::DisplayRole).toFloat()
        && position.z() + 1.0f > table->item(i, PositionY)->data(Qt::DisplayRole).toFloat())
        {
            QVector<QTableWidgetItem*> items;
            items.append(table->item(i, PositionX));
            items.append(table->item(i, PositionY));
            items.append(table->item(i, PositionZ));

            emit selectedPoint(items);

            break;
        }
    }
}

void CameraWidget::itemChanged(QTableWidgetItem* item)
{
    Q_UNUSED(item);

    generateBezierCurves(0);
}

void CameraWidget::setShowPath(int state)
{
    switch(state)
    {
        case 0:
        default:
            emit showPath(false);
            break;

        case 2:
            emit showPath(true);
            break;
    }
}

void CameraWidget::addPosition()
{
    int row = ui->tableWidget->selectionModel()->currentIndex().row();

    if(row < 0)
    {
        ui->tableWidget->insertRow(ui->tableWidget->rowCount());
        ui->tableWidget->blockSignals(true); // We don't need to fire itemChanged signal

        QTableWidgetItem* itemIndex = new QTableWidgetItem(Qt::DisplayRole);
        QTableWidgetItem* itemX     = new QTableWidgetItem(Qt::DisplayRole);
        QTableWidgetItem* itemY     = new QTableWidgetItem(Qt::DisplayRole);
        QTableWidgetItem* itemZ     = new QTableWidgetItem(Qt::DisplayRole);

        itemIndex->setData(Qt::DisplayRole, ui->tableWidget->rowCount() - 1);
        itemX->setData(Qt::DisplayRole,     camera->position().x());
        itemY->setData(Qt::DisplayRole,     camera->position().y());
        itemZ->setData(Qt::DisplayRole,     camera->position().z());

        ui->tableWidget->setItem(ui->tableWidget->rowCount() - 1, Index,     itemIndex);
        ui->tableWidget->setItem(ui->tableWidget->rowCount() - 1, PositionX, itemX);
        ui->tableWidget->setItem(ui->tableWidget->rowCount() - 1, PositionY, itemZ);
        ui->tableWidget->setItem(ui->tableWidget->rowCount() - 1, PositionZ, itemY);

        viewCenter.append(camera->viewCenter());
        upVector.append(camera->upVector());

        ui->tableWidget->blockSignals(false);
    }
    else
    {
        ui->tableWidget->insertRow(row);
        ui->tableWidget->blockSignals(true); // We don't need to fire itemChanged signal

        QTableWidgetItem* itemIndex = new QTableWidgetItem(Qt::DisplayRole);
        QTableWidgetItem* itemX     = new QTableWidgetItem(Qt::DisplayRole);
        QTableWidgetItem* itemY     = new QTableWidgetItem(Qt::DisplayRole);
        QTableWidgetItem* itemZ     = new QTableWidgetItem(Qt::DisplayRole);

        itemIndex->setData(Qt::DisplayRole, row);
        itemX->setData(Qt::DisplayRole,     camera->position().x());
        itemY->setData(Qt::DisplayRole,     camera->position().y());
        itemZ->setData(Qt::DisplayRole,     camera->position().z());

        ui->tableWidget->setItem(row, Index,     itemIndex);
        ui->tableWidget->setItem(row, PositionX, itemX);
        ui->tableWidget->setItem(row, PositionY, itemZ);
        ui->tableWidget->setItem(row, PositionZ, itemY);

        viewCenter.insert(row, camera->viewCenter());
        upVector.insert(row, camera->upVector());

        for(int i = row + 1; i < ui->tableWidget->rowCount(); ++i)
        {
            QTableWidgetItem* itemIndex = ui->tableWidget->item(i, Index);

            itemIndex->setData(Qt::DisplayRole, i);
        }

        ui->tableWidget->blockSignals(false);
    }

    generateBezierCurves(0);

    if(!ui->playButton->isEnabled())
        ui->playButton->setEnabled(true);

    if(!ui->setButton->isEnabled())
        ui->setButton->setEnabled(true);

    if(!ui->deleteButton->isEnabled())
        ui->deleteButton->setEnabled(true);

    if(!ui->clearButton->isEnabled())
        ui->clearButton->setEnabled(true);
}

void CameraWidget::setPosition()
{
    int row = ui->tableWidget->selectionModel()->currentIndex().row();

    if(row < 0)
    {
        QMessageBox msg;
        msg.setText(tr("You didn't selected any row!"));
        msg.setWindowTitle("Error");
        msg.setIcon(QMessageBox::Critical);

        msg.exec();

        return;
    }

    ui->tableWidget->blockSignals(true); // We don't need to fire itemChanged signal

    QTableWidgetItem* itemX = ui->tableWidget->item(row, PositionX);
    QTableWidgetItem* itemY = ui->tableWidget->item(row, PositionZ);
    QTableWidgetItem* itemZ = ui->tableWidget->item(row, PositionY);

    itemX->setData(Qt::DisplayRole, camera->position().x());
    itemY->setData(Qt::DisplayRole, camera->position().y());
    itemZ->setData(Qt::DisplayRole, camera->position().z());

    viewCenter[row] = camera->viewCenter();
    upVector[row]   = camera->upVector();

    ui->tableWidget->blockSignals(false);

    generateBezierCurves(0);
}

void CameraWidget::deletePosition()
{
    int row = ui->tableWidget->selectionModel()->currentIndex().row();

    if(row < 0)
        ui->tableWidget->removeRow(0);
    else
        ui->tableWidget->removeRow(row);

    ui->tableWidget->blockSignals(true); // We don't need to fire itemChanged signal

    for(int i = 0; i < ui->tableWidget->rowCount(); ++i)
    {
        QTableWidgetItem* itemIndex = ui->tableWidget->item(i, Index);

        itemIndex->setData(Qt::DisplayRole, i);
    }

    ui->tableWidget->blockSignals(false);

    if(ui->tableWidget->rowCount() < 1)
    {
        if(playing)
            stop();

        ui->setButton->setEnabled(false);
        ui->deleteButton->setEnabled(false);
        ui->playButton->setEnabled(false);
        ui->stopButton->setEnabled(false);
        ui->clearButton->setEnabled(false);
    }

    generateBezierCurves(0);
}

void CameraWidget::play()
{
    QTableWidget* table = ui->tableWidget;

    if(table->rowCount() < 1)
        return;

    ui->stopButton->setEnabled(true);
    ui->playButton->setEnabled(false);

    playing = true;

    int row = table->selectionModel()->currentIndex().row();

    if(row < 0)
        row = 0;

    if(table->rowCount() < 2)
    {
        QMessageBox msg;
        msg.setText(tr("Not enough points for camera, minimum is 2!"));
        msg.setIcon(QMessageBox::Critical);
        msg.setWindowTitle("Error");

        msg.exec();

        return;
    }

    generateBezierCurves(row);

    camera->play(ui->timeBox->value());
}

void CameraWidget::stop()
{
    ui->playButton->setEnabled(true);
    ui->stopButton->setEnabled(false);

    playing = false;

    camera->stop();
}

void CameraWidget::clear()
{
    if(playing)
        stop();

    for(int i = ui->tableWidget->rowCount(); i >= 0; --i)
        ui->tableWidget->removeRow(i);

    ui->setButton->setEnabled(false);
    ui->deleteButton->setEnabled(false);
    ui->playButton->setEnabled(false);
    ui->stopButton->setEnabled(false);
    ui->clearButton->setEnabled(false);

    camera->deleteCurves();
}

void CameraWidget::generateBezierCurves(int row)
{
    QTableWidget* table = ui->tableWidget;

    QVector<QVector3D> positions;
    QVector<QVector3D> viewCenters;
    QVector<QVector3D> upVectors;

    for(int i = row; i < table->rowCount(); ++i)
    {
        positions.append(QVector3D(table->item(i, PositionX)->data(Qt::DisplayRole).toFloat(),
                                   table->item(i, PositionZ)->data(Qt::DisplayRole).toFloat(),
                                   table->item(i, PositionY)->data(Qt::DisplayRole).toFloat()));

        viewCenters.append(viewCenter.at(i));
        upVectors.append(upVector.at(i));
    }

    if(positions.count() < 2)
        return;

    camera->deleteCurves();

    QVector<BezierCurve*> curves;

    int total = positions.count();

    int four = total / 4;
    total   -= four * 4;

    int three = total / 3;
    total    -= three * 3;

    int two = total / 2;
    total  -= two * 2;

    int one = total;
    total  -= one;

    total = one + two + three + four;

    for(int i = 0; i < total; ++i)
    {
        BezierCurve* curve;

        if(i * 4 + 3 < positions.count() && i == 0)
            curve = new BezierCurve(positions[i * 4], viewCenters[i * 4], upVectors[i * 4],
                                    positions[i * 4 + 1], viewCenters[i * 4 + 1], upVectors[i * 4 + 1],
                                    positions[i * 4 + 2], viewCenters[i * 4 + 2], upVectors[i * 4 + 2],
                                    positions[i * 4 + 3], viewCenters[i * 4 + 3], upVectors[i * 4 + 3]);
        else if(i * 4 + 2 < positions.count())
        {
            if(i == 0)
                curve = new BezierCurve(positions[i * 4], viewCenters[i * 4], upVectors[i * 4],
                                        positions[i * 4 + 1], viewCenters[i * 4 + 1], upVectors[i * 4 + 1],
                                        positions[i * 4 + 2], viewCenters[i * 4 + 2], upVectors[i * 4 + 2]);
            else
                curve = new BezierCurve(positions[i * 4 - 1], viewCenters[i * 4 - 1], upVectors[i * 4 - 1],
                                        positions[i * 4], viewCenters[i * 4], upVectors[i * 4],
                                        positions[i * 4 + 1], viewCenters[i * 4 + 1], upVectors[i * 4 + 1],
                                        positions[i * 4 + 2], viewCenters[i * 4 + 2], upVectors[i * 4 + 2]);
        }
        /*{
            for(int j = 0; j < 3; ++j)
                curve->points[j] = positions[i * 4 + j];

            curve->points[3] = positions[i * 4 + 2];
        }*/
        else if(i * 4 + 1 < positions.count())
        {
            if(i == 0)
                curve = new BezierCurve(positions[i * 4], viewCenters[i * 4], upVectors[i * 4],
                                        positions[i * 4 + 1], viewCenters[i * 4 + 1], upVectors[i * 4 + 1]);
            else
                curve = new BezierCurve(positions[i * 4 - 1], viewCenters[i * 4 - 1], upVectors[i * 4 - 1],
                                        positions[i * 4], viewCenters[i * 4], upVectors[i * 4],
                                        positions[i * 4 + 1], viewCenters[i * 4 + 1], upVectors[i * 4 + 1]);
        }
        /*{
            for(int j = 0; j < 2; ++j)
                curve->points[j] = positions[i * 4 + j];

            curve->points[2] = positions[i * 4 + 1];
            curve->points[3] = positions[i * 4 + 1];
        }*/
        else
            curve = new BezierCurve(positions[i * 4 - 1], viewCenters[i * 4 - 1], upVectors[i * 4 - 1],
                                    positions[i * 4], viewCenters[i * 4], upVectors[i * 4]);
        /*{
            curve->points[0] = positions[i * 4 - 1];
            curve->points[1] = positions[i * 4];
            curve->points[2] = positions[i * 4];
            curve->points[3] = positions[i * 4];
        }*/

        curve->init();

        curves.append(curve);
    }

    camera->setCurves(curves);
}
