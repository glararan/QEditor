#include "ipipeline.h"
#include "mathhelper.h"

IPipeline::IPipeline()
{
    modelMatrix.push_back(QMatrix4x4());
    viewMatrix.push_back(QMatrix4x4());
    projectionMatrix.push_back(QMatrix4x4());

    modelViewMatrix=QMatrix4x4();
    modelViewProjectionMatrix=QMatrix4x4();
    normalMatrix=QMatrix3x3();
    worldNormalMatrix=QMatrix3x3();
    matricesReady=true;
    currentMatrix=0;
}

IPipeline::IPipeline(QMatrix4x4 model, QMatrix4x4 view, QMatrix4x4 projection)
{
    modelMatrix.push_back(model);
    viewMatrix.push_back(view);
    projectionMatrix.push_back(projection);

    modelViewMatrix=QMatrix4x4();
    modelViewProjectionMatrix=QMatrix4x4();
    normalMatrix=QMatrix3x3();
    worldNormalMatrix=QMatrix3x3();
    matricesReady=true;
    currentMatrix=0;
}

void IPipeline::loadIdentity()
{
    if(currentMatrix==MODEL_MATRIX || currentMatrix==VIEW_MATRIX)
    {
        modelMatrix[modelMatrix.size()-1]=QMatrix4x4();
        viewMatrix[viewMatrix.size()-1]=QMatrix4x4();
    }else
        projectionMatrix[projectionMatrix.size()-1]=QMatrix4x4();
    matricesReady=false;
}

void IPipeline::matrixMode(int m)
{
    if(m==MODEL_MATRIX || m==VIEW_MATRIX || m==PROJECTION_MATRIX)
        currentMatrix=m;
}

void IPipeline::translate(float x, float y, float z)
{
    if(currentMatrix==MODEL_MATRIX)
        modelMatrix[modelMatrix.size()-1].translate(x,y,z);
    else if(currentMatrix==VIEW_MATRIX)
        viewMatrix[viewMatrix.size()-1].translate(-x,-y,-z);
    matricesReady=false;
}

void IPipeline::scale(float x, float y, float z)
{
    if(currentMatrix==MODEL_MATRIX)
        modelMatrix[modelMatrix.size()-1].scale(x,y,z);
    else if(currentMatrix==VIEW_MATRIX)
        viewMatrix[viewMatrix.size()-1].scale(x,y,z);
    matricesReady=false;
}

void IPipeline::scale(float v)
{
    if(currentMatrix==MODEL_MATRIX)
        modelMatrix[modelMatrix.size()-1].scale(v,v,v);
    else if(currentMatrix==VIEW_MATRIX)
        viewMatrix[viewMatrix.size()-1].scale(v,v,v);
    matricesReady=false;
}

void IPipeline::rotateX(float angle)
{
    if(currentMatrix==MODEL_MATRIX)
        modelMatrix[modelMatrix.size()-1].rotate(angle,1.0f,0.0f,0.0f);
    else if(currentMatrix==VIEW_MATRIX)
        viewMatrix[viewMatrix.size()-1].rotate(-angle,1.0f,0.0f,0.0f);
    matricesReady=false;
}

void IPipeline::rotateY(float angle)
{
    if(currentMatrix==MODEL_MATRIX)
        modelMatrix[modelMatrix.size()-1].rotate(angle,0.0f,1.0f,0.0f);
    else if(currentMatrix==VIEW_MATRIX)
        viewMatrix[viewMatrix.size()-1].rotate(-angle,0.0f,1.0f,0.0f);
    matricesReady=false;
}

void IPipeline::rotateZ(float angle)
{
    if(currentMatrix==MODEL_MATRIX)
        modelMatrix[modelMatrix.size()-1].rotate(angle,0.0f,0.0f,1.0f);
    else if(currentMatrix==VIEW_MATRIX)
        viewMatrix[viewMatrix.size()-1].rotate(-angle,0.0f,0.0f,1.0f);
    matricesReady=false;
}

void IPipeline::rotate(float angle, float x, float y, float z)
{
    if(currentMatrix==MODEL_MATRIX)
        modelMatrix[modelMatrix.size()-1].rotate(angle,x,y,z);
    else if(currentMatrix==VIEW_MATRIX)
        viewMatrix[viewMatrix.size()-1].rotate(-angle,x,y,z);
    matricesReady=false;
}

void IPipeline::lookAt(const QVector3D &eye, const QVector3D &center, const QVector3D &up)
{
    viewMatrix[viewMatrix.size()-1].lookAt(eye,center,up);
    matricesReady=false;
}

