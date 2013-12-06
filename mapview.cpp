#include "mapview.h"
#include "camera.h"
#include "mathhelper.h"
#include "qeditor.h"

#include <QKeyEvent>
#include <QOpenGLContext>

MapView::MapView(World* mWorld, QWidget* parent)
: QGLWidget(parent)
, world(mWorld)
, GLcontext(this->context()->contextHandle())
, camera(new Camera(this))
, m_v()
, viewCenterFixed(false)
, m_panAngle(0.0f)
, m_tiltAngle(0.0f)
, panAngle(0.0f)
, tiltAngle(0.0f)
, camera_zoom(25.0f)
, aspectRatio(static_cast<float>(width()) / static_cast<float>(height()))
, nearPlane(0.01f)
, farPlane(app().getSetting("environmentDistance", 256.0f).toFloat())
, speed(44.7f) // in m/s. Equivalent to 100 miles/hour)
, speed_mult(app().getSetting("speedMultiplier", 1.0f).toFloat())
, screenSpaceErrorLevel(12.0f)
, modelMatrix()
, time(0.0f)
, m_metersToUnits(4 * MathHelper::PI() / speed) // 500 units == 10 km => 0.05 units/m
, fps(0)
, leftButtonPressed(false)
, rightButtonPressed(false)
, mouse_position(QPoint(0, 0))
, terrain_pos()
, shaping_speed(1.0f)
, shaping_radius(10.0f)
, shaping_radius_multiplier(5.33333f)
, shaping_brush(1)
, shaping_brush_type(1)
, shiftDown(false)
, ctrlDown(false)
, altDown(false)
, eMode(Objects)
, eTerrain(Shaping)
{
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);

    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    format.setMajorVersion(4);
    format.setMinorVersion(3);
    format.setSamples(4);
    format.setProfile(QSurfaceFormat::CoreProfile);

    GLcontext->setFormat(format);
    GLcontext->create();

    modelMatrix.setToIdentity();

    // Initialize the camera position and orientation
    camera->setPosition(QVector3D(250.0f, 10.0f, 250.0f));
    camera->setViewCenter(QVector3D(250.0f, 10.0f, 249.0f));
    camera->setUpVector(QVector3D(0.0f, 1.0f, 0.0f));

    // Assign camera to World
    world->setCamera(camera);

    AddStatusBarMessage("FPS: "             , &fps           , "int");
    AddStatusBarMessage("speed multiplier: ", &speed_mult    , "float");
    AddStatusBarMessage("camera: "          , &camera->pos() , "QVector3D_xzy");
    AddStatusBarMessage("zoom: "            , &camera_zoom   , "float");
    AddStatusBarMessage("pan: "             , &panAngle      , "float");
    AddStatusBarMessage("tilt: "            , &tiltAngle     , "float");
    AddStatusBarMessage("mouse: "           , &mouse_position, "QPoint");
    AddStatusBarMessage("terrain coords: "  , &terrain_pos   , "QVector3D_xzy");

    startTimer(16);
}

MapView::~MapView()
{
    delete camera;
}

void MapView::timerEvent(QTimerEvent*)
{
    this->makeCurrent();

    const qreal time = m_Utime.elapsed() / 1000.0f;

    update(time);
    updateGL();
}

void MapView::initializeGL()
{
    this->makeCurrent();

    // Initialize World
    world->initialize(GLcontext);

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    glClearColor(0.65f, 0.77f, 1.0f, 1.0f);

    m_Utime.start();

    emit initialized();
}

