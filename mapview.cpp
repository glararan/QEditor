#include "mapview.h"
#include "camera.h"

#include <QImage>
#include <QKeyEvent>
#include <QGLWidget>
#include <QOpenGLContext>
#include <QOpenGLFunctions_4_0_Core>

#define _USE_MATH_DEFINES
#include <math.h>

const float degToRad = float(M_PI / 180.0);

MapView::MapView(QWidget* parent)
: QGLWidget(parent)
, m_context(this->context()->contextHandle())
, m_camera(new Camera(this))
, m_v()
, m_viewCenterFixed(false)
, m_panAngle(0.0f)
, m_tiltAngle(0.0f)
, panAngle(0.0f)
, tiltAngle(0.0f)
, camera_zoom(25.0f)
, aspectRatio(static_cast<float>(width()) / static_cast<float>(height()))
, nearPlane(0.1f)
, farPlane(256.0f)
, speed(44.7f) // in m/s. Equivalent to 100 miles/hour)
, speed_mult(1.0f)
, m_patchBuffer(QOpenGLBuffer::VertexBuffer)
, positionData(0)
, m_screenSpaceError(12.0f)
, m_modelMatrix()
, m_horizontalScale(533.33333f)
, m_verticalScale(1.0f)
, m_sunTheta(30.0f)
, m_time(0.0f)
, m_metersToUnits(2 * M_PI / speed) // 500 units == 10 km => 0.05 units/m
, m_leftButtonPressed(false)
, m_rightButtonPressed(false)
, mouse_position(QPoint(0, 0))
, terrain_pos()
, shaping_speed(1.0f)
, shaping_radius(10.0f)
, shaping_radius_multiplier(5.33333f)
, shaping_brush(1)
, shaping_brush_type(1)
, brushColor(0.0f, 1.0f, 0.0f, 1.0f)
, terrainSampler(new Sampler)
, terrainTexture(new Texture)
, shiftDown(false)
, ctrlDown(false)
, altDown(false)
, eMode((eEditingMode)0)
, heightMapImage(0, 0, QImage::Format_Indexed8)
, m_displayMode(TexturedAndLit)
, m_displayModeSubroutines(DisplayModeCount)
, m_funcs(0)
{
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);

    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    format.setMajorVersion(4);
    format.setMinorVersion(3);
    format.setSamples(4);
    format.setProfile(QSurfaceFormat::CoreProfile);

    m_context->setFormat(format);
    m_context->create();

    m_modelMatrix.setToIdentity();

    // Initialize the camera position and orientation
    m_camera->setPosition(QVector3D(250.0f, 10.0f, 250.0f));
    m_camera->setViewCenter(QVector3D(250.0f, 10.0f, 249.0f));
    m_camera->setUpVector(QVector3D(0.0f, 1.0f, 0.0f));

    m_displayModeNames << QStringLiteral("shaderSimple")
                       << QStringLiteral("shadeWorldHeight")
                       << QStringLiteral("shadeWorldNormal")
                       << QStringLiteral("shadeGrass")
                       << QStringLiteral("shadeGrassAndRocks")
                       << QStringLiteral("shadeGrassRocksAndSnow")
                       << QStringLiteral("shadeLightingFactors")
                       << QStringLiteral("shadeTexturedAndLit")
                       << QStringLiteral("shadeWorldTexturedWireframed")
                       << QStringLiteral("shadeHidden");

    AddStatusBarMessage("speed multiplier: ", &speed_mult     , "float");
    AddStatusBarMessage("camera: "          , &m_camera->pos(), "QVector3D_xzy");
    AddStatusBarMessage("zoom: "            , &camera_zoom    , "float");
    AddStatusBarMessage("pan: "             , &panAngle       , "float");
    AddStatusBarMessage("tilt: "            , &tiltAngle      , "float");
    AddStatusBarMessage("mouse: "           , &mouse_position , "QPoint");
    AddStatusBarMessage("terrain coords: "  , &terrain_pos    , "QVector3D_xzy");

    startTimer(16);
}

