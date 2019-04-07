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

#include "mapexplorer.h"

#include "world.h"
#include "mapview.h"
#include "camera.h"
#include "qeditor.h"

#include <QOpenGLFunctions_4_2_Core>
#include <QKeyEvent>
#include <QTabletEvent>
#include <QOpenGLContext>

MapExplorer::MapExplorer(World* mWorld, MapView* mapView, QWidget* parent)
: QOpenGLWidget(parent)
, world(mWorld)
, camera(new Camera(this))
, m_v()
, m_panAngle(0.0f)
, m_tiltAngle(0.0f)
, panAngle(0.0f)
, tiltAngle(0.0f)
, camera_zoom(25.0f)
, aspectRatio(MathHelper::toFloat(width()) / MathHelper::toFloat(height()))
, nearPlane(0.01f)
, farPlane(app().getSetting("environmentDistance", 256.0f).toFloat())
, speed(44.7f) // in m/s. Equivalent to 100 miles/hour)
, speed_mult(app().getSetting("speedMultiplier", 1.0f).toFloat())
, m_metersToUnits(4 * MathHelper::PI() / speed) // 500 units == 10 km => 0.05 units/m
, screenSpaceErrorLevel(12.0f)
, time(0.0f)
, fps(0)
, frames(0)
, leftButtonPressed(false)
, rightButtonPressed(false)
, wheelButtonPressed(false)
, wasLeftButtonPressed(false)
, wasLeftButtonReleased(false)
, wasWheelButtonPressed(false)
, wasWheelButtonReleased(false)
, mouse_position(QPoint(0, 0))
, prevMousePos(QPoint(0, 0))
, mousePos(QPoint(0, 0))
, mousePosStart(QPoint(0, 0))
, mousePosEnd(QPoint(0, 0))
, mouseVector(QVector2D())
, mousePosZ(0)
, prevMousePosZ(0)
{
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);

    _mapView = mapView;

    camera->setPosition(QVector3D(250.0f, 10.0f, 250.0f));
    camera->setViewCenter(QVector3D(250.0f, 10.0f, 249.0f));
    camera->setUpVector(QVector3D(0.0f, 1.0f, 0.0f));

    startTimer(16);
}

MapExplorer::~MapExplorer()
{
    delete camera;
}

void MapExplorer::timerEvent(QTimerEvent*)
{
    this->makeCurrent();

    const qreal time = m_Utime.elapsed() / 1000.0f;

    update(time);
}

void MapExplorer::initializeGL()
{
    this->makeCurrent();

    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    format.setMajorVersion(4);
    format.setMinorVersion(3);
    format.setSamples(4);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setOption(QSurfaceFormat::DebugContext);

    this->context()->setFormat(format);

    QOpenGLContext* _context = _mapView->context();
    _context->setShareContext(this->context());
    _context->create();

    this->context()->create();

    qDebug() << QOpenGLContext::areSharing(_mapView->context(), this->context());
    qDebug() << QOpenGLContext::areSharing(_context, this->context());

    this->makeCurrent();

    initializeOpenGLFunctions();

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    glDepthFunc(GL_LEQUAL); // just testing new depth func

    glClearColor(0.65f, 0.77f, 1.0f, 1.0f);

    fpsTimer.start();
}

