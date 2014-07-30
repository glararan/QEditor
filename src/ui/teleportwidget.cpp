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

#include "teleportwidget.h"
#include "ui_teleportwidget.h"

#include <QMessageBox>
#include <QVector3D>
#include <QFile>
#include <QTextStream>
#include <QDebug>

TeleportWidget::TeleportWidget(QWidget* parent) : QDialog(parent), ui(new Ui::TeleportWidget)
{
    ui->setupUi(this);

    /// load data
    loadBookmarks();

    /// connects
    connect(ui->bookmarksAdd, SIGNAL(clicked()),  this, SLOT(bookmarksAdd()));
    connect(ui->bookmarksGo,  SIGNAL(clicked()),  this, SLOT(bookmarksGo()));
    connect(ui->buttonBox,    SIGNAL(accepted()), this, SLOT(bookmarksTeleport()));
}

TeleportWidget::~TeleportWidget()
{
    delete ui;
}

void TeleportWidget::bookmarksAdd()
{
    QStringList list;
    list << ui->bookmarksLocation->text() << ui->bookmarksX->text() << ui->bookmarksY->text() << ui->bookmarksZ->text();

    for(int i = 0; i < list.count(); i++)
    {
        if(list[i] == "")
        {
            QMessageBox msg;
            msg.setDefaultButton(QMessageBox::Ok);
            msg.setText(tr("Some field is empty!"));
            msg.setWindowTitle("Error");
            msg.exec();

            return;
        }

        if(i != 0)
        {
            if(checkCoords(list[i]))
            {
                QMessageBox msg;
                msg.setDefaultButton(QMessageBox::Ok);
                msg.setText(tr("Some coordinate is not allowed to use!"));
                msg.setWindowTitle("Error");
                msg.exec();

                return;
            }
        }
    }

    QVector3D location = QVector3D(ui->bookmarksX->text().toFloat(), ui->bookmarksZ->text().toFloat(), ui->bookmarksY->text().toFloat());

    writeToBookmarks(ui->bookmarksLocation->text(), location);

    ui->bookmarksList->addItem(ui->bookmarksLocation->text(), location);

    ui->bookmarksLocation->text().clear();
    ui->bookmarksX->text().clear();
    ui->bookmarksY->text().clear();
    ui->bookmarksZ->text().clear();
}

void TeleportWidget::bookmarksGo()
{
    QVector3D location(0.0f, 0.0f, 0.0f);

    if(ui->bookmarksList->currentText() == "")
    {
        QMessageBox msg;
        msg.setDefaultButton(QMessageBox::Ok);
        msg.setText(tr("Selected item is empty!"));
        msg.setWindowTitle("Error");
        msg.exec();

        return;
    }

    QVariant var = ui->bookmarksList->itemData(ui->bookmarksList->currentIndex());
    var.convert(QMetaType::QVector3D);

    location = var.value<QVector3D>();

    if(location.isNull())
    {
        QMessageBox msg;
        msg.setDefaultButton(QMessageBox::Ok);
        msg.setText(tr("We don't localize this item!"));
        msg.setWindowTitle("Error");
        msg.exec();

        return;
    }

    emit TeleportTo(&location);
}

void TeleportWidget::bookmarksTeleport()
{
    QVector3D location(0.0f, 0.0f, 0.0f);

    if(ui->bookmarksX->text() == "")
    {
        QMessageBox msg;
        msg.setDefaultButton(QMessageBox::Ok);
        msg.setText(tr("Coordinate X is empty!"));
        msg.setWindowTitle("Error");
        msg.exec();

        return;
    }
    else
    {
        if(checkCoords(ui->bookmarksX->text()))
            location.setX(ui->bookmarksX->text().toFloat());
        else
        {
            QMessageBox msg;
            msg.setDefaultButton(QMessageBox::Ok);
            msg.setText(tr("Coordinate X is not allowed to use!"));
            msg.setWindowTitle("Error");
            msg.exec();

            return;
        }
    }

    if(ui->bookmarksY->text() == "")
    {
        QMessageBox msg;
        msg.setDefaultButton(QMessageBox::Ok);
        msg.setText(tr("Coordinate Y is empty!"));
        msg.setWindowTitle("Error");
        msg.exec();

        return;
    }
    else
    {
        if(checkCoords(ui->bookmarksY->text()))
            location.setZ(ui->bookmarksY->text().toFloat());
        else
        {
            QMessageBox msg;
            msg.setDefaultButton(QMessageBox::Ok);
            msg.setText(tr("Coordinate Y is not allowed to use!"));
            msg.setWindowTitle("Error");
            msg.exec();

            return;
        }
    }

    if(ui->bookmarksZ->text() == "")
    {
        QMessageBox msg;
        msg.setDefaultButton(QMessageBox::Ok);
        msg.setText(tr("Coordinate Z is empty!"));
        msg.setWindowTitle("Error");
        msg.exec();

        return;
    }
    else
    {
        if(checkCoords(ui->bookmarksZ->text()))
            location.setY(ui->bookmarksZ->text().toFloat());
        else
        {
            QMessageBox msg;
            msg.setDefaultButton(QMessageBox::Ok);
            msg.setText(tr("Coordinate Z is not allowed to use!"));
            msg.setWindowTitle("Error");
            msg.exec();

            return;
        }
    }

    ui->bookmarksLocation->text().clear();
    ui->bookmarksX->text().clear();
    ui->bookmarksY->text().clear();
    ui->bookmarksZ->text().clear();

    emit TeleportTo(&location);
}

