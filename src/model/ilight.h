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

#ifndef ILIGHT_H
#define ILIGHT_H

#include <QtCore>
#include <QtWidgets>

struct ILight
{
    ILight(QVector3D position = QVector3D(0.0f, 0.0f, 0.0f), QVector3D ambient = QVector3D(0.3f, 0.3f, 0.3f), QVector3D diffuse = QVector3D(0.6f, 0.6f, 0.6f), QVector3D specular = QVector3D(1.0f, 1.0f, 1.0f));

    QVector3D position;
    QVector3D ambient;
    QVector3D diffuse;
    QVector3D specular;
};

#endif // ILIGHT_H