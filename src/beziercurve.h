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

#ifndef BEZIERCURVE_H
#define BEZIERCURVE_H

#include "mathhelper.h"
#include "mesh.h"

#include <QVector3D>
#include <QVector2D>

class BezierCurve
{
public:
    BezierCurve(QVector3D& point1, QVector3D& point1viewCenter, QVector3D& point1upVector,
                QVector3D& point2, QVector3D& point2viewCenter, QVector3D& point2upVector,
                QVector3D& point3 = QVector3D(), QVector3D& point3viewCenter = QVector3D(), QVector3D& point3upVector = QVector3D(),
                QVector3D& point4 = QVector3D(), QVector3D& point4viewCenter = QVector3D(), QVector3D& point4upVector = QVector3D());
    ~BezierCurve();

    void init();

    void drawControlPoints(QOpenGLShaderProgram* shader);
    void drawCurve(QOpenGLShaderProgram* shader);

    QVector3D calculatePoint(float t) const;
    QVector3D calculateViewCenter(float t) const;
    QVector3D calculateUpVector(float t) const;

    QVector3D points[4];
    QVector3D viewCenter[4];
    QVector3D upVector[4];

private:
    Mesh mesh, mesh2;

    int usedPoints;
};

extern BezierCurve* bezierCurve;

#endif // BEZIERCURVE_H
