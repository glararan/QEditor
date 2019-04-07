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

#include "camera.h"
#include "camera_p.h"

#include "beziercurve.h"

#include <QtAlgorithms>

Camera::Camera(QObject* parent) : QObject(parent), d_ptr(new CameraPrivate(this))
{
}

void Camera::initialize()
{
    Q_D(Camera);

    d->initialize();
}

void Camera::drawCurve(QMatrix4x4 modelMatrix)
{
    Q_D(Camera);

    QMatrix4x4 modelViewProject = projectionMatrix() * viewMatrix() * modelMatrix;

    d->curveShader->bind();
    d->curveShader->setUniformValue("mvp", modelViewProject);

    foreach(BezierCurve* curve, d->curves)
        curve->drawCurve(d->curveShader);

    d->curvePointsShader->bind();
    d->curvePointsShader->setUniformValue("mvp", modelViewProject);

    foreach(BezierCurve* curve, d->curves)
        curve->drawControlPoints(d->curvePointsShader);
}

Camera::ProjectionType Camera::projectionType() const
{
    Q_D(const Camera);

    return d->m_projectionType;
}

QVector3D Camera::position() const
{
    Q_D(const Camera);

    return d->m_position;
}

const QVector3D& Camera::pos() const
{
    Q_D(const Camera);

    return d->m_position;
}

void Camera::setPosition(const QVector3D& position)
{
    Q_D(Camera);

    d->m_position        = position;
    d->m_cameraToCenter  = d->m_viewCenter - position;
    d->m_viewMatrixDirty = true;
}

void Camera::setUpVector(const QVector3D& upVector)
{
    Q_D(Camera);

    d->m_upVector        = upVector;
    d->m_viewMatrixDirty = true;
}

QVector3D Camera::upVector() const
{
    Q_D(const Camera);

    return d->m_upVector;
}

void Camera::setViewCenter(const QVector3D& viewCenter)
{
    Q_D(Camera);

    d->m_viewCenter      = viewCenter;
    d->m_cameraToCenter  = viewCenter - d->m_position;
    d->m_viewMatrixDirty = true;
}

QVector3D Camera::viewCenter() const
{
    Q_D(const Camera);

    return d->m_viewCenter;
}

QVector3D Camera::viewVector() const
{
    Q_D(const Camera);

    return d->m_cameraToCenter;
}

void Camera::setOrthographicProjection(float left, float right, float bottom, float top, float nearPlane, float farPlane)
{
    Q_D(Camera);

    d->m_left           = left;
    d->m_right          = right;
    d->m_bottom         = bottom;
    d->m_top            = top;
    d->m_nearPlane      = nearPlane;
    d->m_farPlane       = farPlane;
    d->m_projectionType = OrthogonalProjection;

    d->updateOrthogonalProjection();
}

void Camera::setPerspectiveProjection(float fieldOfView, float aspectRatio, float nearPlane, float farPlane)
{
    Q_D(Camera);

    d->m_fieldOfView    = fieldOfView;
    d->m_aspectRatio    = aspectRatio;
    d->m_nearPlane      = nearPlane;
    d->m_farPlane       = farPlane;
    d->m_projectionType = PerspectiveProjection;

    d->updatePerpectiveProjection();
}

void Camera::setNearPlane(const float& nearPlane)
{
    Q_D(Camera);

    if(qFuzzyCompare(d->m_nearPlane, nearPlane))
        return;

    d->m_nearPlane = nearPlane;

    if(d->m_projectionType == PerspectiveProjection)
        d->updatePerpectiveProjection();
}

float Camera::nearPlane() const
{
    Q_D(const Camera);

    return d->m_nearPlane;
}

void Camera::setFarPlane(const float& farPlane)
{
    Q_D(Camera);

    if(qFuzzyCompare(d->m_farPlane, farPlane))
        return;

    d->m_farPlane = farPlane;

    if(d->m_projectionType == PerspectiveProjection)
        d->updatePerpectiveProjection();
}

float Camera::farPlane() const
{
    Q_D(const Camera);
    return d->m_farPlane;
}

void Camera::setFieldOfView(const float& fieldOfView)
{
    Q_D(Camera);

    if(qFuzzyCompare(d->m_fieldOfView, fieldOfView))
        return;

    d->m_fieldOfView = fieldOfView;

    if(d->m_projectionType == PerspectiveProjection)
        d->updatePerpectiveProjection();
}

float Camera::fieldOfView() const
{
    Q_D(const Camera);

    return d->m_fieldOfView;
}

void Camera::setAspectRatio(const float& aspectRatio)
{
    Q_D(Camera);

    if(qFuzzyCompare(d->m_aspectRatio, aspectRatio))
        return;

    d->m_aspectRatio = aspectRatio;

    if(d->m_projectionType == PerspectiveProjection)
        d->updatePerpectiveProjection();
}

float Camera::aspectRatio() const
{
    Q_D(const Camera);

    return d->m_aspectRatio;
}

void Camera::setLeft(const float& left)
{
    Q_D(Camera);

    if(qFuzzyCompare(d->m_left, left))
        return;

    d->m_left = left;

    if(d->m_projectionType == OrthogonalProjection)
        d->updateOrthogonalProjection();
}

