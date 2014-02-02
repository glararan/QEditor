#ifndef MAPVIEW_H
#define MAPVIEW_H

#include <QGLWidget>
#include <QWidget>

#include "world.h"

#include <QOpenGLDebugLogger>
#include <QMatrix4x4>
#include <QStringList>
#include <QTime>
#include <QColor>

class Camera;

class MapView : public QGLWidget
{
    Q_OBJECT

public:
    MapView(World* mWorld, QWidget* parent = 0);
    ~MapView();

    // Camera motion control
    void setSideSpeed(float vx)     { m_v.setX(vx); }
    void setVerticalSpeed(float vy) { m_v.setY(vy); }
    void setForwardSpeed(float vz)  { m_v.setZ(vz); }
    void setFieldOfView(float zoom) { camera_zoom += zoom; }
    void setViewCenterFixed(bool b) { viewCenterFixed = b; }

    // Camera orientation control
    void pan(float angle)  { m_panAngle = angle; }
    void tilt(float angle) { m_tiltAngle = angle; }

    // Screen space error - tessellation control
    void setScreenSpaceError(float error) { screenSpaceErrorLevel = error; qDebug() << error; }
    float screenSpaceError() const        { return screenSpaceErrorLevel; }

    enum eEditingMode
    {
<<<<<<< HEAD:src/mapview.h
        Objects       = 0,
        Terrain       = 1,
        Texturing     = 2,
        VertexShading = 3
=======
        Default       = 0,
        Terrain       = 1,
        Texturing     = 2,
        VertexShading = 3,
        Object        = 5
>>>>>>> origin/netix:src/mapview.h
    };

    void setEditingMode(eEditingMode editingMode) { eMode = editingMode; }
    eEditingMode editingMode() const              { return eMode; }

    enum eTerrainMode
    {
        Shaping   = 0,
        Smoothing = 1
    };

    void setTerrainMode(eTerrainMode terrainMode) { eTerrain = terrainMode; }
    eTerrainMode terrainMode() const              { return eTerrain; }

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
    void SpeedMultiplier(float multiplier);

    void AddStatusBarMessage(const QString message);
    void AddStatusBarMessage(const QString message, const void* data, const QString data_type);
    void ClearStatusBarMessage();

    World* world;

    /// Terrain parameters
    float shaping_speed;
    int   shaping_brush_type;

    /// Texturing parameters
    float texturing_flow;

    // Vertex shading parameters
    QColor vertexShadingColor;

    /// Global parameters
    QOpenGLContext* GLcontext;

    Camera* camera;

    QVector3D m_v;

    bool  viewCenterFixed;
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

    // Terrain rendering controls
    QMatrix4x4 modelMatrix;

    // time
    float time;

    const float m_metersToUnits;

    QTime m_Utime;

    int fps;

    // mouse
    bool leftButtonPressed, rightButtonPressed, wasLeftButtonPressed;

    QPoint mouse_position;

    QPoint prevMousePos;
    QPoint mousePos;

    QVector3D terrain_pos;

    QVector<float> dynamicZoom;

    // keyboard
    bool shiftDown, ctrlDown, altDown, escapeDown;

    // status bar
    QVector<QString>     sbMessageList;
    QVector<const void*> sbDataList;
    QVector<QString>     sbDataTypeList;

    eEditingMode eMode;
    eTerrainMode eTerrain;

public slots:
    void setSpeedMultiplier(float value);
    void setDisplayMode(int mode);
    void setModeEditing(int option);
    void setCameraPosition(QVector3D* position);
    void setBrushSpeed(double speed);
    void setBrushOuterRadius(double radius);
    void setBrushInnerRadius(double radius);
    void setBrush(int brush);
    void setBrushType(int type);
    void setTexturingFlow(double flow);
    void setVertexShading(QColor color);
    void setTerrainMaximumHeight(double value);
    void setTerrainMode(int mode);
    void setBrushColor(QColor* color, bool outer);
    void setEnvionmentDistance(float value);
    void setTextureScaleOption_(int option);
    void setTextureScaleFar(float value);
    void setTextureScaleNear(float value);
    void setTurnChunkLines(bool on);
    void setModelRotationX(double value);
    void setModelRotationY(double value);
    void setModelRotationZ(double value);
    void setModelScale(double value);
    void setModelImpend(double value);

    void resetCamera();

    void save();

    void doTest();

signals:
    void statusBar(QString message);

    void updateBrushSpeed(double speed);
    void updateBrushOuterRadius(double radius);
    void updateBrushInnerRadius(double radius);
    void updateBrush(int brush);

    void selectedMapChunk(MapChunk* chunk);
    void selectedWaterChunk(WaterChunk* chunk);

    void initialized();
};

#endif // MAPVIEW_H