void MapExplorer::update(float t)
{
    // Store the time
    const float dt = t - time;
    time           = t;

    // FPS
    ++frames;

    if(fpsTimer.elapsed() >= 1000)
    {
        fps = frames;

        frames = 0;

        fpsTimer.restart();
    }

    Camera::CameraTranslationOption option = Camera::TranslateViewCenter;

    camera->translate(m_v * dt * m_metersToUnits, option);

    if(!qFuzzyIsNull(m_panAngle))
    {
        camera->pan(m_panAngle, QVector3D(0.0f, 1.0f, 0.0f));
        panAngle  += m_panAngle;
        m_panAngle = 0.0f;

        if(panAngle > 360.0f || panAngle < -360.0f)
            panAngle = 0.0f;
    }

    if(!qFuzzyIsNull(m_tiltAngle))
    {
        if(tiltAngle + m_tiltAngle > 90.0f || tiltAngle + m_tiltAngle < -90.0f)
        {
            float copyTAngle = m_tiltAngle;

            if(tiltAngle + copyTAngle > 90.0f)
            {
                while(tiltAngle + copyTAngle - 0.1f > 90.0f)
                    copyTAngle -= 0.1f;
            }
            else if(tiltAngle + copyTAngle < -90.0f)
            {
                while(tiltAngle + copyTAngle + 0.1f < -90.0f)
                    copyTAngle += 0.1f;
            }

            m_tiltAngle = copyTAngle;
            copyTAngle  = 0.0f;
        }

        camera->tilt(m_tiltAngle);
        tiltAngle  += m_tiltAngle;
        m_tiltAngle = 0.0f;
    }

    // Update the camera perspective projection if camera zoom is changed
    if(camera_zoom != camera->fieldOfView())
        camera->setPerspectiveProjection(camera_zoom, aspectRatio, nearPlane, farPlane);
}

void MapExplorer::paintGL()
{
    this->makeCurrent();

    qDebug() << "MapExplorer" << QOpenGLContext::areSharing(this->context(), _mapView->context()) << (QOpenGLContext::currentContext() == this->context());

    QOpenGLShaderProgram* shader = world->getTerrainShader();
    qDebug() << shader->create();
    shader->bind();

    // We need the viewport size to calculate tessellation levels and the geometry shader also needs the viewport matrix
    shader->setUniformValue("viewportSize",   viewportSize);
    shader->setUniformValue("viewportMatrix", viewportMatrix);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    qDebug() << "MapExlorer paintGL ends";

    QMatrix4x4 modelMatrix;
    modelMatrix.setToIdentity();

    //world->drawExplorerView(camera, modelMatrix, screenSpaceErrorLevel);
}

void MapExplorer::resizeGL(int w, int h)
{
    this->makeCurrent();

    // Make sure the viewport covers the entire window
    glViewport(0, 0, w, h);

    viewportSize = QVector2D(MathHelper::toFloat(w), MathHelper::toFloat(h));

    // Update the projection matrix
    aspectRatio = MathHelper::toFloat(w) / MathHelper::toFloat(h);

    camera->setPerspectiveProjection(camera_zoom, aspectRatio, nearPlane, farPlane);

    // Update World FBO
    //world->setFboSize(QSize(w, h));

    // Update the viewport matrix
    float w2 = w / 2.0f;
    float h2 = h / 2.0f;

    viewportMatrix.setToIdentity();
    viewportMatrix.setColumn(0, QVector4D(w2, 0.0f, 0.0f, 0.0f));
    viewportMatrix.setColumn(1, QVector4D(0.0f, h2, 0.0f, 0.0f));
    viewportMatrix.setColumn(2, QVector4D(0.0f, 0.0f, 1.0f, 0.0f));
    viewportMatrix.setColumn(3, QVector4D(w2, h2, 0.0f, 1.0f));
}

void MapExplorer::keyPressEvent(QKeyEvent* e)
{
    switch(e->key())
    {
        case Qt::Key_W:
            setForwardSpeed(speed * speed_mult);
            break;

        case Qt::Key_S:
            setForwardSpeed(-speed * speed_mult);
            break;

        case Qt::Key_A:
            setSideSpeed(-speed * speed_mult);
            break;

        case Qt::Key_D:
            setSideSpeed(speed * speed_mult);
            break;

        case Qt::Key_Q:
            {
                setForwardSpeed(speed * speed_mult);
                setSideSpeed(-speed * speed_mult);
            }
            break;

        case Qt::Key_E:
            {
                setForwardSpeed(speed * speed_mult);
                setSideSpeed(speed * speed_mult);
            }
            break;

        case Qt::Key_Space:
            setVerticalSpeed(speed * speed_mult);
            break;

        case Qt::Key_X:
            setVerticalSpeed(-speed * speed_mult);
            break;

        case Qt::Key_P:
            setScreenSpaceError(screenSpaceError() + 0.1);
            break;

        case Qt::Key_O:
            setScreenSpaceError(screenSpaceError() - 0.1);
            break;

        /// Used in MainWindow
        case Qt::Key_Plus:
        case Qt::Key_Minus:
        case Qt::Key_F1:
        case Qt::Key_F2:
        case Qt::Key_F3:
        case Qt::Key_F4:
        case Qt::Key_F5:
        case Qt::Key_F6:
        case Qt::Key_F7:
        case Qt::Key_F8:
        case Qt::Key_F9:
        case Qt::Key_F10:
            break;

        default:
            QOpenGLWidget::keyPressEvent(e);
    }
}

