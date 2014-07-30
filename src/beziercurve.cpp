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

#include "beziercurve.h"

BezierCurve::BezierCurve()
{
}

BezierCurve::~BezierCurve()
{
}

void BezierCurve::init()
{
    for(int i = 1; i < 5; ++i)
    {
        points[i].setX(qrand() % 100 + i);
        points[i].setY(qrand() % 150 + i);
        points[i].setZ(qrand() % 100 + i);
    }

    /*mesh.createVertexArrayObject();
    mesh.createBuffer(IMesh::Vertices, positionData.data(), 4 * sizeof(float));
    mesh.setNumFaces(4);*/
}

void BezierCurve::drawControlPoints(QOpenGLShaderProgram* shader)
{
    /*mesh.bind();
    mesh.createAttributeArray(IMesh::Vertices, shader, "position", GL_FLOAT, 0, 3);

    glDrawArrays(GL_POINTS, 0, 4);*/
}

void BezierCurve::drawCurve(float r, float g, float b)
{

}

void BezierCurve::updateVertex()
{

}
