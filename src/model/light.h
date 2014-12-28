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

#ifndef LIGHT_H
#define LIGHT_H

#include <QtCore>
#include <QtWidgets>

struct Light
{
    Light(const QVector3D position = QVector3D(0.0f, 0.0f, 0.0f), const QVector3D ambient = QVector3D(0.3f, 0.3f, 0.3f), const QVector3D diffuse = QVector3D(0.6f, 0.6f, 0.6f), const QVector3D specular = QVector3D(1.0f, 1.0f, 1.0f));

    QVector3D position;
    QVector3D ambient;
    QVector3D diffuse;
    QVector3D specular;
};

#endif // LIGHT_H