float Camera::left() const
{
    Q_D(const Camera);

    return d->m_left;
}

void Camera::setRight(const float& right)
{
    Q_D(Camera);

    if (qFuzzyCompare(d->m_right, right))
        return;

    d->m_right = right;

    if(d->m_projectionType == OrthogonalProjection)
        d->updateOrthogonalProjection();
}

float Camera::right() const
{
    Q_D(const Camera);

    return d->m_right;
}

void Camera::setBottom(const float& bottom)
{
    Q_D(Camera);

    if(qFuzzyCompare(d->m_bottom, bottom))
        return;

    d->m_bottom = bottom;

    if(d->m_projectionType == OrthogonalProjection)
        d->updateOrthogonalProjection();
}

float Camera::bottom() const
{
    Q_D(const Camera);

    return d->m_bottom;
}

void Camera::setTop(const float& top)
{
    Q_D(Camera);

    if(qFuzzyCompare(d->m_top, top))
        return;

    d->m_top = top;

    if(d->m_projectionType == OrthogonalProjection)
        d->updateOrthogonalProjection();
}

float Camera::top() const
{
    Q_D(const Camera);

    return d->m_top;
}

void Camera::setLock(const bool& lock)
{
    Q_D(Camera);

    d->locked = lock;
}

bool Camera::lock() const
{
    Q_D(const Camera);

    return d->locked;
}

void Camera::setPlayRepeat(const bool& repeat)
{
    Q_D(Camera);

    d->repeatPlay = repeat;
}

bool Camera::playRepeat() const
{
    Q_D(const Camera);

    return d->repeatPlay;
}

void Camera::setCurves(const QVector<BezierCurve*>& BCurves)
{
    Q_D(Camera);

    d->curves = BCurves;
}

void Camera::deleteCurves()
{
    Q_D(Camera);

    qDeleteAll(d->curves); // delete BezierCurves pointers, thanks to QtAlgorithms

    d->curves.clear();
}

void Camera::play(const int& secs)
{
    Q_D(Camera);

    d->play_ticks = MathHelper::toInt(floor(MathHelper::toFloat(secs) * 1000.0f / 16.6f));
    d->play_tick  = 0;
    d->play       = true;
    d->locked     = true;
}

void Camera::playSequence()
{
    Q_D(Camera);

    int curveIndex = MathHelper::toInt(floor(MathHelper::toFloat(d->play_tick) / MathHelper::toFloat(d->play_ticks / d->curves.count())));
    int playTicks  = d->play_ticks / d->curves.count();
    int playTick   = d->play_tick - (playTicks * curveIndex);

    if(curveIndex >= d->curves.count())
    {
        stop();

        return;
    }

    BezierCurve* curve = d->curves.at(curveIndex);

    ++d->play_tick;

    float t = 1.0f / playTicks * playTick;

    setPosition(curve->calculatePoint(t));
    setViewCenter(curve->calculateViewCenter(t));
    setUpVector(curve->calculateUpVector(t));

    if(d->play_tick == d->play_ticks)
        stop(false);
}

void Camera::stop(bool override)
{
    Q_D(Camera);

    if(!d->repeatPlay || override)
    {
        d->play   = false;
        d->locked = false;
    }
    else if(d->repeatPlay && !override)
        d->play_tick = 0;
}

bool Camera::playing() const
{
    Q_D(const Camera);

    return d->play;
}

QMatrix4x4 Camera::viewMatrix() const
{
    Q_D(const Camera);

    if(d->m_viewMatrixDirty)
    {
        d->m_viewMatrix.setToIdentity();
        d->m_viewMatrix.lookAt(d->m_position, d->m_viewCenter, d->m_upVector);

        d->m_viewMatrixDirty = false;
    }

    return d->m_viewMatrix;
}

QMatrix4x4 Camera::projectionMatrix() const
{
    Q_D(const Camera);

    return d->m_projectionMatrix;
}

QMatrix4x4 Camera::viewProjectionMatrix() const
{
    Q_D(const Camera);

    if(d->m_viewMatrixDirty || d->m_viewProjectionMatrixDirty)
    {
        d->m_viewProjectionMatrix      = d->m_projectionMatrix * viewMatrix();
        d->m_viewProjectionMatrixDirty = false;
    }

    return d->m_viewProjectionMatrix;
}

