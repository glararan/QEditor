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

#ifndef IPIPELINE_H
#define IPIPELINE_H

#include <QtCore>
#include <QtOpenGL>
#include <QMatrix4x4>

enum matrixModes
{
    MODEL_MATRIX      = 0,
    VIEW_MATRIX       = 1,
    PROJECTION_MATRIX = 2
};

class IPipeline
{    
public:
    IPipeline();
    IPipeline(QMatrix4x4 model, QMatrix4x4 view, QMatrix4x4 projection);

    bool isShadowEnabled();

    void loadIdentity();

    void matrixMode(int m);

    void translate(float x, float y, float z);

    void scale(float x, float y, float z);
    void scale(float v);

    void rotateX(float angle);
    void rotateY(float angle);
    void rotateZ(float angle);
    void rotate(float angle, float x, float y, float z);

    void lookAt(const QVector3D& eye, const QVector3D& center, const QVector3D& up);
    void lookAt(QVector3D& position, QVector3D& direction);

    void ortho(float left, float right, float bottom, float top, float nearPlane, float farPlane);
    void perspective(float angle, float aRatio, float nearPlane, float farPlane);

    void resize(int width, int height);

    void setLight(QVector3D& position, QVector3D& direction);

    float getHeight() { return viewportSize.y(); }
    float getWidth()  { return viewportSize.x(); }

    QMatrix4x4 getModelMatrix();
    QMatrix4x4 getViewMatrix();
    QMatrix4x4 getModelViewMatrix();
    QMatrix4x4 getProjectionMatrix();
    QMatrix4x4 getModelViewProjectionMatrix();
    QMatrix4x4 getViewPortMatrix();
    QVector2D  getViewportSize();

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

#endif // IPIPELINE_H