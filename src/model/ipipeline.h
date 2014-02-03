#ifndef IPIPELINE_H
#define IPIPELINE_H

#include <QtCore>
#include <QtOpenGL>
#include <QMatrix4x4>

enum matrixModes{
    MODEL_MATRIX=0,
    VIEW_MATRIX,
    PROJECTION_MATRIX
};

class IPipeline
{    
public:
    IPipeline();
    IPipeline(QMatrix4x4 model, QMatrix4x4 view, QMatrix4x4 projection);
    void loadIdentity();
    void matrixMode(int m);
    void translate(float x,float y,float z);
    void scale(float x,float y,float z);
    void scale(float v);
    void rotateX(float angle);
    void rotateY(float angle);
    void rotateZ(float angle);
    void rotate(float angle, float x, float y, float z);
    void lookAt(const QVector3D & eye, const QVector3D & center, const QVector3D & up);
    void lookAt(QVector3D & position, QVector3D & direction);
    void ortho(float left,float right,float bottom,float top,float nearPlane,float farPlane);
    void perspective(float angle,float aRatio,float nearPlane,float farPlane);
    void resize(int width, int height);

    QMatrix4x4 getModelMatrix();
    QMatrix4x4 getViewMatrix();
    QMatrix4x4 getModelViewMatrix();
    QMatrix4x4 getProjectionMatrix();
    QMatrix4x4 getModelViewProjectionMatrix();
    QMatrix4x4 getViewPortMatrix();
    QVector2D getViewportSize();

    void pushMatrix();
    void popMatrix();

    void updateMatrices(QOpenGLShaderProgram *shader);


private:
    int currentMatrix;
    QVector<QMatrix4x4> modelMatrix;
    QVector<QMatrix4x4> viewMatrix;
    QVector<QMatrix4x4> projectionMatrix;
    QMatrix4x4 viewportMatrix;
    QVector2D viewportSize;

    bool matricesReady;
    QMatrix4x4 modelViewMatrix;
    QMatrix4x4 modelViewProjectionMatrix;
    QMatrix3x3 normalMatrix;
    QMatrix3x3 worldNormalMatrix;
};

#endif // IPIPELINE_H