void Camera::translate(const QVector3D& vLocal, CameraTranslationOption option)
{
    Q_D(Camera);

    // Calculate the amount to move by in world coordinates
    QVector3D vWorld;

    if(!qFuzzyIsNull(vLocal.x()))
    {
        // Calculate the vector for the local x axis
        QVector3D x = QVector3D::crossProduct(d->m_cameraToCenter, d->m_upVector).normalized();
        vWorld += vLocal.x() * x;
    }

    if(!qFuzzyIsNull(vLocal.y()))
        vWorld += vLocal.y() * d->m_upVector;

    if(!qFuzzyIsNull(vLocal.z()))
        vWorld += vLocal.z() * d->m_cameraToCenter.normalized();

    // Update the camera position using the calculated world vector
    d->m_position += vWorld;

    // May be also update the view center coordinates
    if(option == TranslateViewCenter)
        d->m_viewCenter += vWorld;

    // Refresh the camera -> view center vector
    d->m_cameraToCenter = d->m_viewCenter - d->m_position;

    // Calculate a new up vector. We do this by:
    // 1) Calculate a new local x-direction vector from the cross product of the new
    //    camera to view center vector and the old up vector.
    // 2) The local x vector is the normal to the plane in which the new up vector
    //    must lay. So we can take the cross product of this normal and the new
    //    x vector. The new normal vector forms the last part of the orthonormal basis
    QVector3D x   = QVector3D::crossProduct(d->m_cameraToCenter, d->m_upVector).normalized();
    d->m_upVector = QVector3D::crossProduct(x, d->m_cameraToCenter).normalized();

    d->m_viewMatrixDirty = true;
}

void Camera::translateWorld(const QVector3D& vWorld, CameraTranslationOption option)
{
    Q_D(Camera);

    // Update the camera position using the calculated world vector
    d->m_position += vWorld;

    // May be also update the view center coordinates
    if(option == TranslateViewCenter)
        d->m_viewCenter += vWorld;

    // Refresh the camera -> view center vector
    d->m_cameraToCenter = d->m_viewCenter - d->m_position;

    d->m_viewMatrixDirty = true;
}

QQuaternion Camera::tiltRotation(const float& angle) const
{
    Q_D(const Camera);

    QVector3D xBasis = QVector3D::crossProduct(d->m_upVector, d->m_cameraToCenter.normalized()).normalized();

    return QQuaternion::fromAxisAndAngle(xBasis, -angle);
}

QQuaternion Camera::panRotation(const float& angle) const
{
    Q_D(const Camera);

    return QQuaternion::fromAxisAndAngle(d->m_upVector, angle);
}

QQuaternion Camera::panRotation(const float& angle, const QVector3D& axis) const
{
    return QQuaternion::fromAxisAndAngle(axis, angle);
}

QQuaternion Camera::rollRotation(const float& angle) const
{
    Q_D(const Camera);

    return QQuaternion::fromAxisAndAngle(d->m_cameraToCenter, -angle);
}

void Camera::tilt(const float& angle)
{
    Q_D(Camera);

    if(d->locked)
        return;

    QQuaternion q = tiltRotation(angle);
    rotate(q);
}

void Camera::pan(const float& angle)
{
    Q_D(Camera);

    if(d->locked)
        return;

    QQuaternion q = panRotation(-angle);
    rotate(q);
}

void Camera::pan(const float& angle, const QVector3D& axis)
{
    Q_D(Camera);

    if(d->locked)
        return;

    QQuaternion q = panRotation(-angle, axis);
    rotate(q);
}

void Camera::roll(const float& angle)
{
    QQuaternion q = rollRotation(-angle);
    rotate(q);
}

void Camera::tiltAboutViewCenter(const float& angle)
{
    QQuaternion q = tiltRotation(-angle);
    rotateAboutViewCenter(q);
}

void Camera::panAboutViewCenter(const float& angle)
{
    QQuaternion q = panRotation(angle);
    rotateAboutViewCenter(q);
}

void Camera::rollAboutViewCenter(const float& angle)
{
    QQuaternion q = rollRotation(angle);
    rotateAboutViewCenter(q);
}

void Camera::rotate(const QQuaternion& q)
{
    Q_D(Camera);

    d->m_upVector       = q.rotatedVector(d->m_upVector);
    d->m_cameraToCenter = q.rotatedVector(d->m_cameraToCenter);
    d->m_viewCenter     = d->m_position + d->m_cameraToCenter;
}

void Camera::rotateAboutViewCenter(const QQuaternion& q)
{
    Q_D(Camera);

    d->m_upVector       = q.rotatedVector(d->m_upVector);
    d->m_cameraToCenter = q.rotatedVector(d->m_cameraToCenter);
    d->m_position       = d->m_viewCenter - d->m_cameraToCenter;
}

void Camera::resetRotation()
{
    Q_D(Camera);

    QVector3D Cpos = d->m_position;

    setUpVector(QVector3D(0.0f, 1.0f, 0.0f));
    setPosition(Cpos);

    Cpos.setZ(Cpos.z() - 1.0f);

    setViewCenter(Cpos);
}

void Camera::invertY(const float& terrainHeight)
{
    Q_D(Camera);

    float heightDiff = terrainHeight - d->m_position.y() * 2;

    setPosition(QVector3D(d->m_position.x(), d->m_position.y() + heightDiff, d->m_position.z()));
    setViewCenter(QVector3D(d->m_viewCenter.x(), d->m_viewCenter.y() + heightDiff, d->m_viewCenter.z()));

    QVector2D direction = MathHelper::getDirections(d->m_viewMatrix);

    tilt(-(direction.y() * 2));
}

void Camera::moveToPosition(const QVector3D& position)
{
    setPosition(position);
    setViewCenter(QVector3D(position.x(), position.y(), position.z() - 1.0f));
    setUpVector(QVector3D(0.0f, 1.0f, 0.0f));
}
