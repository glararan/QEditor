#include "terraintessellationscene.h"
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
, speed_mult(1.0f)
, m_patchBuffer(QOpenGLBuffer::VertexBuffer)
, m_screenSpaceError(12.0f)
, m_modelMatrix()
, m_horizontalScale(500.0f)
, m_verticalScale(20.0f)
, m_sunTheta(30.0f)
, m_time(0.0f)
, m_metersToUnits(0.05f) // 500 units == 10 km => 0.05 units/m
, m_leftButtonPressed(false)
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

    m_displayModeNames << QStringLiteral("shaderSimpleWireFrame")
                       << QStringLiteral("shadeWorldHeight")
                       << QStringLiteral("shadeWorldNormal")
                       << QStringLiteral("shadeGrass")
                       << QStringLiteral("shadeGrassAndRocks")
                       << QStringLiteral("shadeGrassRocksAndSnow")
                       << QStringLiteral("shadeLightingFactors")
                       << QStringLiteral("shadeTexturedAndLit");

    AddStatusBarMessage("speed multiplier: ", speed_mult);

    startTimer(16);
}

MapView::~MapView()
{
    delete m_context;
    delete m_camera;
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
    //m_context->makeCurrent()
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

    // Set the wireframe line properties
    QOpenGLShaderProgramPtr shader = m_material->shader();
    shader->bind();
    shader->setUniformValue("line.width", 0.2f);
    //shader->setUniformValue("line.color", QVector4D(1.0f, 1.0f, 1.0f, 0.0f)); // white
    shader->setUniformValue("line.color", QVector4D(0.17f, 0.50f, 1.0f, 1.0f)); // blue

    // Set the fog parameters
    shader->setUniformValue("fog.color", QVector4D(0.65f, 0.77f, 1.0f, 1.0f));
    shader->setUniformValue("fog.minDistance", 50.0f);
    shader->setUniformValue("fog.maxDistance", 128.0f);

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
        m_panAngle = 0.0f;
    }

    if(!qFuzzyIsNull(m_tiltAngle))
    {
        m_camera->tilt(m_tiltAngle);
        m_tiltAngle = 0.0f;
    }

    // Update status bar
    QString sbMessage = QString("speed multiplier: %1").arg(speed_mult);
    /*QString sbMessage = "Initialized!";

    for(int i = 0; i < sbMessageList.count(); i++)
    {
        QVariant sbData = NULL;

        if(!sbDataList.isEmpty())
            sbData = sbDataList.takeFirst();

        qDebug() << sbData.toString();
        QString combine = QString(sbMessageList[i]).arg(sbData.toString());

        if(i == 0)
            sbMessage = combine;
        else
            sbMessage += ", " + combine;
    }*/

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

    // Render the quad as a patch
    {
        QOpenGLVertexArrayObject::Binder binder(&m_vao);
        shader->setPatchVertexCount(1);

        glDrawArrays(GL_PATCHES, 0, m_patchCount);
    }
}