void MapView::update(float t)
{
    modelMatrix.setToIdentity();

    // Store the time
    const float dt = t - time;
    time           = t;

    // FPS
    fps = 1000.0f / (dt * 1000.0f);

    // Update the camera position and orientation
    Camera::CameraTranslationOption option = viewCenterFixed ? Camera::DontTranslateViewCenter : Camera::TranslateViewCenter;

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

    // Dynamic zoom
    if(dynamicZoom.count() > 0)
    {
        QVector<int> posIndexs;
        QVector<int> negIndexs;

        for(int i = 0; i < dynamicZoom.count(); i++)
        {
            if(dynamicZoom[i] == 0.5f)
                posIndexs.append(i);
            else if(dynamicZoom[i] == -0.5f)
                negIndexs.append(i);
        }

        if(posIndexs.count() >= negIndexs.count() && posIndexs.count() > 0)
        {
            for(int i = 0; i < posIndexs.count(); i++)
            {
                if(negIndexs.count() == 0)
                    break;

                dynamicZoom.remove(posIndexs[i]);
                dynamicZoom.remove(negIndexs[i]);

                posIndexs.remove(i);
                negIndexs.remove(i);
            }
        }
        else if(negIndexs.count() > posIndexs.count() && negIndexs.count() > 0)
        {
            for(int i = 0; i < negIndexs.count(); i++)
            {
                if(posIndexs.count() == 0)
                    break;

                dynamicZoom.remove(posIndexs[i]);
                dynamicZoom.remove(negIndexs[i]);

                posIndexs.remove(i);
                negIndexs.remove(i);
            }
        }

        if(dynamicZoom[0] > 0)
        {
            dynamicZoom[0] += 0.5f;

            if(dynamicZoom[0] > 13)
                dynamicZoom.remove(0);
            else
               setFieldOfView(0.02f);
        }
        else
        {
            dynamicZoom[0] -= 0.5f;

            if(dynamicZoom[0] < -13)
                dynamicZoom.remove(0);
            else
                setFieldOfView(-0.02f);
        }

        if(camera_zoom > 25.0f)
            camera_zoom = 25.0f;
        else if(camera_zoom < 20.0f)
            camera_zoom = 20.0f;
    }

    // Update the camera perspective projection if camera zoom is changed
    if(camera_zoom != camera->fieldOfView())
        camera->setPerspectiveProjection(camera_zoom, aspectRatio, nearPlane, farPlane);

    /// mouse on terrain
    if(eMode == Terrain)
    {
        // getWorldCoordinates can be used to spawn object in middle of screen
        terrain_pos = getWorldCoordinates(mouse_position.x(), mouse_position.y());
    }

    // Change terrain
    if(leftButtonPressed)
    {
        const QVector3D& position(terrain_pos);

        if(shiftDown)
        {
            if(eMode == Terrain)
            {
                if(eTerrain == Shaping)
                    world->changeTerrain(position.x(), position.z(), 7.5f * dt * shaping_speed, shaping_brush_type);
                else if(eTerrain == Smoothing)
                    world->flattenTerrain(position.x(), position.z(), position.y(), pow(0.2f, dt) * shaping_speed, shaping_brush_type);
            }
            else if(eMode == Texturing)
            {
                world->paintTerrain();
            }
        }
        else if(ctrlDown)
        {
            if(eMode == Terrain)
            {
                if(eTerrain == Shaping)
                    world->changeTerrain(position.x(), position.z(), -7.5f * dt * shaping_speed, shaping_brush_type);
                else if(eTerrain == Smoothing)
                    world->blurTerrain(position.x(), position.z(), pow(0.2f, dt) * shaping_speed, shaping_brush_type);
            }
            else if(eMode == Texturing)
            {
                world->paintTerrain();
            }
        }
    }

    world->update(t);

    // Update status bar
    QString sbMessage = "Initialized!";

    if(sbMessageList.count() > 0)
    {
        for(int i = 0; i < sbMessageList.count(); i++)
        {
            QVariant sbData = NULL;

            if(!sbDataTypeList.isEmpty() && !sbDataList.isEmpty())
            {
                if(sbDataTypeList[i] != "")
                {
                    if(sbDataTypeList[i] == "bool")
                        sbData = *(bool*)sbDataList.at(i);
                    else if(sbDataTypeList[i] == "byte")
                        sbData = *(byte*)sbDataList.at(i);
                    else if(sbDataTypeList[i] == "char")
                        sbData = *(char*)sbDataList.at(i);
                    else if(sbDataTypeList[i] == "int")
                        sbData = *(int*)sbDataList.at(i);
                    else if(sbDataTypeList[i] == "long")
                        sbData = *(long*)sbDataList.at(i);
                    else if(sbDataTypeList[i] == "float")
                        sbData = *(float*)sbDataList.at(i);
                    else if(sbDataTypeList[i] == "double")
                        sbData = *(double*)sbDataList.at(i);
                    else if(sbDataTypeList[i] == "uint")
                        sbData = *(uint*)sbDataList.at(i);
                    else if(sbDataTypeList[i] == "QString")
                        sbData = *(QString*)sbDataList.at(i);
                    else if(sbDataTypeList[i] == "QVector3D")
                        sbData = QString("(x: %1, y: %2, z: %3)").arg((*(QVector3D*)sbDataList.at(i)).x()).arg((*(QVector3D*)sbDataList.at(i)).y()).arg((*(QVector3D*)sbDataList.at(i)).z());
                    else if(sbDataTypeList[i] == "QVector3D_xzy")
                        sbData = QString("(x: %1, y: %2, z: %3)").arg((*(QVector3D*)sbDataList.at(i)).x()).arg((*(QVector3D*)sbDataList.at(i)).z()).arg((*(QVector3D*)sbDataList.at(i)).y());
                    else if(sbDataTypeList[i] == "QPoint")
                        sbData = QString("(x: %1, y: %2)").arg((*(QPoint*)sbDataList.at(i)).x()).arg((*(QPoint*)sbDataList.at(i)).y());
                }
            }

            QString combine = QString(sbMessageList[i] + "%1").arg(sbData.toString());

            if(i == 0)
                sbMessage = combine;
            else
                sbMessage += ", " + combine;
        }
    }

    emit statusBar(sbMessage);

    sbMessage.clear();
}

