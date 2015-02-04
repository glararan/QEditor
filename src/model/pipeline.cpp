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

#include "pipeline.h"
#include "mathhelper.h"

Pipeline::Pipeline()
{
    modelMatrix.push_back(QMatrix4x4());
    viewMatrix.push_back(QMatrix4x4());
    projectionMatrix.push_back(QMatrix4x4());
    skyboxMatrix.push_back(QMatrix4x4());

    modelViewMatrix           = QMatrix4x4();
    modelViewProjectionMatrix = QMatrix4x4();
    normalMatrix              = QMatrix3x3();
    worldNormalMatrix         = QMatrix3x3();
    matricesReady             = true;
    currentMatrix             = 0;

    biasMatrix = QMatrix4x4(0.5f, 0.0f, 0.0f, 0.5f,
                            0.0f, 0.5f, 0.0f, 0.5f,
                            0.0f, 0.0f, 0.5f, 0.5f,
                            0.0f, 0.0f, 0.0f, 1.0f);
}

Pipeline::Pipeline(const QMatrix4x4 model, const QMatrix4x4 view, const QMatrix4x4 projection)
{
    modelMatrix.push_back(model);
    viewMatrix.push_back(view);
    projectionMatrix.push_back(projection);

    modelViewMatrix           = QMatrix4x4();
    modelViewProjectionMatrix = QMatrix4x4();
    normalMatrix              = QMatrix3x3();
    worldNormalMatrix         = QMatrix3x3();
    matricesReady             = true;
    currentMatrix             = 0;
}

void Pipeline::loadIdentity()
{
    if(currentMatrix == MODEL_MATRIX || currentMatrix == VIEW_MATRIX)
    {
        modelMatrix[modelMatrix.size() - 1] = QMatrix4x4();
        viewMatrix[viewMatrix.size() - 1]   = QMatrix4x4();
    }
    else
        projectionMatrix[projectionMatrix.size() - 1] = QMatrix4x4();

    matricesReady=false;
}

void Pipeline::matrixMode(const int m)
{
    if(m == MODEL_MATRIX || m == VIEW_MATRIX || m == PROJECTION_MATRIX)
        currentMatrix=m;
}

void Pipeline::translate(const float x, const float y, const float z)
{
    if(currentMatrix == MODEL_MATRIX)
        modelMatrix[modelMatrix.size() - 1].translate(x, y, z);

    else if(currentMatrix == VIEW_MATRIX)
        viewMatrix[viewMatrix.size() - 1].translate(-x, -y, -z);

    matricesReady = false;
}

void Pipeline::scale(const float x, const float y, const float z)
{
    if(currentMatrix == MODEL_MATRIX)
        modelMatrix[modelMatrix.size() - 1].scale(x, y, z);
    else if(currentMatrix == VIEW_MATRIX)
        viewMatrix[viewMatrix.size() - 1].scale(x, y, z);

    matricesReady = false;
}

void Pipeline::scale(const float v)
{
    if(currentMatrix == MODEL_MATRIX)
        modelMatrix[modelMatrix.size() - 1].scale(v, v, v);
    else if(currentMatrix == VIEW_MATRIX)
        viewMatrix[viewMatrix.size() - 1].scale(v, v, v);

    matricesReady = false;
}

void Pipeline::rotateX(const float angle)
{
    if(currentMatrix == MODEL_MATRIX)
        modelMatrix[modelMatrix.size() - 1].rotate(angle, 1.0f, 0.0f, 0.0f);
    else if(currentMatrix == VIEW_MATRIX)
        viewMatrix[viewMatrix.size() - 1].rotate(-angle, 1.0f, 0.0f, 0.0f);

    matricesReady = false;
}

void Pipeline::rotateY(const float angle)
{
    if(currentMatrix == MODEL_MATRIX)
        modelMatrix[modelMatrix.size() - 1].rotate(angle, 0.0f, 1.0f, 0.0f);
    else if(currentMatrix == VIEW_MATRIX)
        viewMatrix[viewMatrix.size() - 1].rotate(-angle, 0.0f, 1.0f, 0.0f);

    matricesReady = false;
}