void IPipeline::lookAt(QVector3D &position, QVector3D &direction)
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

    this->lookAt(QVector3D(position.x(),position.y(),position.z()),QVector3D(cX,cY,cZ),QVector3D(0,1,0));
}

void IPipeline::ortho(float left, float right, float bottom, float top, float nearPlane, float farPlane)
{
    projectionMatrix[projectionMatrix.size()-1].ortho(left,right,bottom,top,nearPlane,farPlane);
    matricesReady=false;
}

void IPipeline::perspective(float angle, float aRatio, float nearPlane, float farPlane)
{
    projectionMatrix[projectionMatrix.size()-1].perspective(angle,aRatio,nearPlane,farPlane);
    matricesReady=false;
}

void IPipeline::resize(int width, int height)
{
    viewportSize = QVector2D(float(width), float(height));

    float w2 = width / 2.0f;
    float h2 = height / 2.0f;

    viewportMatrix.setToIdentity();
    viewportMatrix.setColumn(0, QVector4D(w2, 0.0f, 0.0f, 0.0f));
    viewportMatrix.setColumn(1, QVector4D(0.0f, h2, 0.0f, 0.0f));
    viewportMatrix.setColumn(2, QVector4D(0.0f, 0.0f, 1.0f, 0.0f));
    viewportMatrix.setColumn(3, QVector4D(w2, h2, 0.0f, 1.0f));
}

QMatrix4x4 IPipeline::getModelMatrix()
{
    return modelMatrix[modelMatrix.size()-1];
}

QMatrix4x4 IPipeline::getViewMatrix()
{
    return viewMatrix[viewMatrix.size()-1];
}

QMatrix4x4 IPipeline::getModelViewMatrix()
{
    if(!matricesReady)
        return viewMatrix[viewMatrix.size()-1]*modelMatrix[modelMatrix.size()-1];
    else
        return modelViewMatrix;
}

QMatrix4x4 IPipeline::getProjectionMatrix()
{
    return projectionMatrix[projectionMatrix.size()-1];
}

QMatrix4x4 IPipeline::getModelViewProjectionMatrix()
{
    if(!matricesReady)
        return viewMatrix[viewMatrix.size()-1]*modelMatrix[modelMatrix.size()-1]*projectionMatrix[projectionMatrix.size()-1];
    else
        return modelViewProjectionMatrix;
}

QMatrix4x4 IPipeline::getViewPortMatrix()
{
    return viewportMatrix;
}

QVector2D IPipeline::getViewportSize()
{
    return viewportSize;
}

void IPipeline::pushMatrix()
{
    QMatrix4x4 matrix;
    if(currentMatrix==MODEL_MATRIX)
    {
        matrix = getModelMatrix();
        modelMatrix.push_back(matrix);
    }
    else if(currentMatrix==VIEW_MATRIX)
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

void IPipeline::popMatrix()
{
    if(currentMatrix==MODEL_MATRIX)
    {
        if(modelMatrix.size() > 1)
            modelMatrix.pop_back();
    }
    else if(currentMatrix==VIEW_MATRIX)
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

void IPipeline::updateMatrices(QOpenGLShaderProgram *shader)
{
    if(!matricesReady)
    {
        modelViewMatrix=viewMatrix[viewMatrix.size()-1]*modelMatrix[modelMatrix.size()-1];
        modelViewProjectionMatrix=projectionMatrix[projectionMatrix.size()-1]*viewMatrix[viewMatrix.size()-1]*modelMatrix[modelMatrix.size()-1];
        normalMatrix=modelViewMatrix.normalMatrix();
        worldNormalMatrix=modelMatrix[modelMatrix.size()-1].normalMatrix();
    }
    shader->setUniformValue("modelMatrix",modelMatrix[modelMatrix.size()-1]);
    shader->setUniformValue("viewMatrix",viewMatrix[viewMatrix.size()-1]);
    shader->setUniformValue("projectionMatrix",projectionMatrix[projectionMatrix.size()-1]);
    shader->setUniformValue("modelViewMatrix",modelViewMatrix);
    shader->setUniformValue("modelViewProjectionMatrix",modelViewProjectionMatrix);
    shader->setUniformValue("normalMatrix",normalMatrix);
    shader->setUniformValue("viewportSize",viewportSize);
    shader->setUniformValue("viewportMatrix",viewportMatrix);
    shader->setUniformValue("worldNormalMatrix",worldNormalMatrix);
}
