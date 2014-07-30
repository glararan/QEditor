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

#ifndef CAMERA_P_H
#define CAMERA_P_H

#include <QMatrix4x4>
#include <QVector3D>
#include <QOpenGLShaderProgram>

class BezierCurve;

class CameraPrivate
{
public:
    CameraPrivate(Camera* qq)
    : q_ptr(qq)
    , m_position(0.0f, 0.0f, 1.0f)
    , m_upVector(0.0f, 1.0f, 0.0f)
    , m_viewCenter(0.0f, 0.0f, 0.0f)
    , m_cameraToCenter(0.0f, 0.0f, -1.0f)
    , m_projectionType(Camera::OrthogonalProjection)
    , m_nearPlane(0.1f)
    , m_farPlane(1024.0f)
    , m_fieldOfView(60.0f)
    , m_aspectRatio(1.0f)
    , m_left(-0.5)
    , m_right(0.5f)
    , m_bottom(-0.5f)
    , m_top(0.5f)
    , locked(false)
    , m_viewMatrixDirty(true)
    , m_viewProjectionMatrixDirty(true)
    , curveShader(NULL)
    , curvePointsShader(NULL)
    {
        updateOrthogonalProjection();
    }

    ~CameraPrivate()
    {
        delete curveShader;
    }

    inline void updatePerpectiveProjection()
    {
        m_projectionMatrix.setToIdentity();
        m_projectionMatrix.perspective(m_fieldOfView, m_aspectRatio, m_nearPlane, m_farPlane);

        m_viewProjectionMatrixDirty = true;
    }

    inline void updateOrthogonalProjection()
    {
        m_projectionMatrix.setToIdentity();
        m_projectionMatrix.ortho(m_left, m_right, m_bottom, m_top, m_nearPlane, m_farPlane);

        m_viewProjectionMatrixDirty = true;
    }

    inline void initialize()
    {
        curveShader = new QOpenGLShaderProgram();

        if(!curveShader->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/data/shaders/qeditor_bezier.vert"))
            qCritical() << QObject::tr("Could not compile vertex shader. Log:") << curveShader->log();

        if(!curveShader->addShaderFromSourceFile(QOpenGLShader::Geometry, ":/data/shaders/qeditor_bezier.geom"))
            qCritical() << QObject::tr("Could not compile geometry shader. Log:") << curveShader->log();

        if(!curveShader->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/data/shaders/qeditor_bezier.frag"))
            qCritical() << QObject::tr("Could not compile fragment shader. Log:") << curveShader->log();

        if(!curveShader->link())
            qCritical() << QObject::tr("Could not link shader program. Log:") << curveShader->log();

        curvePointsShader = new QOpenGLShaderProgram();

        if(!curvePointsShader->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/data/shaders/qeditor_bezier_point.vert"))
            qCritical() << QObject::tr("Could not compile vertex shader. Log:") << curvePointsShader->log();

        if(!curvePointsShader->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/data/shaders/qeditor_bezier_point.frag"))
            qCritical() << QObject::tr("Could not compile fragment shader. Log:") << curvePointsShader->log();

        if(!curvePointsShader->link())
            qCritical() << QObject::tr("Could not link shader program. Log:") << curvePointsShader->log();
    }

    Q_DECLARE_PUBLIC(Camera)

    Camera* q_ptr;

    QOpenGLShaderProgram* curveShader;
    QOpenGLShaderProgram* curvePointsShader;

    QVector<BezierCurve> curves;

    QVector3D m_position;
    QVector3D m_upVector;
    QVector3D m_viewCenter;

    QVector3D m_cameraToCenter; // The vector from the camera position to the view center

    Camera::ProjectionType m_projectionType;

    float m_nearPlane;
    float m_farPlane;

    float m_fieldOfView;
    float m_aspectRatio;

    float m_left;
    float m_right;
    float m_bottom;
    float m_top;

    bool locked;

    mutable QMatrix4x4 m_viewMatrix;
    mutable QMatrix4x4 m_projectionMatrix;
    mutable QMatrix4x4 m_viewProjectionMatrix;

    mutable bool m_viewMatrixDirty;
    mutable bool m_viewProjectionMatrixDirty;
};

#endif // CAMERA_P_H