MapView::~MapView()
{
    delete m_context;
    delete m_camera;

    m_patchBuffer.destroy();
    m_vao.destroy();

    delete m_funcs;

    for(int i = 0; i < 1024 * 1024 * sizeof(float); ++i)
        mapData[i] = 0;
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

    m_funcs = m_context->versionFunctions<QOpenGLFunctions_4_0_Core>();

    if(!m_funcs)
    {
        qFatal("Requires OpenGL >= 4.0");
        exit(1);
    }

    m_funcs->initializeOpenGLFunctions();

    // Initialize resources
    prepareShaders();
    prepareTextures();
    prepareVertexBuffers(m_heightMapSize);
    prepareVertexArrayObject();

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    glClearColor(0.65f, 0.77f, 1.0f, 1.0f);

    // Set the  line properties
    QOpenGLShaderProgramPtr shader = m_material->shader();
    shader->bind();
    shader->setUniformValue("line.width", 0.2f);
    shader->setUniformValue("line.color", QVector4D(0.17f, 0.50f, 1.0f, 1.0f)); // blue

    // Set the fog parameters
    shader->setUniformValue("fog.color", QVector4D(0.65f, 0.77f, 1.0f, 1.0f));
    shader->setUniformValue("fog.minDistance", farPlane / 2);
    shader->setUniformValue("fog.maxDistance", farPlane - 32.0f);

    // Set the brush parameteres
    shader->setUniformValue("brush", 0);
    shader->setUniformValue("cursorPos", QVector2D(0.0f, 0.0f));
    shader->setUniformValue("brushRadius", 0.0f);

    // Get subroutine indices
    for(int i = 0; i < DisplayModeCount; ++i)
        m_displayModeSubroutines[i] = m_funcs->glGetSubroutineIndex(shader->programId(), GL_FRAGMENT_SHADER, m_displayModeNames.at(i).toLatin1());

    m_Utime.start();
}