void MapView::resizeGL(int w, int h)
{
    this->makeCurrent();

    // Make sure the viewport covers the entire window
    glViewport(0, 0, w, h);

    m_viewportSize = QVector2D(float(w), float(h));

    // Update the projection matrix
    float aspect = static_cast<float>(w) / static_cast<float>(h);
    m_camera->setPerspectiveProjection(25.0f, aspect, 0.1f, 128.0f);

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
    SamplerPtr sampler(new Sampler);
    sampler->create();
    sampler->setMinificationFilter(GL_LINEAR);
    sampler->setMagnificationFilter(GL_LINEAR);
    sampler->setWrapMode(Sampler::DirectionS, GL_CLAMP_TO_EDGE);
    sampler->setWrapMode(Sampler::DirectionT, GL_CLAMP_TO_EDGE);

    QImage heightMapImage("../terrain_tessellation/heightmap-1024x1024.png");
    m_funcs->glActiveTexture(GL_TEXTURE0);

    TexturePtr heightMap(new Texture);
    heightMap->create();
    heightMap->bind();
    heightMap->setImage(heightMapImage);

    m_heightMapSize = heightMapImage.size();
    m_material->setTextureUnitConfiguration(0, heightMap, sampler, QByteArrayLiteral("heightMap"));

    SamplerPtr tilingSampler(new Sampler);
    tilingSampler->create();
    tilingSampler->setMinificationFilter(GL_LINEAR_MIPMAP_LINEAR);
    m_funcs->glSamplerParameterf(tilingSampler->samplerId(), GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f);
    tilingSampler->setMagnificationFilter(GL_LINEAR);
    tilingSampler->setWrapMode(Sampler::DirectionS, GL_REPEAT);
    tilingSampler->setWrapMode(Sampler::DirectionT, GL_REPEAT);

    QImage grassImage("../terrain_tessellation/grass.png");
    m_funcs->glActiveTexture(GL_TEXTURE1);

    TexturePtr grassTexture(new Texture);
    grassTexture->create();
    grassTexture->bind();
    grassTexture->setImage(grassImage);
    grassTexture->generateMipMaps();
    m_material->setTextureUnitConfiguration(1, grassTexture, tilingSampler, QByteArrayLiteral("grassTexture"));

    QImage rockImage("../terrain_tessellation/rock.png");
    m_funcs->glActiveTexture(GL_TEXTURE2);

    TexturePtr rockTexture(new Texture);
    rockTexture->create();
    rockTexture->bind();
    rockTexture->setImage(rockImage);
    rockTexture->generateMipMaps();
    m_material->setTextureUnitConfiguration(2, rockTexture, tilingSampler, QByteArrayLiteral("rockTexture"));

    QImage snowImage("../terrain_tessellation/snowrocks.png");
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
    QVector<float> positionData(2 * m_patchCount); // 2 floats per vertex

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

        qDebug() << shader.data();

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

void MapView::AddStatusBarMessage(QString message)
{
    sbMessageList << message;
    sbDataList << NULL;
}

void MapView::AddStatusBarMessage(QString message, QVariant data)
{
    sbMessageList << message;
    sbDataList << data;
}

void MapView::ClearStatusBarMessage()
{
    sbMessageList.clear();
    sbDataList.clear();
}

void MapView::keyPressEvent(QKeyEvent* e)
{
    const float speed = 44.7f; // in m/s. Equivalent to 100 miles/hour

    switch (e->key())
    {
        case Qt::Key_Escape:
            exit(0);
            break;

        case Qt::Key_D:
            setSideSpeed(speed * speed_mult);
            break;

        case Qt::Key_A:
            setSideSpeed(-speed * speed_mult);
            break;

        case Qt::Key_W:
            setForwardSpeed(speed * speed_mult);
            break;

        case Qt::Key_S:
            setForwardSpeed(-speed * speed_mult);
            break;

        case Qt::Key_PageUp:
            setVerticalSpeed(speed * speed_mult);
            break;

        case Qt::Key_PageDown:
            setVerticalSpeed(-speed * speed_mult);
            break;

        case Qt::Key_Shift:
            setViewCenterFixed(true);
            break;

        case Qt::Key_Plus:
            //setTerrainHorizontalScale(terrainHorizontalScale() + 0.1);
            SpeedMultiplier(speed_mult + 0.1f);
            break;

        case Qt::Key_Minus:
            //setTerrainHorizontalScale(terrainHorizontalScale() - 0.1);
            SpeedMultiplier(speed_mult - 0.1f);
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

        case Qt::Key_F1:
            setDisplayMode(TexturedAndLit);
            break;

        case Qt::Key_F2:
            setDisplayMode(SimpleWireFrame);
            break;

        case Qt::Key_F3:
            setDisplayMode(WorldHeight);
            break;

        case Qt::Key_F4:
            setDisplayMode(WorldNormals);
            break;

        case Qt::Key_F5:
            setDisplayMode(Grass);
            break;

        case Qt::Key_F6:
            setDisplayMode(GrassAndRocks);
            break;

        case Qt::Key_F7:
            setDisplayMode(GrassRocksAndSnow);
            break;

        case Qt::Key_F8:
            setDisplayMode(LightingFactors);
            break;

        default:
            QGLWidget::keyPressEvent(e);
    }
}

void MapView::keyReleaseEvent(QKeyEvent* e)
{
    switch (e->key())
    {
        case Qt::Key_D:
        case Qt::Key_A:
            setSideSpeed(0.0f);
            break;

        case Qt::Key_W:
        case Qt::Key_S:
            setForwardSpeed(0.0f);
            break;

        case Qt::Key_PageUp:
        case Qt::Key_PageDown:
            setVerticalSpeed(0.0f);
            break;

        case Qt::Key_Shift:
            setViewCenterFixed(false);
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

    QGLWidget::mousePressEvent(e);
}

void MapView::mouseReleaseEvent(QMouseEvent* e)
{
    if(e->button() == Qt::LeftButton)
        m_leftButtonPressed = false;

    QGLWidget::mouseReleaseEvent(e);
}

void MapView::mouseMoveEvent(QMouseEvent* e)
{
    if(m_leftButtonPressed)
    {
        m_pos = e->pos();

        float dx = 0.2f * (m_pos.x() - m_prevPos.x());
        float dy = -0.2f * (m_pos.y() - m_prevPos.y());

        m_prevPos = m_pos;

        pan(dx);
        tilt(dy);
    }

    QGLWidget::mouseMoveEvent(e);
}