void Pipeline::rotateZ(const float angle)
{
    if(currentMatrix == MODEL_MATRIX)
        modelMatrix[modelMatrix.size() - 1].rotate(angle, 0.0f, 0.0f, 1.0f);
    else if(currentMatrix == VIEW_MATRIX)
        viewMatrix[viewMatrix.size() - 1].rotate(-angle, 0.0f, 0.0f, 1.0f);

    matricesReady = false;
}

void Pipeline::rotate(const float angle, const float x, const float y, const float z)
{
    if(currentMatrix == MODEL_MATRIX)
        modelMatrix[modelMatrix.size() - 1].rotate(angle, x, y, z);
    else if(currentMatrix == VIEW_MATRIX)
        viewMatrix[viewMatrix.size() - 1].rotate(-angle, x, y, z);

    matricesReady = false;
}

void Pipeline::lookAt(const QVector3D& eye, const QVector3D& center, const QVector3D& up)
{
    viewMatrix[viewMatrix.size() - 1].lookAt(eye, center, up);

    matricesReady = false;

    // calculate light matrix
    lightMatrix = viewMatrix[viewMatrix.size() - 1] * lightModelMatrix;
}

void Pipeline::lookAt(const QVector3D& position, QVector3D& direction)
{
    float angleUD = direction.x();

    if(angleUD > 89.9f)
        direction.setX(89.9f);
    if(angleUD < -89.9f)
        direction.setX(-89.9f);

    angleUD = direction.x();

    float angleLR = direction.y() + 180;
    //float angleCT = direction.getZ();

    float vLRx = sin(MathHelper::degreesToRadians(angleLR)), vLRz = -cos(MathHelper::degreesToRadians(angleLR));
    float vUDy = sin(MathHelper::degreesToRadians(angleUD)), vUDz = -cos(MathHelper::degreesToRadians(angleUD));
    //float vCTx = cos(DEG2RAD(angleCT)), vCTy = sin(DEG2RAD(angleCT)), vCTz = 0;

    float cX = position.x() + (vLRx * vUDz);
    float cY = position.y() + vUDy;
    float cZ = position.z() + (vLRz * vUDz);

    lookAt(QVector3D(position.x(), position.y(), position.z()), QVector3D(cX, cY, cZ), QVector3D(0, 1, 0));

    skyboxMatrix[skyboxMatrix.size() - 1].setToIdentity();
    skyboxMatrix[skyboxMatrix.size() - 1].translate(position.x(), position.y(), position.z());
    skyboxMatrix[skyboxMatrix.size() - 1].rotate(direction.x(), 1.0f, 0.0f, 0.0f);
    skyboxMatrix[skyboxMatrix.size() - 1].rotate(direction.y(), 0.0f, 1.0f, 0.0f);
}

void Pipeline::ortho(const float left, const float right, const float bottom, const float top, const float nearPlane, const float farPlane)
{
    projectionMatrix[projectionMatrix.size() - 1].ortho(left, right, bottom, top, nearPlane, farPlane);

    matricesReady = false;
}

void Pipeline::perspective(const float angle, const float aRatio, const float nearPlane, const float farPlane)
{
    projectionMatrix[projectionMatrix.size() - 1].perspective(angle, aRatio, nearPlane, farPlane);

    matricesReady = false;
}

void Pipeline::resize(const int width, const int height)
{
    viewportSize = QVector2D(float(width), float(height));

    float w2 = width  / 2.0f;
    float h2 = height / 2.0f;

    viewportMatrix.setToIdentity();
    viewportMatrix.setColumn(0, QVector4D(w2, 0.0f, 0.0f, 0.0f));
    viewportMatrix.setColumn(1, QVector4D(0.0f, h2, 0.0f, 0.0f));
    viewportMatrix.setColumn(2, QVector4D(0.0f, 0.0f, 1.0f, 0.0f));
    viewportMatrix.setColumn(3, QVector4D(w2, h2, 0.0f, 1.0f));
}

void Pipeline::setLight(const QVector3D& position, QVector3D& direction)
{
    lightPosition = position;

    lookAt(position, direction);

    lightViewMatrix       = getViewMatrix();
    lightProjectionMatrix = getProjectionMatrix();

    lightModelMatrix.setToIdentity();
    lightModelMatrix.translate(position.x(), position.y(), position.z());
}

