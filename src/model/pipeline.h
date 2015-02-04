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

#ifndef PIPELINE_H
#define PIPELINE_H

#include <QtCore>
#include <QtOpenGL>
#include <QMatrix4x4>

enum matrixModes
{
    MODEL_MATRIX      = 0,
    VIEW_MATRIX       = 1,
    PROJECTION_MATRIX = 2
};

class Pipeline
{    
public:
    Pipeline();
    Pipeline(const QMatrix4x4 model, const QMatrix4x4 view, const QMatrix4x4 projection);

    const bool isShadowEnabled() const { return false; }

    void loadIdentity();

    void matrixMode(const int m);

    void translate(const float x, const float y, const float z);

    void scale(const float x, const float y, const float z);
    void scale(const float v);

    void rotateX(const float angle);
    void rotateY(const float angle);
    void rotateZ(const float angle);
    void rotate(const float angle, const float x, const float y, const float z);

    void lookAt(const QVector3D& eye, const QVector3D& center, const QVector3D& up);
    void lookAt(const QVector3D& position, QVector3D& direction);

    void ortho(const float left, const float right, const float bottom, const float top, const float nearPlane, const float farPlane);
    void perspective(const float angle, const float aRatio, const float nearPlane, const float farPlane);

    void resize(const int width, const int height);

    void setLight(const QVector3D& position, QVector3D& direction);
    void setCamera(const QMatrix4x4& model, const QMatrix4x4& view, const QMatrix4x4& proj);

    const float getHeight() const { return viewportSize.y(); }
    const float getWidth() const  { return viewportSize.x(); }

    const QMatrix4x4 getModelViewMatrix() const;
    const QMatrix4x4 getModelViewProjectionMatrix() const;

    const QMatrix4x4 getModelMatrix() const      { return modelMatrix[modelMatrix.size() - 1]; }
    const QMatrix4x4 getViewMatrix() const       { return viewMatrix[viewMatrix.size() - 1]; }
    const QMatrix4x4 getProjectionMatrix() const { return projectionMatrix[projectionMatrix.size() - 1]; }
    const QMatrix4x4 getViewPortMatrix() const   { return viewportMatrix; }
    const QVector2D  getViewportSize() const     { return viewportSize; }

    void pushMatrix();
    void popMatrix();

    void updateMatrices(QOpenGLShaderProgram* shader);

private:
    int currentMatrix;

    QVector<QMatrix4x4> modelMatrix;
    QVector<QMatrix4x4> viewMatrix;
    QVector<QMatrix4x4> projectionMatrix;
    QVector<QMatrix4x4> skyboxMatrix;

    QMatrix4x4 viewportMatrix;
    QVector2D  viewportSize;

    bool matricesReady;

    QMatrix4x4 modelViewMatrix;
    QMatrix4x4 modelViewProjectionMatrix;
    QMatrix3x3 normalMatrix;
    QMatrix3x3 worldNormalMatrix;

    // shadowmapping
    QMatrix4x4 biasMatrix;
    QMatrix4x4 lightViewMatrix;
    QMatrix4x4 lightProjectionMatrix;
    QMatrix4x4 lightModelMatrix;
    QMatrix4x4 lightMatrix;

    QVector3D lightPosition;
};

#endif // PIPELINE_H