void MapView::update(float t)
{
    m_modelMatrix.setToIdentity();

    // Store the time
    const float dt = t - m_time;
    m_time         = t;

    // Update the camera position and orientation
    Camera::CameraTranslationOption option = m_viewCenterFixed ? Camera::DontTranslateViewCenter : Camera::TranslateViewCenter;
    m_camera->translate(m_v * dt * m_metersToUnits, option);

    if(!qFuzzyIsNull(m_panAngle))
    {
        m_camera->pan(m_panAngle, QVector3D(0.0f, 1.0f, 0.0f));
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

        m_camera->tilt(m_tiltAngle);
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
    if(camera_zoom != m_camera->fieldOfView())
        m_camera->setPerspectiveProjection(camera_zoom, aspectRatio, nearPlane, farPlane);

    /// mouse on terrain
    if(editingMode() == Terrain)
    {
        // getWorldCoordinates can be used to spawn object in middle of screen
        terrain_pos = getWorldCoordinates(mouse_position.x(), mouse_position.y());
    }

    // Change terrain
    if(m_leftButtonPressed)
    {
        if(shiftDown)
        {
            if(eMode == Terrain)
            {
                const QVector3D& position(terrain_pos);

                changeTerrain(position.x(), position.z(), 7.5f * dt * shaping_speed, shaping_radius / shaping_radius_multiplier, shaping_brush, shaping_brush_type);
            }
        }
        else if(ctrlDown)
        {
            if(eMode == Terrain)
            {
                const QVector3D& position(terrain_pos);

                changeTerrain(position.x(), position.z(), -7.5f * dt * shaping_speed, shaping_radius / shaping_radius_multiplier, shaping_brush, shaping_brush_type);
            }
        }
    }

    // Update status bar
    /*QString sbMessage = QString("speed multiplier: %1, x: %2, y: %3, z: %4, zoom: %5, pan: %6, tilt: %7, mouseX: %8, mouseY: %9")
            .arg(speed_mult)
            .arg(m_camera->position().x())
            .arg(m_camera->position().z())
            .arg(m_camera->position().y())
            .arg(camera_zoom)
            .arg(panAngle)
            .arg(tiltAngle)
            .arg(mouse_position.x())
            .arg(mouse_position.y());*/
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

    m_material->bind();
    QOpenGLShaderProgramPtr shader = m_material->shader();
    shader->bind();

    // Set the fragment shader display mode subroutine
    m_funcs->glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &m_displayModeSubroutines[m_displayMode]);

    // Set the horizontal and vertical scales applied in the tess eval shader
    shader->setUniformValue("horizontalScale", m_horizontalScale);
    shader->setUniformValue("verticalScale", m_verticalScale);
    shader->setUniformValue("pixelsPerTriangleEdge", m_screenSpaceError);

    // Pass in the usual transformation matrices
    QMatrix4x4 viewMatrix        = m_camera->viewMatrix();
    QMatrix4x4 modelViewMatrix   = viewMatrix * m_modelMatrix;
    QMatrix3x3 worldNormalMatrix = m_modelMatrix.normalMatrix();
    QMatrix3x3 normalMatrix      = modelViewMatrix.normalMatrix();
    QMatrix4x4 mvp               = m_camera->projectionMatrix() * modelViewMatrix;

    shader->setUniformValue("modelMatrix", m_modelMatrix);
    shader->setUniformValue("modelViewMatrix", modelViewMatrix);
    shader->setUniformValue("worldNormalMatrix", worldNormalMatrix);
    shader->setUniformValue("normalMatrix", normalMatrix);
    shader->setUniformValue("mvp", mvp);

    // Set the lighting parameters
    QVector4D worldLightDirection(sinf(m_sunTheta * degToRad), cosf(m_sunTheta * degToRad), 0.0f, 0.0f);
    QMatrix4x4 worldToEyeNormal(normalMatrix);
    QVector4D lightDirection = worldToEyeNormal * worldLightDirection;

    shader->setUniformValue("light.position", lightDirection);
    shader->setUniformValue("light.intensity", QVector3D(1.0f, 1.0f, 1.0f));

    // Set the material properties
    shader->setUniformValue("material.Ka", QVector3D(0.1f, 0.1f, 0.1f));
    shader->setUniformValue("material.Kd", QVector3D(1.0f, 1.0f, 1.0f));
    shader->setUniformValue("material.Ks", QVector3D(0.3f, 0.3f, 0.3f));
    shader->setUniformValue("material.shininess", 10.0f);

    if(displayMode() != Hidden)
    {
        // Render the quad as a patch
        {
            QOpenGLVertexArrayObject::Binder binder(&m_vao);
            shader->setPatchVertexCount(1);

            glDrawArrays(GL_PATCHES, 0, m_patchCount);
        }
    }

    /// Terrain brush
    if(eMode == Terrain)
    {
        shader->setUniformValue("brush", shaping_brush);
        shader->setUniformValue("cursorPos", QVector2D(terrain_pos.x(), terrain_pos.z()));
        shader->setUniformValue("brushRadius", shaping_radius);
        shader->setUniformValue("brushRadiusMultiplier", shaping_radius_multiplier);
        shader->setUniformValue("brushColor", brushColor);
    }
    else
    {
        shader->setUniformValue("brush", 0);
        shader->setUniformValue("cursorPos", QVector2D(0.0f, 0.0f));
        shader->setUniformValue("brushRadius", 0.0f);
        shader->setUniformValue("brushRadiusMultiplier", 0.0f);
        shader->setUniformValue("brushColor", QVector4D(0.0f, 0.0f, 0.0f, 0.0f));
    }
}

void MapView::resizeGL(int w, int h)
{
    this->makeCurrent();

    // Make sure the viewport covers the entire window
    glViewport(0, 0, w, h);

    m_viewportSize = QVector2D(float(w), float(h));

    // Update the projection matrix
    aspectRatio = static_cast<float>(w) / static_cast<float>(h);

    m_camera->setPerspectiveProjection(camera_zoom, aspectRatio, nearPlane, farPlane);

    // Update the viewport matrix
    float w2 = w / 2.0f;
    float h2 = h / 2.0f;

    m_viewportMatrix.setToIdentity();
    m_viewportMatrix.setColumn(0, QVector4D(w2, 0.0f, 0.0f, 0.0f));
    m_viewportMatrix.setColumn(1, QVector4D(0.0f, h2, 0.0f, 0.0f));
    m_viewportMatrix.setColumn(2, QVector4D(0.0f, 0.0f, 1.0f, 0.0f));
    m_viewportMatrix.setColumn(3, QVector4D(w2, h2, 0.0f, 1.0f));

    // We need the viewport size to calculate tessellation levels
    QOpenGLShaderProgramPtr shader = m_material->shader();
    shader->setUniformValue("viewportSize", m_viewportSize);

    // The geometry shader also needs the viewport matrix
    shader->setUniformValue("viewportMatrix", m_viewportMatrix);
}

