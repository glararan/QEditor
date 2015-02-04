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

#ifndef BOUNDINGBOX_H
#define BOUNDINGBOX_H

#include <QVector4D>
#include <QVector3D>
#include <QVector>
#include <QOpenGLShaderProgram>

#include "mesh.h"
#include "pipeline.h"

class BoundingBox
{
public:
    BoundingBox(const QVector4D& bbColor = QVector4D(1.0f, 1.0f, 1.0f, 1.0f));
    BoundingBox(const QVector3D& minimum, const QVector3D& maximum, const QVector4D& bbColor = QVector4D(1.0f, 1.0f, 1.0f, 1.0f));

    void initialize(const QVector3D& minimum, const QVector3D& maximum);
    void reallocate(const QVector3D& minimum, const QVector3D& maximum);

    void draw(QOpenGLShaderProgram* shader, const QMatrix4x4& view, const QMatrix4x4& proj);

    void rotation(const QVector3D& vector)  { rotationVec = vector; }
    void scale(const QVector3D& vector)     { scaleVec = vector; }
    void translate(const QVector3D& vector) { translateVec = vector; }

    const QVector3D getCenter() const { return (max + min) / 2; }

    const bool isInitialized() const { return initialized; }

private:
    QVector3D min, max;
    QVector4D color;

    QVector3D rotationVec, scaleVec, translateVec;

    Mesh mainMesh, mesh2, mesh3, mesh4;

    QMatrix4x4 mvpMatrix;

    Pipeline pipeline;

    bool initialized;

    void add(QVector<float>& data, const int& index, const float& x, const float& y, const float& z);
    void updateMatrix(const QMatrix4x4& mvp);
};

#endif // BOUNDINGBOX_H