void Pipeline::setCamera(const QMatrix4x4& model, const QMatrix4x4& view, const QMatrix4x4& proj)
{
    modelMatrix.push_back(model);
    viewMatrix.push_back(view);
    projectionMatrix.push_back(proj);

    modelViewMatrix           = QMatrix4x4();
    modelViewProjectionMatrix = QMatrix4x4();
    normalMatrix              = QMatrix3x3();
    worldNormalMatrix         = QMatrix3x3();
    matricesReady             = true;
    currentMatrix             = 0;
}

const QMatrix4x4 Pipeline::getModelViewMatrix() const
{
    if(!matricesReady)
        return viewMatrix[viewMatrix.size() - 1] * modelMatrix[modelMatrix.size() - 1];
    else
        return modelViewMatrix;
}

const QMatrix4x4 Pipeline::getModelViewProjectionMatrix() const
{
    if(!matricesReady)
        return viewMatrix[viewMatrix.size() - 1] * modelMatrix[modelMatrix.size() - 1] * projectionMatrix[projectionMatrix.size() - 1];
    else
        return modelViewProjectionMatrix;
}

void Pipeline::pushMatrix()
{
    QMatrix4x4 matrix;

    if(currentMatrix == MODEL_MATRIX)
    {
        matrix = getModelMatrix();

        modelMatrix.push_back(matrix);
    }
    else if(currentMatrix == VIEW_MATRIX)
    {
        matrix = getViewMatrix();

        viewMatrix.push_back(matrix);
    }
    else
    {
        matrix = getProjectionMatrix();

        projectionMatrix.push_back(matrix);
    }
}

void Pipeline::popMatrix()
{
    if(currentMatrix == MODEL_MATRIX)
    {
        if(modelMatrix.size() > 1)
            modelMatrix.pop_back();
    }
    else if(currentMatrix == VIEW_MATRIX)
    {
        if(viewMatrix.size() > 1)
            viewMatrix.pop_back();
    }
    else
    {
        if(projectionMatrix.size() > 1)
            projectionMatrix.pop_back();
    }
}

void Pipeline::updateMatrices(QOpenGLShaderProgram* shader)
{
    if(!matricesReady)
    {
        modelViewMatrix           = viewMatrix[viewMatrix.size() - 1] * modelMatrix[modelMatrix.size() - 1];
        modelViewProjectionMatrix = projectionMatrix[projectionMatrix.size() - 1] * viewMatrix[viewMatrix.size() - 1] * modelMatrix[modelMatrix.size() - 1];
        normalMatrix              = modelViewMatrix.normalMatrix();
        worldNormalMatrix         = modelMatrix[modelMatrix.size() - 1].normalMatrix();
    }

    shader->setUniformValue("modelMatrix",               modelMatrix[modelMatrix.size() - 1]);
    shader->setUniformValue("viewMatrix",                viewMatrix[viewMatrix.size() - 1]);
    shader->setUniformValue("projectionMatrix",          projectionMatrix[projectionMatrix.size() - 1]);
    shader->setUniformValue("modelViewMatrix",           modelViewMatrix);
    shader->setUniformValue("modelViewProjectionMatrix", modelViewProjectionMatrix);
    shader->setUniformValue("normalMatrix",              normalMatrix);
    shader->setUniformValue("viewportSize",              viewportSize); // We need the viewport size to calculate tessellation levels and the geometry shader also needs the viewport matrix
    shader->setUniformValue("viewportMatrix",            viewportMatrix);
    shader->setUniformValue("worldNormalMatrix",         worldNormalMatrix);

    shader->setUniformValue("shadowEnabled", isShadowEnabled());

    if(isShadowEnabled())
    {
        shader->setUniformValue("lightModelViewProjectionMatrix", biasMatrix * lightProjectionMatrix * lightViewMatrix * modelMatrix[modelMatrix.size() - 1]);
        shader->setUniformValue("lightPosition",                  lightPosition);
        shader->setUniformValue("lightMatrix",                    lightMatrix);
    }
}