void MapView::prepareShaders()
{
    m_material = MaterialPtr(new Material);
    m_material->setShaders(":/shaders/terraintessellation.vert",
                           ":/shaders/terraintessellation.tcs",
                           ":/shaders/terraintessellation.tes",
                           ":/shaders/terraintessellation.geom",
                           ":/shaders/terraintessellation.frag");
}

void MapView::prepareTextures()
{
    //SamplerPtr sampler(new Sampler);
    terrainSampler->create();
    terrainSampler->setMinificationFilter(GL_LINEAR);
    terrainSampler->setMagnificationFilter(GL_LINEAR);
    terrainSampler->setWrapMode(Sampler::DirectionS, GL_CLAMP_TO_EDGE);
    terrainSampler->setWrapMode(Sampler::DirectionT, GL_CLAMP_TO_EDGE);

    for(int i = 0; i < 1024 * 1024 * sizeof(float); ++i)
        mapData[i] = 0.1f * (qrand() % 5);

    //heightMapImage.load("heightmap-1024x1024.png");

    m_funcs->glActiveTexture(GL_TEXTURE0);

    //TexturePtr heightMap(new Texture);
    terrainTexture->create();
    terrainTexture->bind();
    //terrainTexture->setImage(heightMapImage);
    terrainTexture->setImage(&mapData, 1024, 1024);

    m_heightMapSize = QSize(1024, 1024);//heightMapImage.size();
    m_material->setTextureUnitConfiguration(0, terrainTexture, terrainSampler, QByteArrayLiteral("heightMap"));

    SamplerPtr tilingSampler(new Sampler);
    tilingSampler->create();
    tilingSampler->setMinificationFilter(GL_LINEAR_MIPMAP_LINEAR);
    m_funcs->glSamplerParameterf(tilingSampler->samplerId(), GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f);
    tilingSampler->setMagnificationFilter(GL_LINEAR);
    tilingSampler->setWrapMode(Sampler::DirectionS, GL_REPEAT);
    tilingSampler->setWrapMode(Sampler::DirectionT, GL_REPEAT);

    QImage grassImage("grass.png");
    m_funcs->glActiveTexture(GL_TEXTURE1);

    TexturePtr grassTexture(new Texture);
    grassTexture->create();
    grassTexture->bind();
    grassTexture->setImage(grassImage);
    grassTexture->generateMipMaps();
    m_material->setTextureUnitConfiguration(1, grassTexture, tilingSampler, QByteArrayLiteral("grassTexture"));

    QImage rockImage("rock.png");
    m_funcs->glActiveTexture(GL_TEXTURE2);

    TexturePtr rockTexture(new Texture);
    rockTexture->create();
    rockTexture->bind();
    rockTexture->setImage(rockImage);
    rockTexture->generateMipMaps();
    m_material->setTextureUnitConfiguration(2, rockTexture, tilingSampler, QByteArrayLiteral("rockTexture"));

    QImage snowImage("snowrocks.png");
    m_funcs->glActiveTexture(GL_TEXTURE3);

    TexturePtr snowTexture(new Texture);
    snowTexture->create();
    snowTexture->bind();
    snowTexture->setImage(snowImage);
    snowTexture->generateMipMaps();
    m_material->setTextureUnitConfiguration(3, snowTexture, tilingSampler, QByteArrayLiteral("snowTexture"));

    m_funcs->glActiveTexture(GL_TEXTURE0);
}

