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

#ifndef CAMERA_H
#define CAMERA_H

#include <QObject>

#include <QMatrix4x4>
#include <QQuaternion>
#include <QVector3D>

class CameraPrivate;
class BezierCurve;

class Camera : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QVector3D position READ position WRITE setPosition)
    Q_PROPERTY(QVector3D upVector READ upVector WRITE setUpVector)
    Q_PROPERTY(QVector3D viewCenter READ viewCenter WRITE setViewCenter)

    Q_PROPERTY(ProjectionType projectionType READ projectionType)
    Q_PROPERTY(float nearPlane READ nearPlane WRITE setNearPlane)
    Q_PROPERTY(float farPlane READ farPlane WRITE setFarPlane)

    Q_PROPERTY(float fieldOfView READ fieldOfView WRITE setFieldOfView)
    Q_PROPERTY(float aspectRatio READ aspectRatio WRITE setAspectRatio)

    Q_PROPERTY(float left READ left WRITE setLeft)
    Q_PROPERTY(float right READ right WRITE setRight)
    Q_PROPERTY(float bottom READ bottom WRITE setBottom)
    Q_PROPERTY(float top READ top WRITE setTop)

    Q_PROPERTY(bool locked READ lock WRITE setLock)

    Q_PROPERTY(QVector<BezierCurve> curves WRITE setCurves)

    Q_ENUMS(ProjectionType)

public:
    explicit Camera(QObject* parent = 0);

    enum ProjectionType
    {
        OrthogonalProjection,
        PerspectiveProjection
    };

    enum CameraTranslationOption
    {
        TranslateViewCenter,
        DontTranslateViewCenter
    };

    void initialize();

    void drawCurve(QMatrix4x4 modelMatrix);

    QVector3D position() const; // Return camera eye
    QVector3D upVector() const;
    QVector3D viewCenter() const;
    QVector3D viewVector() const;

    const QVector3D& pos() const; // Return camera eye

    ProjectionType projectionType() const;

    void setOrthographicProjection(float left, float right,
                                   float bottom, float top,
                                   float nearPlane, float farPlane);

    void setPerspectiveProjection(float fieldOfView, float aspect,
                                  float nearPlane, float farPlane);

    void setNearPlane(const float& nearPlane);
    float nearPlane() const;

    void setFarPlane(const float& nearPlane);
    float farPlane() const;

    void setFieldOfView(const float& fieldOfView);
    float fieldOfView() const;

    void setAspectRatio(const float& aspectRatio);
    float aspectRatio() const;

    void setLeft(const float& left);
    float left() const;

    void setRight(const float& right);
    float right() const;

    void setBottom(const float& bottom);
    float bottom() const;

    void setTop(const float& top);
    float top() const;

    void setLock(const bool& lock);
    bool lock() const;

    void setCurves(const QVector<BezierCurve>& BCurves);

    void play();
    void stop();

    void resetRotation();

    void moveToPosition(const QVector3D& position);

    QMatrix4x4 viewMatrix() const;
    QMatrix4x4 projectionMatrix() const;
    QMatrix4x4 viewProjectionMatrix() const;

    QQuaternion tiltRotation(const float& angle) const;
    QQuaternion panRotation(const float& angle) const;
    QQuaternion panRotation(const float& angle, const QVector3D& axis) const;
    QQuaternion rollRotation(const float& angle) const;

public slots:
    void setPosition(const QVector3D& position);
    void setUpVector(const QVector3D& upVector);
    void setViewCenter(const QVector3D& viewCenter);

    // Translate relative to camera orientation axes
    void translate(const QVector3D& vLocal, CameraTranslationOption option = TranslateViewCenter);

    // Translate relative to world axes
    void translateWorld(const QVector3D& vWorld, CameraTranslationOption option = TranslateViewCenter);

    void tilt(const float& angle);
    void pan(const float& angle);
    void pan(const float& angle, const QVector3D& axis);
    void roll(const float& angle);

    void tiltAboutViewCenter(const float& angle);
    void panAboutViewCenter(const float& angle);
    void rollAboutViewCenter(const float& angle);

    void rotate(const QQuaternion& q);
    void rotateAboutViewCenter(const QQuaternion& q);

protected:
    Q_DECLARE_PRIVATE(Camera)

private:
    CameraPrivate* d_ptr;
};

#endif // CAMERA_H