void MapView::paintGL()
{
    this->makeCurrent();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if(eMode == Terrain || eMode == Texturing)
        world->draw(modelMatrix, screenSpaceErrorLevel, QVector2D(terrain_pos.x(), terrain_pos.z()), true);
    else
        world->draw(modelMatrix, screenSpaceErrorLevel, QVector2D(terrain_pos.x(), terrain_pos.z()));
}

void MapView::resizeGL(int w, int h)
{
    this->makeCurrent();

    // Make sure the viewport covers the entire window
    glViewport(0, 0, w, h);

    viewportSize = QVector2D(float(w), float(h));

    // Update the projection matrix
    aspectRatio = static_cast<float>(w) / static_cast<float>(h);

    camera->setPerspectiveProjection(camera_zoom, aspectRatio, nearPlane, farPlane);

    // Update the viewport matrix
    float w2 = w / 2.0f;
    float h2 = h / 2.0f;

    viewportMatrix.setToIdentity();
    viewportMatrix.setColumn(0, QVector4D(w2, 0.0f, 0.0f, 0.0f));
    viewportMatrix.setColumn(1, QVector4D(0.0f, h2, 0.0f, 0.0f));
    viewportMatrix.setColumn(2, QVector4D(0.0f, 0.0f, 1.0f, 0.0f));
    viewportMatrix.setColumn(3, QVector4D(w2, h2, 0.0f, 1.0f));

    // We need the viewport size to calculate tessellation levels
    world->setChunkShaderUniform("viewportSize", viewportSize);

    // The geometry shader also needs the viewport matrix
    world->setChunkShaderUniform("viewportMatrix", viewportMatrix);
}

void MapView::SpeedMultiplier(float multiplier)
{
    speed_mult = multiplier;

    if(speed_mult < 0.0f)
        speed_mult = 0.0f;

    if(speed_mult > 10.0f)
        speed_mult = 10.0f;

    app().setSetting("speedMultiplier", speed_mult);

    world->getBrush()->setMultiplier(speed_mult);
}