void MapView::prepareVertexBuffers(QSize heightMapSize)
{
    // Generate patch primitive data to cover the heightmap texture

    // Each patch consists of a single point located at the lower-left corner
    // of a rectangle (in the xz-plane)
    const int maxTessellationLevel = 64;
    const int trianglesPerHeightSample = 10;
    const int xDivisions = trianglesPerHeightSample * heightMapSize.width() / maxTessellationLevel;
    const int zDivisions = trianglesPerHeightSample * heightMapSize.height() / maxTessellationLevel;

    m_patchCount = xDivisions * zDivisions;
    positionData.resize(2 * m_patchCount); // 2 floats per vertex

    qDebug() << "Total number of patches =" << m_patchCount;

    const float dx = 1.0f / static_cast<float>(xDivisions);
    const float dz = 1.0f / static_cast<float>(zDivisions);

    for(int j = 0; j < 2 * zDivisions; j += 2)
    {
        float z = static_cast<float>(j) * dz * 0.5;

        for(int i = 0; i < 2 * xDivisions; i += 2)
        {
            float x         = static_cast<float>(i) * dx * 0.5;
            const int index = xDivisions * j + i;

            positionData[index]     = x;
            positionData[index + 1] = z;
        }
    }

    m_patchBuffer.create();
    m_patchBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    m_patchBuffer.bind();
    m_patchBuffer.allocate(positionData.data(), positionData.size() * sizeof(float));
    m_patchBuffer.release();
}

void MapView::prepareVertexArrayObject()
{
    // Create a VAO for this "object"
    m_vao.create();
    {
        QOpenGLVertexArrayObject::Binder binder(&m_vao);
        QOpenGLShaderProgramPtr shader = m_material->shader();

        shader->bind();
        m_patchBuffer.bind();
        shader->enableAttributeArray("vertexPosition");
        shader->setAttributeBuffer("vertexPosition", GL_FLOAT, 0, 2);
    }
}

void MapView::SpeedMultiplier(float multiplier)
{
    speed_mult = multiplier;

    if(speed_mult < 0.0f)
        speed_mult = 0.0f;

    if(speed_mult > 10.0f)
        speed_mult = 10.0f;
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
            eMode = (eEditingMode)option;
            break;

        default:
            eMode = (eEditingMode)0;
            break;
    }
}

void MapView::setDisplayMode(int mode)
{
    if(mode == SimpleWireFrame || mode == WorldHeight || mode == WorldTexturedWireframed)
    {
        QOpenGLShaderProgramPtr shader = m_material->shader();
        shader->bind();

        if(mode == WorldTexturedWireframed)
            shader->setUniformValue("line.width", 0.5f);
        else
            shader->setUniformValue("line.width", 0.2f);
    }

    switch(mode)
    {
        case SimpleWireFrame:
        case WorldHeight:
        case WorldNormals:
        case Grass:
        case GrassAndRocks:
        case GrassRocksAndSnow:
        case LightingFactors:
        case TexturedAndLit:
        case WorldTexturedWireframed:
        case Hidden:
            setDisplayMode((DisplayMode)mode);
            break;

        default:
            setDisplayMode((DisplayMode)0);
            break;
    }
}

int MapView::horizToHMapSize(float position)
{
    return static_cast<int>(position / m_horizontalScale * m_heightMapSize.width());
}

float MapView::HMapSizeToHoriz(int position)
{
    return static_cast<float>(position) / static_cast<float>(m_heightMapSize.width()) * m_horizontalScale;
}

float MapView::swapPosition(float position)
{
    return m_horizontalScale - position;
}

float MapView::swapPositionBack(float position)
{
    return m_heightMapSize.width() - position;
}

int MapView::swapPositionBackInt(int position)
{
    return m_heightMapSize.width() - position;
}

