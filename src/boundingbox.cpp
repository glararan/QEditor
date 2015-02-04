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

#include "boundingbox.h"

BoundingBox::BoundingBox(const QVector4D& bbColor)
: color(bbColor)
, initialized(false)
{
}

BoundingBox::BoundingBox(const QVector3D& minimum, const QVector3D& maximum, const QVector4D& bbColor)
: color(bbColor)
, initialized(false)
{
    initialize(minimum, maximum);
}

void BoundingBox::initialize(const QVector3D& minimum, const QVector3D& maximum)
{
    if(initialized)
        return;

    initialized = true;

    reallocate(minimum, maximum);
}

void BoundingBox::reallocate(const QVector3D& minimum, const QVector3D& maximum)
{
    min = minimum;
    max = maximum;

    QVector<float> positionData(10 * 3); // 10 points * 3 vertex per point
    QVector<float> positionData2(2 * 3); // 2 points * 3 vertex per point
    QVector<float> positionData3(2 * 3); // 2 points * 3 vertex per point
    QVector<float> positionData4(2 * 3); // 2 points * 3 vertex per point

    add(positionData, 0, min.x(), max.y(), min.z());
    add(positionData, 3, min.x(), min.y(), min.z());
    add(positionData, 6, max.x(), min.y(), min.z());
    add(positionData, 9, max.x(), min.y(), max.z());
    add(positionData, 12, max.x(), max.y(), max.z());
    add(positionData, 15, max.x(), max.y(), min.z());
    add(positionData, 18, min.x(), max.y(), min.z());
    add(positionData, 21, min.x(), max.y(), max.z());
    add(positionData, 24, min.x(), min.y(), max.z());
    add(positionData, 27, min.x(), min.y(), min.z());

    add(positionData2, 0, min.x(), min.y(), max.z());
    add(positionData2, 3, max.x(), min.y(), max.z());

    add(positionData3, 0, max.x(), max.y(), min.z());
    add(positionData3, 3, max.x(), min.y(), min.z());

    add(positionData4, 0, min.x(), max.y(), max.z());
    add(positionData4, 3, max.x(), max.y(), max.z());

    mainMesh.createVertexArrayObject();
    mainMesh.createBuffer(Mesh::Vertices, positionData.data(), positionData.size() * sizeof(float));
    mainMesh.setNumFaces(10);

    mesh2.createVertexArrayObject();
    mesh2.createBuffer(Mesh::Vertices, positionData2.data(), positionData2.size() * sizeof(float));
    mesh2.setNumFaces(2);

    mesh3.createVertexArrayObject();
    mesh3.createBuffer(Mesh::Vertices, positionData3.data(), positionData3.size() * sizeof(float));
    mesh3.setNumFaces(2);

    mesh4.createVertexArrayObject();
    mesh4.createBuffer(Mesh::Vertices, positionData4.data(), positionData4.size() * sizeof(float));
    mesh4.setNumFaces(2);
}

void BoundingBox::draw(QOpenGLShaderProgram* shader, const QMatrix4x4& view, const QMatrix4x4& proj)//const QMatrix4x4& mvp)
{
    if(min == max)
        return;

    glEnable(GL_LINE_SMOOTH);
    glLineWidth(1.0f);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    /// Set shader
    shader->bind();

    Pipeline pip(QMatrix4x4(), view, proj);
    pip.translate(translateVec.x(), translateVec.y(), translateVec.z());
    pip.scale(scaleVec.x(), scaleVec.y(), scaleVec.z());
    pip.rotateX(rotationVec.x() * 360.0f);
    pip.rotateY(rotationVec.y() * 360.0f);
    pip.rotateZ(rotationVec.z() * 360.0f);
    pip.updateMatrices(shader);

    shader->setUniformValue("color", color);

    /// Main draw
    mainMesh.bind();
    mainMesh.createAttributeArray(Mesh::Vertices, shader, "position", GL_FLOAT, 0, 3);

    glDrawArrays(GL_LINE_STRIP, 0, mainMesh.getNumFaces());

    /// draw second line
    mesh2.bind();
    mesh2.createAttributeArray(Mesh::Vertices, shader, "position", GL_FLOAT, 0, 3);

    glDrawArrays(GL_LINES, 0, mesh2.getNumFaces());

    /// draw third line
    mesh3.bind();
    mesh3.createAttributeArray(Mesh::Vertices, shader, "position", GL_FLOAT, 0, 3);

    glDrawArrays(GL_LINES, 0, mesh3.getNumFaces());

    /// draw fourth line
    mesh4.bind();
    mesh4.createAttributeArray(Mesh::Vertices, shader, "position", GL_FLOAT, 0, 3);

    glDrawArrays(GL_LINES, 0, mesh4.getNumFaces());

    /// Set default GL
    glDisable(GL_LINE_SMOOTH); // no need to set gl line to default, 1.0f is already default
}

void BoundingBox::add(QVector<float>& data, const int& index, const float& x, const float& y, const float& z)
{
    data[index + 0] = x;
    data[index + 1] = y;
    data[index + 2] = z;
}