void MapView::setSpeedMultiplier(float value)
{
    if(value >= 1 || value == 0)
        SpeedMultiplier(value);
    else
        SpeedMultiplier(speed_mult + value);
}

void MapView::AddStatusBarMessage(const QString message)
{
    sbMessageList << message;
    sbDataList.append(NULL);
    sbDataTypeList.append("");
}

void MapView::AddStatusBarMessage(const QString message, const void* data, const QString data_type)
{
    sbMessageList << message;
    sbDataList.append(data);
    sbDataTypeList.append(data_type);
}

void MapView::ClearStatusBarMessage()
{
    sbMessageList.clear();
    sbDataList.clear();
}

void MapView::setModeEditing(int option)
{
    switch(option)
    {
        case Objects:
        case Terrain:
        case Texturing:
            eMode = (eEditingMode)option;
            break;

        default:
            eMode = Objects;
            break;
    }
}

void MapView::setDisplayMode(int mode)
{
    world->setDisplayMode(mode);
}

void MapView::doTest()
{
    world->test();
}

void MapView::setCameraPosition(QVector3D* position)
{
    // reset tilt and pan angle
    tiltAngle = 0.0f;
    panAngle  = 0.0f;

    camera->moveToPosition(*position);
}

QVector3D MapView::getWorldCoordinates(float mouseX, float mouseY)
{
    QMatrix4x4 viewMatrix       = camera->viewMatrix();
    QMatrix4x4 modelViewMatrix  = viewMatrix * modelMatrix;
    QMatrix4x4 modelViewProject = camera->projectionMatrix() * modelViewMatrix;
    QMatrix4x4 inverted         = viewportMatrix * modelViewProject;

    inverted = inverted.inverted();

    float posZ;
    float posY = viewportSize.y() - mouseY - 1.0f;

    world->getGLFunctions()->glReadPixels((int)mouseX, (int)posY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &posZ);

    QVector4D clickedPointOnScreen(mouseX, posY, 2.0f * posZ - 1.0f, 1.0f);
    QVector4D clickedPointIn3DOrgn = inverted * clickedPointOnScreen;

    clickedPointIn3DOrgn /= clickedPointIn3DOrgn.w();

    return clickedPointIn3DOrgn.toVector3DAffine();
}

void MapView::resetCamera()
{
    camera->resetRotation();

    panAngle  = 0.0f;
    tiltAngle = 0.0f;
}

void MapView::setShapingSpeed(double speed)
{
    shaping_speed = static_cast<float>(speed);
}

void MapView::setShapingRadius(double radius)
{
    shaping_radius = static_cast<float>(radius);

    world->getBrush()->setRadius(shaping_radius);
}

void MapView::setShapingBrush(int brush)
{
    shaping_brush = brush;

    world->getBrush()->setBrush(shaping_brush);
}

void MapView::setShapingBrushType(int type)
{
    shaping_brush_type = type + 1;
}

void MapView::setTerrainMode(int mode)
{
    eTerrain = (eTerrainMode)mode;
}

void MapView::setBrushColor(QColor* color)
{
    world->getBrush()->setColor(*color);

    app().setSetting("brushColor", *color);
}

void MapView::setEnvionmentDistance(float value)
{
    farPlane = value;

    app().setSetting("environmentDistance", farPlane);

    world->setChunkShaderUniform("fog.minDistance", farPlane / 2);
    world->setChunkShaderUniform("fog.maxDistance", farPlane - 32.0f);

    camera->setPerspectiveProjection(camera_zoom, aspectRatio, nearPlane, farPlane);
}

void MapView::setTextureScaleOption_(int option)
{
    app().setSetting("textureScaleOption", option);

    world->setChunkShaderUniform("textureScaleOption", option);
}

void MapView::setTextureScaleFar(float value)
{
    app().setSetting("textureScaleFar", value);

    world->setChunkShaderUniform("textureScaleFar", value);
}

void MapView::setTextureScaleNear(float value)
{
    app().setSetting("textureScaleNear", value);

    world->setChunkShaderUniform("textureScaleNear", value);
}

