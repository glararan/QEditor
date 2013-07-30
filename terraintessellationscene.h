#ifndef MAPVIEW_H
#define MAPVIEW_H

#include <QGLWidget>
#include <QWidget>

#include "material.h"

#include <QOpenGLBuffer>
#include <QOpenGLDebugLogger>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QMatrix4x4>
#include <QStringList>
#include <QTime>
#include <QLinkedList>

class Camera;

class QOpenGLFunctions_4_0_Core;

class MapView : public QGLWidget
{
    Q_OBJECT

public:
    MapView(QWidget* parent = 0);
    ~MapView();

    // Camera motion control
    void setSideSpeed(float vx)     { m_v.setX(vx); }
    void setVerticalSpeed(float vy) { m_v.setY(vy); }
    void setForwardSpeed(float vz)  { m_v.setZ(vz); }
    void setViewCenterFixed(bool b) { m_viewCenterFixed = b; }

    // Camera orientation control
    void pan(float angle)  { m_panAngle = angle; }
    void tilt(float angle) { m_tiltAngle = angle; }

    // Terrain scales
    void setTerrainHorizontalScale(float horizontalScale) { m_horizontalScale = horizontalScale; }
    float terrainHorizontalScale() const                  { return m_horizontalScale; }

    void setTerrainVerticalScale(float verticalScale)     { m_verticalScale = verticalScale; }
    float terrainVerticalScale() const                    { return m_verticalScale; }

    // Sun position
    void setSunAngle(float sunAngle) { m_sunTheta = sunAngle; qDebug() << m_sunTheta; }
    float sunAngle() const           { return m_sunTheta; }

    // Screen space error - tessellation control
    void setScreenSpaceError(float error) { m_screenSpaceError = error; qDebug() << error; }
    float screenSpaceError() const        { return m_screenSpaceError; }

    enum DisplayMode
    {
        SimpleWireFrame = 0,
        WorldHeight,
        WorldNormals,
        Grass,
        GrassAndRocks,
        GrassRocksAndSnow,
        LightingFactors,
        TexturedAndLit,
        DisplayModeCount
    };

    void setDisplayMode(DisplayMode displayMode) { m_displayMode = displayMode; }
    DisplayMode displayMode() const              { return m_displayMode; }

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

    void timerEvent(QTimerEvent*);

private:
    void prepareShaders();
    void prepareTextures();
    void prepareVertexBuffers(QSize heightMapSize);
    void prepareVertexArrayObject();

    void SpeedMultiplier(float multiplier);

    void AddStatusBarMessage(QString message);
    void AddStatusBarMessage(QString message, QVariant data);
    void ClearStatusBarMessage();

    QOpenGLContext* m_context;

    Camera* m_camera;

    QVector3D m_v;

    bool  m_viewCenterFixed;
    float m_panAngle;
    float m_tiltAngle;

    const float speed;
    float speed_mult;

    QMatrix4x4 m_viewportMatrix;
    QVector2D  m_viewportSize;

    // The terrain "object"
    QOpenGLVertexArrayObject m_vao;
    QOpenGLBuffer            m_patchBuffer;

    int   m_patchCount;
    float m_screenSpaceError;

    MaterialPtr m_material;

    // Terrain rendering controls
    QMatrix4x4 m_modelMatrix;

    float m_horizontalScale;
    float m_verticalScale;

    // Angle of sun. 0 is directly overhead, 90 to the East, -90 to the West
    float m_sunTheta;

    float m_time;
    const float m_metersToUnits;

    // time
    QTime m_Utime;

    // mouse
    bool m_leftButtonPressed;
    bool m_rightButtonPressed;

    QPoint m_prevPos;
    QPoint m_pos;

    // status bar
    QVector<QString>      sbMessageList;
    QLinkedList<QVariant> sbDataList;

    DisplayMode m_displayMode;
    QStringList m_displayModeNames;
    QVector<GLuint> m_displayModeSubroutines;

    QOpenGLFunctions_4_0_Core* m_funcs;
    QSize m_heightMapSize;

public slots:
    void setSpeedMultiplier(float value);

signals:
    void statusBar(QString message);
};

#endif // MAPVIEW_H