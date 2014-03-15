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

class BezierCurve
{
public:
    BezierCurve();
    ~BezierCurve();

    void init();

    void drawControlPoints();
    void drawCurve(float r, float g, float b);

    void updateVertex();

    QVector3D points[5];
};

extern BezierCurve* bezierCurve;

#endif // BEZIERCURVE_H