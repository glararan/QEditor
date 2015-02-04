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

#ifndef MODELHEADER_H
#define MODELHEADER_H

#include <QVector3D>

struct ModelHeader
{
    QVector3D vertexBoxMin, vertexBoxMax;
    QVector3D boundingBoxMin, boundingBoxMax;

    QVector3D center;

    const QVector3D& getVertexBoxRadius() const   { (vertexBoxMax + vertexBoxMin) / 2; }
    const QVector3D& getBoundingBoxRadius() const { (boundingBoxMin + boundingBoxMax) / 2; }

    const QVector3D& getCenter() const { return center; }
};

#endif // MODELHEADER_H