void MapExplorer::keyReleaseEvent(QKeyEvent* e)
{
    switch (e->key())
    {
        case Qt::Key_W:
        case Qt::Key_S:
            setForwardSpeed(0.0f);
            break;

        case Qt::Key_D:
        case Qt::Key_A:
            setSideSpeed(0.0f);
            break;

        case Qt::Key_Q:
        case Qt::Key_E:
            {
                setForwardSpeed(0.0f);
                setSideSpeed(0.0f);
            }
            break;

        case Qt::Key_Space:
        case Qt::Key_X:
            setVerticalSpeed(0.0f);
            break;

        default:
            QOpenGLWidget::keyReleaseEvent(e);
    }
}

void MapExplorer::mousePressEvent(QMouseEvent* e)
{
    if(e->button() == Qt::LeftButton)
        leftButtonPressed = wasLeftButtonPressed = true;

    if(e->button() == Qt::RightButton)
        rightButtonPressed = true;

    if(e->button() == Qt::MidButton)
        wheelButtonPressed = wasWheelButtonPressed = true;

    if(leftButtonPressed && rightButtonPressed)
        setForwardSpeed(speed * speed_mult);

    mousePos = prevMousePos = mousePosStart = e->pos();

    QOpenGLWidget::mousePressEvent(e);
}

void MapExplorer::mouseReleaseEvent(QMouseEvent* e)
{
    if(e->button() == Qt::LeftButton)
    {
        leftButtonPressed     = false;
        wasLeftButtonReleased = true;

        if(m_v.z() > 0.0f)
            setForwardSpeed(0.0f);
    }

    if(e->button() == Qt::RightButton)
    {
        rightButtonPressed = false;

        if(m_v.z() > 0.0f)
            setForwardSpeed(0.0f);
    }

    if(e->button() == Qt::MidButton)
    {
        wheelButtonPressed     = false;
        wasWheelButtonReleased = true;
    }

    mousePosEnd = e->pos();

    QOpenGLWidget::mouseReleaseEvent(e);
}

void MapExplorer::mouseMoveEvent(QMouseEvent* e)
{
    mousePos = e->pos();

    float dx =  0.4f * (mousePos.x() - prevMousePos.x());
    float dy = -0.4f * (mousePos.y() - prevMousePos.y());

    if((leftButtonPressed || rightButtonPressed))
    {
        pan(dx);
        tilt(dy);
    }

    prevMousePos  = mousePos;

    mouse_position = this->mapFromGlobal(QCursor::pos());

    QOpenGLWidget::mouseMoveEvent(e);
}

void MapExplorer::wheelEvent(QWheelEvent* e)
{
    QOpenGLWidget::wheelEvent(e);
}

void MapExplorer::focusInEvent(QFocusEvent* e)
{
    leftButtonPressed      = false;
    rightButtonPressed     = false;
    wasLeftButtonPressed   = false;
    wasLeftButtonReleased  = false;
    wheelButtonPressed     = false;
    wasWheelButtonPressed  = false;
    wasWheelButtonReleased = false;

    setForwardSpeed(0.0f);
    setSideSpeed(0.0f);
    setVerticalSpeed(0.0f);

    QOpenGLWidget::focusInEvent(e);
}

void MapExplorer::focusOutEvent(QFocusEvent* e)
{
    leftButtonPressed      = false;
    rightButtonPressed     = false;
    wasLeftButtonPressed   = false;
    wasLeftButtonReleased  = false;
    wheelButtonPressed     = false;
    wasWheelButtonPressed  = false;
    wasWheelButtonReleased = false;

    setForwardSpeed(0.0f);
    setSideSpeed(0.0f);
    setVerticalSpeed(0.0f);

    QOpenGLWidget::focusOutEvent(e);
}