bool MapView::changeTerrain(float x, float z, float change, float radius, int brush, int brush_type)
{
    bool changed = false;

    int minX = horizToHMapSize(x - radius);
    int maxX = horizToHMapSize(x + radius);

    int minY = horizToHMapSize(swapPosition(z + radius));
    int maxY = horizToHMapSize(swapPosition(z - radius));

    for(int _x = minX; _x < maxX; ++_x)
    {
        for(int _y = minY; _y < maxY; ++_y) // _y mean z!
        {
            switch(brush)
            {
                case 1: // Circle
                default:
                    {
                        switch(brush_type)
                        {
                            case 1: // Linear
                            default:
                                {
                                    float xdiff = HMapSizeToHoriz(_x) - x;
                                    float ydiff = HMapSizeToHoriz(swapPositionBack(_y)) - z;

                                    float dist = sqrt(xdiff * xdiff + ydiff * ydiff);

                                    float changeFormula = change * (1.0f - dist / radius);

                                    if(dist < radius)
                                    {
                                        int X = _x % m_heightMapSize.width();
                                        int Y = swapPositionBackInt(_y) % m_heightMapSize.width();

                                        int index = (Y * m_heightMapSize.width()) + X;

                                        mapData[index * 4] += changeFormula;

                                        changed = true;
                                    }
                                }
                                break;

                            case 2: // Flat
                                {
                                    float xdiff = HMapSizeToHoriz(_x) - x;
                                    float ydiff = HMapSizeToHoriz(swapPositionBack(_y)) - z;

                                    float dist = sqrt(xdiff * xdiff + ydiff * ydiff);

                                    float changeFormula = change;

                                    if(dist < radius)
                                    {
                                        int X = _x % m_heightMapSize.width();
                                        int Y = swapPositionBackInt(_y) % m_heightMapSize.width();

                                        int index = (Y * m_heightMapSize.width()) + X;

                                        mapData[index * 4] += changeFormula;

                                        changed = true;
                                    }
                                }
                                break;

                            case 3: // Smooth
                                {
                                    float xdiff = HMapSizeToHoriz(_x) - x;
                                    float ydiff = HMapSizeToHoriz(swapPositionBack(_y)) - z;

                                    float dist = sqrt(xdiff * xdiff + ydiff * ydiff);

                                    float changeFormula = change / (1.0f + dist / radius);

                                    if(dist < radius)
                                    {
                                        int X = _x % m_heightMapSize.width();
                                        int Y = swapPositionBackInt(_y) % m_heightMapSize.width();

                                        int index = (Y * m_heightMapSize.width()) + X;

                                        mapData[index * 4] += changeFormula;

                                        changed = true;
                                    }
                                }
                                break;

                            case 4: // ...
                                {
                                    float xdiff = HMapSizeToHoriz(_x) - x;
                                    float ydiff = HMapSizeToHoriz(swapPositionBack(_y)) - z;

                                    float dist = sqrt(xdiff * xdiff + ydiff * ydiff);

                                    float changeFormula = change * ((dist / radius) * (dist / radius) + dist / radius + 1.0f);

                                    if(dist < radius)
                                    {
                                        int X = _x % m_heightMapSize.width();
                                        int Y = swapPositionBackInt(_y) % m_heightMapSize.width();

                                        int index = (Y * m_heightMapSize.width()) + X;

                                        mapData[index * 4] += changeFormula;

                                        changed = true;
                                    }
                                }
                                break;

                            case 5: // ...
                                {
                                    float xdiff = HMapSizeToHoriz(_x) - x;
                                    float ydiff = HMapSizeToHoriz(swapPositionBack(_y)) - z;

                                    float dist = sqrt(xdiff * xdiff + ydiff * ydiff);

                                    float changeFormula = change * cos(dist / radius);

                                    if(dist < radius)
                                    {
                                        int X = _x % m_heightMapSize.width();
                                        int Y = swapPositionBackInt(_y) % m_heightMapSize.width();

                                        int index = (Y * m_heightMapSize.width()) + X;

                                        mapData[index * 4] += changeFormula;

                                        changed = true;
                                    }
                                }
                                break;
                        }
                    }
                    break;
            }
        }
    }

    if(changed)
    {
        terrainTexture->updateImage(&mapData, 1024, 1024);

        m_material->setTextureUnitConfiguration(0, terrainTexture, terrainSampler, QByteArrayLiteral("heightMap"));
    }
    else
        qDebug() << "changed: " << changed;

    return changed;
}

void MapView::doTest()
{
}