bool TeleportWidget::checkCoords(QString coords)
{
    QVector<QChar> allowedChars;
    allowedChars << '0' << '1' << '2' << '3' << '4' << '5' << '6' << '7' << '8' << '9' << '.' << ',';

    bool dotComma = false;

    for(int i = 0; i < coords.length(); i++)
    {
        for(int j = 0; j < allowedChars.count(); j++)
        {
            if(coords[i] == allowedChars[j])
            {
                if(allowedChars[j] == '.' || allowedChars[j] == ',')
                {
                    if(dotComma)
                    {
                        QMessageBox msg;
                        msg.setDefaultButton(QMessageBox::Ok);
                        msg.setText(tr("Coords contain more then 1 dot or comma!"));
                        msg.setWindowTitle("Error");
                        msg.exec();

                        return false;
                    }
                    else
                        dotComma = true;
                }

                break;
            }
            else
            {
                if(j + 1 >= allowedChars.count())
                    return false;
            }
        }
    }

    return true;
}

void TeleportWidget::writeToBookmarks(QString name, QVector3D location)
{
    if(readFromBookmarks(name, location))
    {
        QMessageBox msg;
        msg.setDefaultButton(QMessageBox::Ok);
        msg.setText(tr("In bookmarks list already exists location with this name!"));
        msg.setWindowTitle("Error");
        msg.exec();

        return;
    }

    while(name.endsWith(' '))
    {
        QString cName = name;

        name.clear();

        for(int i = 0; i < name.length() - 2; i++)
            name[i] = cName[i];
    }

    QFile file("bookmarks.txt");
    file.open(QIODevice::WriteOnly | QIODevice::Append);

    QTextStream tStream(&file);
    tStream << name << " " << location.x() << " " << location.z() << " " << location.y() << "\n";

    file.close();
}

bool TeleportWidget::readFromBookmarks(QString search, QVector3D& location)
{
    QFile file("bookmarks.txt");
    file.open(QIODevice::ReadOnly);

    QTextStream tStream(&file);

    QString bookmarksAll = tStream.readAll();
    QStringList bookmarksLines = bookmarksAll.split("\n");

    file.close();

    bool found = false;

    if(bookmarksLines.count() > 0)
    {
        foreach(QString line, bookmarksLines)
        {
            QStringList fields = line.split(" ");

            if(line.length() <= 0 || fields.count() != 4)
                continue;

            if(fields[0] == search)
            {
                for(int i = 1; i < fields.count(); i++)
                {
                    if(fields[i].contains(","))
                        fields[i] = fields[i].replace(",", ".");
                }

                location = QVector3D(fields[1].toFloat(), fields[2].toFloat(), fields[3].toFloat());

                return true;
            }
        }
    }

    return found;
}

void TeleportWidget::loadBookmarks()
{
    QFile file("bookmarks.txt");

    if(file.exists() && file.open(QIODevice::ReadOnly))
    {
        QTextStream tStream(&file);

        QString bookmarksAll = tStream.readAll();
        QStringList bookmarksLines = bookmarksAll.split("\n");

        file.close();

        if(bookmarksLines.count() > 0)
        {
            foreach(QString line, bookmarksLines)
            {
                QStringList fields = line.split(" ");

                if(line.length() <= 0 || fields.count() != 4)
                    continue;

                for(int i = 1; i < fields.count(); i++)
                {
                    if(fields[i].contains(","))
                        fields[i] = fields[i].replace(",", ".");
                }

                //                        x                    z                    y
                QVector3D location(fields[1].toFloat(), fields[3].toFloat(), fields[2].toFloat());

                ui->bookmarksList->addItem(fields[0], location);
            }
        }
    }
    else
        qDebug() << tr("Bookmarks doesn't exists.");
}