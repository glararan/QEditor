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

#ifndef MAPEXPLORER_H
#define MAPEXPLORER_H

#include <QOpenGLWidget>
#include <QWidget>
#include <QOpenGLFunctions_4_2_Core>

#include <QVector3D>
#include <QVector2D>
#include <QMatrix4x4>
#include <QTime>

class World;
class MapView;
class Camera;

class MapExplorer : public QOpenGLWidget, protected QOpenGLFunctions_4_2_Core
{
    Q_OBJECT

public:
    MapExplorer(World* mWorld, MapView* mapView, QWidget* parent = 0);
    ~MapExplorer();

    // Camera motion control
    void setSideSpeed(float vx)     { m_v.setX(vx); }
    void setVerticalSpeed(float vy) { m_v.setY(vy); }
    void setForwardSpeed(float vz)  { m_v.setZ(vz); }
    void setFieldOfView(float zoom) { camera_zoom += zoom; }

    // Camera orientation control
    void pan(float angle)  { m_panAngle = angle; }
    void tilt(float angle) { m_tiltAngle = angle; }

    // Screen space error - tessellation control
    void setScreenSpaceError(float error) { screenSpaceErrorLevel = error; }
    float screenSpaceError() const        { return screenSpaceErrorLevel; }

protected:
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();
    void update(float t);

    void keyPressEvent(QKeyEvent* e);
    void keyReleaseEvent(QKeyEvent* e);

    void mousePressEvent(QMouseEvent* e);
    void mouseReleaseEvent(QMouseEvent* e);
    void mouseMoveEvent(QMouseEvent* e);

    void wheelEvent(QWheelEvent* e);
    void timerEvent(QTimerEvent*);

    void focusInEvent(QFocusEvent* e);
    void focusOutEvent(QFocusEvent* e);

private:
    World* world;
    MapView* _mapView;

    Camera* camera;

    QVector3D m_v;

    float m_panAngle;
    float m_tiltAngle;
    float panAngle;
    float tiltAngle;
    float camera_zoom;
    float aspectRatio;
    float nearPlane;
    float farPlane;

    const float speed;
    float       speed_mult;

    QMatrix4x4 viewportMatrix;
    QVector2D  viewportSize;

    float screenSpaceErrorLevel;
    float time;

    const float m_metersToUnits;

    QTime m_Utime, fpsTimer;

    int fps, frames;

    bool leftButtonPressed, rightButtonPressed, wheelButtonPressed, wasLeftButtonPressed, wasLeftButtonReleased, wasWheelButtonPressed, wasWheelButtonReleased;

    QPoint mouse_position;

    QPoint prevMousePos;
    QPoint mousePos;
    QPoint mousePosStart, mousePosEnd;

    QVector2D mouseVector;

    int mousePosZ, prevMousePosZ;
};

/*class MapExplorerWidget : public QWidget
{
public:
    MapExplorerWidget(Q)

private:
};*/

#endif // MAPEXPLORER_H