void MapView::setCameraPosition(QVector3D* position)
{
    const QVector3D location(position->x(), position->y(), position->z());

    // reset tilt angle
    tiltAngle = 0.0f;
    panAngle  = 0.0f;

    // set position and rotation
    m_camera->setPosition(location);
    m_camera->setViewCenter(QVector3D(location.x(), location.y(), location.z() - 1.0f));
    m_camera->setUpVector(QVector3D(0.0f, 1.0f, 0.0f));
}

QVector3D MapView::getWorldCoordinates(float mouseX, float mouseY)
{
    QMatrix4x4 viewMatrix       = m_camera->viewMatrix();
    QMatrix4x4 modelViewMatrix  = viewMatrix * m_modelMatrix;
    QMatrix4x4 modelViewProject = m_camera->projectionMatrix() * modelViewMatrix;
    QMatrix4x4 inverted         = m_viewportMatrix * modelViewProject;

    inverted = inverted.inverted();

    float posZ;
    float posY = m_viewportSize.y() - mouseY - 1.0f;

    m_funcs->glReadPixels((int)mouseX, (int)posY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &posZ);

    QVector4D clickedPointOnScreen(mouseX, posY, 2.0f * posZ - 1.0f, 1.0f);
    QVector4D clickedPointIn3DOrgn = inverted * clickedPointOnScreen;

    clickedPointIn3DOrgn /= clickedPointIn3DOrgn.w();

    return clickedPointIn3DOrgn.toVector3DAffine();
}

void MapView::resetCamera()
{
    QVector3D Cpos = m_camera->position();

    m_camera->setUpVector(QVector3D(0.0f, 1.0f, 0.0f));
    m_camera->setPosition(Cpos);

    Cpos.setZ(Cpos.z() - 1.0f);

    m_camera->setViewCenter(Cpos);

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
}

void MapView::setShapingBrush(int brush)
{
    shaping_brush = brush;
}

void MapView::setShapingBrushType(int type)
{
    shaping_brush_type = type + 1;
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
            //setTerrainVerticalScale(terrainVerticalScale() + 0.05);
            break;

        case Qt::Key_End:
            //setTerrainVerticalScale(terrainVerticalScale() - 0.05);
            break;

        case Qt::Key_BracketLeft:
            setSunAngle(sunAngle() - 0.2);
            break;

        case Qt::Key_BracketRight:
            setSunAngle(sunAngle() + 0.2);
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
    {
        m_leftButtonPressed = true;
        m_pos = m_prevPos = e->pos();
    }

    if(e->button() == Qt::RightButton)
    {
        m_rightButtonPressed = true;
        m_pos = m_prevPos = e->pos();
    }

    if(m_leftButtonPressed && m_rightButtonPressed)
    {
        setForwardSpeed(speed * speed_mult);

        m_pos = m_prevPos = e->pos();
    }

    QGLWidget::mousePressEvent(e);
}

void MapView::mouseReleaseEvent(QMouseEvent* e)
{
    if(e->button() == Qt::LeftButton)
    {
        m_leftButtonPressed = false;

        if(m_v.z() > 0.0f)
            setForwardSpeed(0.0f);
    }

    if(e->button() == Qt::RightButton)
    {
        m_rightButtonPressed = false;

        if(m_v.z() > 0.0f)
            setForwardSpeed(0.0f);
    }

    QGLWidget::mouseReleaseEvent(e);
}

void MapView::mouseMoveEvent(QMouseEvent* e)
{
    m_pos = e->pos();

    float dx = 0.4f * (m_pos.x() - m_prevPos.x());
    float dy = -0.4f * (m_pos.y() - m_prevPos.y());

    if((m_leftButtonPressed || m_rightButtonPressed) && !shiftDown && !altDown && !ctrlDown)
    {
        pan(dx);
        tilt(dy);
    }
    else if(m_leftButtonPressed && altDown)
    {
        if(eMode == Terrain)
            updateShapingRadius(dx);
    }

    m_prevPos = m_pos;

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

    m_leftButtonPressed  = false;
    m_rightButtonPressed = false;

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

    m_leftButtonPressed  = false;
    m_rightButtonPressed = false;

    setForwardSpeed(0.0f);
    setSideSpeed(0.0f);
    setVerticalSpeed(0.0f);

    QGLWidget::focusOutEvent(e);
}
