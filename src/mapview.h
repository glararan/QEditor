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

#ifndef MAPVIEW_H
#define MAPVIEW_H

#include <QOpenGLWidget>
#include <QWidget>

#include "world.h"

#include <QOpenGLDebugLogger>
#include <QMatrix4x4>
#include <QStringList>
#include <QTime>
#include <QColor>
#include <QTableWidgetItem>

class Camera;
class IPipeline;

class MapView : public QOpenGLWidget, protected QOpenGLFunctions_4_2_Core
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

    enum eMouseMode
    {
        Nothing      = 0,
        ShiftOnly    = 1,
        CtrlOnly     = 2,
        AltOnly      = 3,
        ShiftCtrl    = 4,
        ShiftAlt     = 5,
        CtrlAlt      = 6,
        ShiftCtrlAlt = 7
    };

    void setMouseMode(eMouseMode mouseMode) { eMMode = mouseMode; }
    eMouseMode mouseMode() const            { return eMMode; }

    enum eEditingMode
    {
        Default       = 0,
        Terrain       = 1,
        Texturing     = 2,
        VertexShading = 3,
        Object        = 5,
        CameraCurves  = 6
    };

    void setEditingMode(eEditingMode editingMode) { eMode = editingMode; }
    eEditingMode editingMode() const              { return eMode; }

    enum eTerrainMode
    {
        Shaping   = 0,
        Smoothing = 1,
        Uniform   = 2
    };

    void setTerrainMode(eTerrainMode terrainMode) { eTerrain = terrainMode; }
    eTerrainMode terrainMode() const              { return eTerrain; }

    enum eModelMode
    {
        Insertion = 0,
        Removal   = 1
    };

    void setModelMode(eModelMode modelMode) { eModel = modelMode; }
    eModelMode modelMode() const            { return eModel; }

    QVector3D getWorldCoordinates(float mouseX, float mouseY);

    IPipeline* getPipeline() const { return pipeline; }

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

    void tabletEvent(QTabletEvent* e);

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

    float uniform_height;

    /// Texturing parameters
    float texturing_flow;

    // Vertex shading parameters
    QColor vertexShadingColor;

    /// Global parameters
    QOpenGLContext* GLcontext;

    Camera* camera;

    QVector<QTableWidgetItem*> cameraCurvePoint;

    QVector3D m_v;

    bool viewCenterFixed;
    bool showCameraCurve;
    bool repeatCameraPlay;

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
    IPipeline* pipeline;

    bool stereoscopic;

    // time
    float time;

    const float m_metersToUnits;

    QTime m_Utime;

    int fps;

    // mouse
    bool leftButtonPressed, rightButtonPressed, wasLeftButtonPressed;
    bool changedMouseMode, tabletMode, tablet;

    QPoint mouse_position;

    QPoint prevMousePos;
    QPoint mousePos;

    int mousePosZ, prevMousePosZ;

    QVector3D terrain_pos;
    QVector3D object_move;

    QVector<float> dynamicZoom;

    struct Tablet
    {
        qreal pressure;
        qreal rotation;
        qreal tangentialPressure;

        int x, xTilt;
        int y, yTilt;

        QTabletEvent::TabletDevice device;
        QTabletEvent::PointerType  pointer;
    } wacom;

    // keyboard
    bool shiftDown, ctrlDown, altDown, escapeDown;

    // status bar
    QVector<QString>     sbMessageList;
    QVector<const void*> sbDataList;
    QVector<QString>     sbDataTypeList;

    eMouseMode   eMMode;
    eEditingMode eMode;
    eTerrainMode eTerrain;
    eModelMode   eModel;

public slots:
    void setSpeedMultiplier(float value);
    void setDisplayMode(int mode);
    void setModeEditing(int option);
    void setCameraPosition(QVector3D* position);
    void setCameraCurvePoint(QVector<QTableWidgetItem*>& item);
    void setCameraCurvePointPosition(QVector<QTableWidgetItem*>& item, const QVector3D& position);
    void setBrushSpeed(double speed);
    void setBrushOuterRadius(double radius);
    void setBrushInnerRadius(double radius);
    void setBrush(int brush);
    void setBrushType(int type);
    void setBrushMode(int mode);
    void setTexturingFlow(double flow);
    void setVertexShading(QColor color);
    void setCameraShowCurve(bool show);
    void setCameraRepeatPlay(bool repeat);
    void setTerrainMaximumHeight(double value);
    void setTerrainUniformHeight(double value);
    void setPaintMaximumAlpha(double value);
    void setTerrainMode(int mode);
    void setBrushColor(QColor* color, bool outer);
    void setWireframeColor(QColor* color, bool terrain);
    void setEnvionmentDistance(float value);
    void setTextureScaleOption_(int option);
    void setTextureScaleFar(float value);
    void setTextureScaleNear(float value);
    void setTabletMode(bool enable);
    void setTurnChunkLines(bool on);
    void setModelRotationX(double value);
    void setModelRotationY(double value);
    void setModelRotationZ(double value);
    void setModelScale(double value);
    void setModelImpend(double value);
    void set3DStreoscopic(bool enable);

    void resetCamera();
    void lockCamera(bool lock);

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

    void eMModeChanged(MapView::eMouseMode& mouseMode, MapView::eEditingMode& editingMode);

    void getCameraCurvePoint(const QVector3D& position);

    void initialized();
};

#endif // MAPVIEW_H
