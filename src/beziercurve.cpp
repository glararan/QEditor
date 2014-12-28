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

#include "mathhelper.h"

BezierCurve::BezierCurve(QVector3D& point1, QVector3D& point1viewCenter, QVector3D& point1upVector,
                         QVector3D& point2, QVector3D& point2viewCenter, QVector3D& point2upVector,
                         QVector3D& point3, QVector3D& point3viewCenter, QVector3D& point3upVector,
                         QVector3D& point4, QVector3D& point4viewCenter, QVector3D& point4upVector)
{
    points[0] = point1;
    points[1] = point2;
    points[2] = point3;
    points[3] = point4;

    viewCenter[0] = point1viewCenter;
    viewCenter[1] = point2viewCenter;
    viewCenter[2] = point3viewCenter;
    viewCenter[3] = point4viewCenter;

    upVector[0] = point1upVector;
    upVector[1] = point2upVector;
    upVector[2] = point3upVector;
    upVector[3] = point4upVector;

    if(point3 != QVector3D() && point4 == QVector3D())
        usedPoints = 3;
    else if(point4 != QVector3D())
        usedPoints = 4;
    else
        usedPoints = 2;
}

BezierCurve::~BezierCurve()
{
}

void BezierCurve::init()
{
    QVector<float> positionData(usedPoints * 3); // 3 vertex per point
    QVector<float> positionData2(12);            // 3 vertex per point

    for(int i = 0; i < usedPoints; ++i)
    {
        positionData[i * 3]     = points[i].x();
        positionData[i * 3 + 1] = points[i].y();
        positionData[i * 3 + 2] = points[i].z();

        positionData2[i * 3]     = points[i].x();
        positionData2[i * 3 + 1] = points[i].y();
        positionData2[i * 3 + 2] = points[i].z();
    }

    // if used points are less than 4
    for(int i = usedPoints; i < 4; ++i)
    {
        positionData2[i * 3]     = points[usedPoints - 1].x();
        positionData2[i * 3 + 1] = points[usedPoints - 1].y();
        positionData2[i * 3 + 2] = points[usedPoints - 1].z();
    }

    mesh.createVertexArrayObject();
    mesh.createBuffer(Mesh::Vertices, positionData.data(), positionData.size() * sizeof(float));
    mesh.setNumFaces(usedPoints);

    mesh2.createVertexArrayObject();
    mesh2.createBuffer(Mesh::Vertices, positionData2.data(), positionData2.size() * sizeof(float));
    mesh2.setNumFaces(12);
}

void BezierCurve::drawControlPoints(QOpenGLShaderProgram* shader)
{
    mesh.bind();
    mesh.createAttributeArray(Mesh::Vertices, shader, "position", GL_FLOAT, 0, 3);

    glPointSize(15.0f);
    glDrawArrays(GL_POINTS, 0, mesh.getNumFaces()); // draw 4 points
}

void BezierCurve::drawCurve(QOpenGLShaderProgram* shader)
{
    mesh2.bind();
    mesh2.createAttributeArray(Mesh::Vertices, shader, "vertexPosition", GL_FLOAT, 0, 3);

    shader->setPatchVertexCount(4);

    glLineWidth(5.0f);
    glDrawArrays(GL_PATCHES, 0, mesh2.getNumFaces());
}

QVector3D BezierCurve::calculatePoint(float t) const
{
    float u   = 1 - t;
    float uu  = u * u;
    float uuu = uu * u;

    float tt  = t * t;
    float ttt = tt * t;

    QVector3D pointThree  = points[2];
    QVector3D pointFourth = points[3];

    if(pointThree == QVector3D())
        pointThree = points[1];

    if(pointFourth == QVector3D())
        pointFourth = pointThree;

    QVector3D point = points[0] * uuu;
    point += 3 * uu * t * points[1];
    point += 3 * u * tt * pointThree;
    point += ttt * pointFourth;

    return point;
}

QVector3D BezierCurve::calculateViewCenter(float t) const
{
    float u   = 1 - t;
    float uu  = u * u;
    float uuu = uu * u;

    float tt  = t * t;
    float ttt = tt * t;

    QVector3D viewCenterThree  = viewCenter[2];
    QVector3D viewCenterFourth = viewCenter[3];

    if(viewCenterThree == QVector3D())
        viewCenterThree = viewCenter[1];

    if(viewCenterFourth == QVector3D())
        viewCenterFourth = viewCenterThree;

    QVector3D _viewCenter = viewCenter[0] * uuu;
    _viewCenter += 3 * uu * t * viewCenter[1];
    _viewCenter += 3 * u * tt * viewCenterThree;
    _viewCenter += ttt * viewCenterFourth;

    return _viewCenter;
}

QVector3D BezierCurve::calculateUpVector(float t) const
{
    float u   = 1 - t;
    float uu  = u * u;
    float uuu = uu * u;

    float tt  = t * t;
    float ttt = tt * t;

    QVector3D upVectorThree  = upVector[2];
    QVector3D upVectorFourth = upVector[3];

    if(upVectorThree == QVector3D())
        upVectorThree = upVector[1];

    if(upVectorFourth == QVector3D())
        upVectorFourth = upVectorThree;

    QVector3D _upVector = upVector[0] * uuu;
    _upVector += 3 * uu * t * upVector[1];
    _upVector += 3 * u * tt * upVectorThree;
    _upVector += ttt * upVectorFourth;

    return _upVector;
}