void MapView::setTurnChunkLines(bool on)
{
    app().setSetting("chunkLines", on);

    world->setChunkShaderUniform("chunkLines", on);
}

void MapView::save()
{
    world->save();
}

void MapView::keyPressEvent(QKeyEvent* e)
{
    switch (e->key())
    {
        case Qt::Key_Escape:
            exit(0);
            break;

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

        case Qt::Key_Shift:
            //setViewCenterFixed(true);
            shiftDown = true;
            break;

        case Qt::Key_Control:
            ctrlDown = true;
            break;

        case Qt::Key_Alt:
            altDown = true;
            break;

        case Qt::Key_Home:
            world->setSunAngle(world->sunAngle() - 0.2);
            break;

        case Qt::Key_End:
            world->setSunAngle(world->sunAngle() + 0.2);
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
            QGLWidget::keyPressEvent(e);
    }
}

void MapView::keyReleaseEvent(QKeyEvent* e)
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

        case Qt::Key_Shift:
            //setViewCenterFixed(false);
            shiftDown = false;
            break;

        case Qt::Key_Control:
            ctrlDown = false;
            break;

        case Qt::Key_Alt:
            altDown = false;
            break;

        default:
            QGLWidget::keyReleaseEvent(e);
    }
}

void MapView::mousePressEvent(QMouseEvent* e)
{
    if(e->button() == Qt::LeftButton)
        leftButtonPressed = true;

    if(e->button() == Qt::RightButton)
        rightButtonPressed = true;

    if(leftButtonPressed && rightButtonPressed)
        setForwardSpeed(speed * speed_mult);

    mousePos = prevMousePos = e->pos();

    QGLWidget::mousePressEvent(e);
}

void MapView::mouseReleaseEvent(QMouseEvent* e)
{
    if(e->button() == Qt::LeftButton)
    {
        leftButtonPressed = false;

        if(m_v.z() > 0.0f)
            setForwardSpeed(0.0f);
    }

    if(e->button() == Qt::RightButton)
    {
        rightButtonPressed = false;

        if(m_v.z() > 0.0f)
            setForwardSpeed(0.0f);
    }

    QGLWidget::mouseReleaseEvent(e);
}

void MapView::mouseMoveEvent(QMouseEvent* e)
{
    mousePos = e->pos();

    float dx =  0.4f * (mousePos.x() - prevMousePos.x());
    float dy = -0.4f * (mousePos.y() - prevMousePos.y());

    if((leftButtonPressed || rightButtonPressed) && !shiftDown && !altDown && !ctrlDown)
    {
        pan(dx);
        tilt(dy);
    }
    else if(leftButtonPressed && altDown)
    {
        if(eMode == Terrain)
            updateShapingRadius(dx);
    }

    prevMousePos = mousePos;

    mouse_position = this->mapFromGlobal(QCursor::pos());

    QGLWidget::mouseMoveEvent(e);
}

void MapView::wheelEvent(QWheelEvent* e)
{
    if(!shiftDown && !altDown && !ctrlDown)
        dynamicZoom.push_back(-(static_cast<float>(e->delta()) / 240));

    QGLWidget::wheelEvent(e);
}

void MapView::focusInEvent(QFocusEvent* e)
{
    altDown   = false;
    ctrlDown  = false;
    shiftDown = false;

    leftButtonPressed  = false;
    rightButtonPressed = false;

    setForwardSpeed(0.0f);
    setSideSpeed(0.0f);
    setVerticalSpeed(0.0f);

    QGLWidget::focusInEvent(e);
}

void MapView::focusOutEvent(QFocusEvent* e)
{
    altDown   = false;
    ctrlDown  = false;
    shiftDown = false;

    leftButtonPressed  = false;
    rightButtonPressed = false;

    setForwardSpeed(0.0f);
    setSideSpeed(0.0f);
    setVerticalSpeed(0.0f);

    QGLWidget::focusOutEvent(e);
}