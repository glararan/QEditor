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

#include "mapview.h"
#include "camera.h"
#include "mathhelper.h"
#include "mapobject.h"
#include "mapchunk.h"
#include "qeditor.h"

#include "pipeline.h"

#include <QKeyEvent>
#include <QTabletEvent>
#include <QOpenGLContext>

MapView::MapView(World* mWorld, UndoRedoManager* undoManager, QWidget* parent, bool** mapCoords)
: QOpenGLWidget(parent)
, world(mWorld)
, worldCoords(mapCoords) // coordinats for cache tiles
, GLcontext(this->context())
, camera(new Camera(this))
, pipeline(new Pipeline())
, m_v()
, viewCenterFixed(false)
, showCameraCurve(false)
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
, screenSpaceErrorLevel(12.0f)
, modelMatrix()
, stereoscopic(false)
, time(0.0f)
, m_metersToUnits(4 * MathHelper::PI() / speed) // 500 units == 10 km => 0.05 units/m
, fps(0)
, frames(0)
, leftButtonPressed(false)
, rightButtonPressed(false)
, wheelButtonPressed(false)
, wasLeftButtonPressed(false)
, wasLeftButtonReleased(false)
, wasWheelButtonPressed(false)
, wasWheelButtonReleased(false)
, changedMouseMode(false)
, tabletMode(app().getSetting("tabletMode", false).toBool())
, tablet(false)
, mouse_position(QPoint(0, 0))
, prevMousePos(QPoint(0, 0))
, mousePos(QPoint(0, 0))
, mousePosStart(QPoint(0, 0))
, mousePosEnd(QPoint(0, 0))
, mouseVector(QVector2D())
, mousePosZ(0)
, prevMousePosZ(0)
, terrain_pos()
, object_move()
, shaping_speed(1.0f)
, shaping_brush_type(1)
, uniform_height(0.0f)
, texturing_flow(1.0f)
, vertexShadingColor(QColor(255, 255, 255, 255))
, spawn_on_click(false)
, drawModel(false)
, shiftDown(false)
, ctrlDown(false)
, altDown(false)
, escapeDown(false)
, deleteDown(false)
, undoRedoManager(undoManager)
, mapObjectStart(QVector3D())
, mapObjectScale(false)
, eMMode(Nothing)
, eMode(Default)
, eTerrain(Shaping)
, eModel(Insertion)
{
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);

    modelMatrix.setToIdentity();
    //pipeline->getModelMatrix().setToIdentity();

    // Initialize the camera position and orientation
    camera->setPosition(QVector3D(250.0f, 10.0f, 250.0f));
    camera->setViewCenter(QVector3D(250.0f, 10.0f, 249.0f));
    camera->setUpVector(QVector3D(0.0f, 1.0f, 0.0f));

    // Assign camera to World
    world->setCamera(camera);

    // tablet initial
    wacom.pressure = 1.0f;

    AddStatusBarMessage("FPS: "             , &fps           , "int");
    AddStatusBarMessage("speed multiplier: ", &speed_mult    , "float");
    AddStatusBarMessage("camera: "          , &camera->pos() , "QVector3D_xzy");
    AddStatusBarMessage("zoom: "            , &camera_zoom   , "float");
    AddStatusBarMessage("pan: "             , &panAngle      , "float");
    AddStatusBarMessage("tilt: "            , &tiltAngle     , "float");
    AddStatusBarMessage("mouse: "           , &mouse_position, "QPoint");
    AddStatusBarMessage("terrain coords: "  , &terrain_pos   , "QVector3D_xzy");

    //setUpdateBehavior(QOpenGLWidget::PartialUpdate);

    startTimer(16);
}

MapView::~MapView()
{
    delete camera;

    if(worldCoords)
    {
        for(int i = 0; i < TILES; ++i)
            delete[] worldCoords[i];

        delete[] worldCoords;
    }
}

void MapView::timerEvent(QTimerEvent*)
{
    this->makeCurrent();

    const qreal time = m_Utime.elapsed() / 1000.0f;

    update(time);
    paintGL();
}

void MapView::initializeGL()
{
    this->makeCurrent();

    initializeOpenGLFunctions();

    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    format.setMajorVersion(4);
    format.setMinorVersion(3);
    format.setSamples(4);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setOption(QSurfaceFormat::DebugContext);

    this->context()->setFormat(format);

    // Initialize World
    world->initialize(this->context(), size(), worldCoords);

    // Initialize camera shader
    camera->initialize();

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    glDepthFunc(GL_LEQUAL); // just testing new depth func

    glClearColor(0.65f, 0.77f, 1.0f, 1.0f);

    m_Utime.start();
    fpsTimer.start();

    emit initialized();
}

void MapView::update(float t)
{
    modelMatrix.setToIdentity();
    //pipeline->getModelMatrix().setToIdentity();

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
    /*if(dynamicZoom.count() > 0)
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
    }*/

    // Update the camera perspective projection if camera zoom is changed
    if(camera_zoom != camera->fieldOfView())
        camera->setPerspectiveProjection(camera_zoom, aspectRatio, nearPlane, farPlane);

    // Move camera when playing
    if(camera->playing())
        camera->playSequence();

    // Update the mouse mode in MainWindow
    if(!leftButtonPressed && !rightButtonPressed && (shiftDown || altDown || ctrlDown) && tabletMode)
    {
        if(!changedMouseMode)
        {
            eMouseMode lastMMode = eMMode;

            if(shiftDown && !altDown && !ctrlDown)
                eMMode = ShiftOnly;
            else if(ctrlDown && !shiftDown && !altDown)
                eMMode = CtrlOnly;
            else if(altDown && !shiftDown && !ctrlDown)
                eMMode = AltOnly;
            else if(shiftDown && ctrlDown && !altDown)
                eMMode = ShiftCtrl;
            else if(shiftDown && altDown && !ctrlDown)
                eMMode = ShiftAlt;
            else if(ctrlDown && altDown && !shiftDown)
                eMMode = CtrlAlt;
            else if(shiftDown && altDown && ctrlDown)
                eMMode = ShiftCtrlAlt;

            // if current is last set to nothing
            if(lastMMode == eMMode)
                eMMode = Nothing;

            emit eMModeChanged(eMMode, eMode);

            changedMouseMode = true;
        }
    }
    else
        changedMouseMode = false;

    // highlight and select chunk
    if(eMode == Default)
    {
        const QVector3D& position(terrain_pos);

        if(altDown && world->getCurrentSelection()->getType() == Selection::None)
        {
            world->highlightMapChunkAt(position);

            if(leftButtonPressed)
            {
                emit selectedMapTile(world->getTileAt(position.x(), position.z()));
                emit selectedMapChunk(world->getMapChunkAt(position));
                emit selectedWaterChunk(world->getWaterChunkAt(position));
            }
        }
        else if(wasLeftButtonPressed && !spawn_on_click && world->getCurrentSelection()->getType() == Selection::MapObjectType && world->getCurrentSelection()->getData().mapObject)
        {
            //if(world->trySelectMapObject(position))
            //{
                if(!ctrlDown && !altDown)
                {
                    mapObjectStart = world->getCurrentSelection()->getData().mapObject->getRotation();

                    mapObjectScale = false;
                }
                else if(ctrlDown && altDown)
                {
                    mapObjectStart = world->getCurrentSelection()->getData().mapObject->getScale();

                    mapObjectScale = true;
                }
            //}
        }
        else if(wasLeftButtonPressed && spawn_on_click)
        {
            spawn_on_click = drawModel = false;

            world->spawnMapObjectFromFavouriteList(terrain_pos);
        }
        else if(!wasLeftButtonPressed && (spawn_on_click || drawModel) && escapeDown)
        {
            world->getModelManager()->setCurrentModel(-1);

            escapeDown = spawn_on_click = drawModel = false;
        }
        else if(deleteDown && world->getCurrentSelection()->getType() == Selection::MapObjectType)
        {
            undoRedoManager->push(new DeleteMapObjectCommand(world->getCurrentSelection()->getData().mapObject, world));

            //world->removeObject(world->getCurrentSelection()->getData().mapObject);

            deleteDown = false;
        }
        else if(wheelButtonPressed && world->getCurrentSelection()->getType() == Selection::MapObjectType && object_move != QVector3D())
        {
            QVector3D directionUp(1.0f, 0.0f, 0.0f);
            QVector3D directionRight(0.0f, 0.0f, 1.0f);

            if(wasWheelButtonPressed)
                mapObjectStart = world->getCurrentSelection()->getData().mapObject->getTranslate();

            if(!shiftDown && !altDown && !ctrlDown) // xyz move
            {
                directionUp    = Rotate(0.0f, 0.0f, directionUp,    panAngle * MathHelper::toFloat(MathHelper::PI()) / 180.0f);
                directionRight = Rotate(0.0f, 0.0f, directionRight, panAngle * MathHelper::toFloat(MathHelper::PI()) / 180.0f);

                QVector3D translation = world->getCurrentSelection()->getData().mapObject->getTranslate();

                translation += mouseVector.x() * directionUp    * -80.0f;
                translation -= mouseVector.y() * directionRight * 80.0f;

                world->getCurrentSelection()->getData().mapObject->setTranslate(translation, true);
            }
            else if(shiftDown) // x
            {
                QVector3D translation = world->getCurrentSelection()->getData().mapObject->getTranslate() + QVector3D(object_move.x(), 0.0f, 0.0f);

                world->getCurrentSelection()->getData().mapObject->setTranslate(translation, true);
            }
            else if(ctrlDown) // y
            {
                QVector3D translation = world->getCurrentSelection()->getData().mapObject->getTranslate() + QVector3D(0.0f, object_move.y(), 0.0f);

                world->getCurrentSelection()->getData().mapObject->setTranslate(translation, true);
            }
            else if(altDown) // z
            {
                QVector3D translation = world->getCurrentSelection()->getData().mapObject->getTranslate() + QVector3D(0.0f, 0.0f, object_move.x());

                world->getCurrentSelection()->getData().mapObject->setTranslate(translation, true);
            }
        }
        else if(wasWheelButtonReleased && world->getCurrentSelection()->getType() == Selection::MapObjectType && mapObjectStart != world->getCurrentSelection()->getData().mapObject->getTranslate() && mapObjectStart != QVector3D())
        {
            undoRedoManager->push(new MoveMapObjectCommand(world->getCurrentSelection()->getData().mapObject, mapObjectStart));

            mapObjectStart = QVector3D();
        }
        else if(leftButtonPressed && world->getCurrentSelection()->getType() == Selection::MapObjectType)
        {
            // rotation and scale is based on mouseX move
            if(shiftDown) // rotateY
            {
                QVector3D rotation = world->getCurrentSelection()->getData().mapObject->getRotation() + QVector3D(0.0f, object_move.x(), 0.0f);

                world->getCurrentSelection()->getData().mapObject->setRotation(rotation, true);
            }
            else if(ctrlDown && !altDown) // rotateX
            {
                QVector3D rotation = world->getCurrentSelection()->getData().mapObject->getRotation() + QVector3D(object_move.x(), 0.0f, 0.0f);

                world->getCurrentSelection()->getData().mapObject->setRotation(rotation, true);
            }
            else if(altDown && !ctrlDown) // rotateZ
            {
                QVector3D rotation = world->getCurrentSelection()->getData().mapObject->getRotation() + QVector3D(0.0f, 0.0f, object_move.x());

                world->getCurrentSelection()->getData().mapObject->setRotation(rotation, true);
            }
            else if(ctrlDown && altDown) // scale
            {
                QVector3D scale = world->getCurrentSelection()->getData().mapObject->getScale() + QVector3D(object_move.x(), object_move.x(), object_move.x());

                world->getCurrentSelection()->getData().mapObject->setScale(scale, true);
            }
        }
        else if(wasLeftButtonReleased && world->getCurrentSelection()->getType() == Selection::MapObjectType && mapObjectStart != world->getCurrentSelection()->getData().mapObject->getRotation() && !mapObjectScale)
        {
            undoRedoManager->push(new RotateMapObjectCommand(world->getCurrentSelection()->getData().mapObject, mapObjectStart));

            mapObjectStart = QVector3D();
        }
        else if(wasLeftButtonReleased && world->getCurrentSelection()->getType() == Selection::MapObjectType && mapObjectStart != world->getCurrentSelection()->getData().mapObject->getScale() && mapObjectScale)
        {
            undoRedoManager->push(new ScaleMapObjectCommand(world->getCurrentSelection()->getData().mapObject, mapObjectStart));

            mapObjectStart = QVector3D();
        }
    }

    // objects
    if(eMode == Object)
    {
        switch(eModel)
        {
            case Insertion:
                {
                    world->updateNewModel(shiftDown, wasLeftButtonPressed);

                    if(escapeDown)
                    {
                        world->getModelManager()->setCurrentModel(-1);

                        escapeDown = false;
                    }
                }
                break;

            case Removal:
                {
                    if(leftButtonPressed && eMMode == ShiftOnly)
                    {
                        const QVector3D& position(terrain_pos);

                        world->removeObject(position.x(), position.z());
                    }
                }
                break;
        }
    }

    // Change terrain
    if(leftButtonPressed || tablet)
    {
        const QVector3D& position(terrain_pos);

        if((eMMode == ShiftOnly && tabletMode) || (eMMode == ShiftOnly && tablet) || shiftDown)
        {
            switch(eMode)
            {
                case Terrain:
                    {
                        switch(eTerrain)
                        {
                            case Shaping:
                                world->changeTerrain(position.x(), position.z(), 7.5f * dt * shaping_speed * wacom.pressure);
                                break;

                            case Smoothing:
                                world->flattenTerrain(position.x(), position.z(), position.y(), pow(0.2f, dt) * shaping_speed * wacom.pressure);
                                break;

                            case Uniform:
                                world->uniformTerrain(position.x(), position.z(), uniform_height);
                                break;
                        }
                    }
                    break;

                case Texturing:
                    {
                        world->paintTerrain(position.x(), position.z(), qMin(texturing_flow * dt, 1.0f) * wacom.pressure);
                    }
                    break;

                case VertexShading:
                    {
                        world->paintVertexShading(position.x(), position.z(), qMin(texturing_flow * dt, 1.0f) * wacom.pressure, vertexShadingColor);
                    }
                    break;

                case CameraCurves:
                    {
                        if(cameraCurvePoint.count() == 0) // select point
                            emit getCameraCurvePoint(position);
                        else // move with selected point
                            setCameraCurvePointPosition(cameraCurvePoint, object_move);
                    }
                    break;

                case VertexLighting:
                    {
                        world->paintVertexLighting(position.x(), position.z(), qMin(texturing_flow * dt, 1.0f) * wacom.pressure, vertexShadingColor);
                    }
                    break;
            }
        }
        else if((eMMode == CtrlOnly && tabletMode) || (eMMode == CtrlOnly && tablet) || ctrlDown)
        {
            switch(eMode)
            {
                case Terrain:
                    {
                        switch(eTerrain)
                        {
                            case Shaping:
                                world->changeTerrain(position.x(), position.z(), -7.5f * dt * shaping_speed * wacom.pressure);
                                break;

                            case Smoothing:
                                world->blurTerrain(position.x(), position.z(), pow(0.2f, dt) * shaping_speed * wacom.pressure);
                                break;

                            case Uniform:
                                world->uniformTerrain(position.x(), position.z(), uniform_height);
                                break;
                        }
                    }
                    break;

                case Texturing:
                    {
                        world->paintTerrain(position.x(), position.z(), qMin(texturing_flow * dt, 1.0f) * wacom.pressure);
                    }
                    break;

                case VertexShading:
                    {
                        world->paintVertexShading(position.x(), position.z(), qMin(texturing_flow * dt, 1.0f) * wacom.pressure, vertexShadingColor);
                    }
                    break;

                case VertexLighting:
                    {
                        world->paintVertexLighting(position.x(), position.z(), qMin(texturing_flow * dt, 1.0f) * wacom.pressure, vertexShadingColor);
                    }
                    break;
            }
        }
    }

    if(wasLeftButtonReleased)
    {
        switch(eMode)
        {
            case Default:
                {
                    if(!spawn_on_click && mousePosStart == mousePosEnd)
                    {
                        if(world->trySelectMapObject(terrain_pos))
                        {
                            if(!ctrlDown && !altDown)
                            {
                                mapObjectStart = world->getCurrentSelection()->getData().mapObject->getRotation();

                                mapObjectScale = false;
                            }
                            else if(ctrlDown && altDown)
                            {
                                mapObjectStart = world->getCurrentSelection()->getData().mapObject->getScale();

                                mapObjectScale = true;
                            }
                        }
                    }
                }
                break;

            case Terrain:
                {
                    if(world->getModifiedTerrain().count() > 0)
                    {
                        QHash<QVector2D, TerrainUndoData> modTerrain = world->getModifiedTerrain();

                        QVector<QPair<QVector2D, TerrainUndoData>> modifiedTerrain, modifiedTerrain2;

                        QPair<QVector2D, TerrainUndoData> pair, pair2;

                        QVector<QVector2D> keys         = modTerrain.keys().toVector();
                        QVector<TerrainUndoData> values = modTerrain.values().toVector();

                        int kCount = keys.count();

                        TerrainUndoData value;

                        for(int i = 0; i < kCount; ++i) // lost 99% of time on world->....
                        {
                            pair.first  = keys.at(i);
                            pair.second = values.at(i);

                            modifiedTerrain.append(pair);

                            value.index    = pair.second.index;
                            value.value    = world->getMapChunkAt(QVector3D(pair.first.x(), 0.0f, pair.first.y()))->getMapData(value.index);
                            value.position = pair.second.position;

                            pair2.first  = pair.first;
                            pair2.second = value;

                            modifiedTerrain2.append(pair2);
                        }

                        undoRedoManager->push(new ModifyTerrainCommand(modifiedTerrain, modifiedTerrain2, world)); // 800ms

                        world->clearModifiedTerrain();
                    }
                }
                break;

            case Texturing:
                {
                    QHash<QVector2D, TextureUndoData> modTextures = world->getModifiedTextures();

                    QVector<QPair<QVector2D, TextureUndoData>> modifiedTextures, modifiedTextures2;

                    QPair<QVector2D, TextureUndoData> pair, pair2;

                    QVector<QVector2D> keys         = modTextures.keys().toVector();
                    QVector<TextureUndoData> values = modTextures.values().toVector();

                    int kCount = keys.count();

                    QVector<float> value;

                    TextureUndoData val;

                    for(int i = 0; i < kCount; ++i) // lost 99% of time on world->....
                    {
                        pair.first  = keys.at(i);
                        pair.second = values.at(i);

                        modifiedTextures.append(pair);

                        for(int j = 1; j < MAX_TEXTURES; ++j) // base texture dont own alpha layer, so we start at j = 1
                            value.append(world->getMapChunkAt(QVector3D(pair.first.x(), 0.0f, pair.first.y()))->getAlphaMapsData(pair.second.index, j));

                        val.index    = pair.second.index;
                        val.position = pair.second.position;
                        val.values   = value;

                        pair2.first  = pair.first;
                        pair2.second = val;

                        modifiedTextures2.append(pair2);

                        value.clear();
                    }

                    undoRedoManager->push(new ModifyTexturesCommand(modifiedTextures, modifiedTextures2, world)); // 800ms

                    world->clearModifiedTextures();
                }
                break;

            case VertexShading:
            case VertexLighting:
                {
                    QHash<QVector2D, VertexUndoData> modVertexs = world->getModifiedVertexs();

                    QVector<QPair<QVector2D, VertexUndoData>> modifiedVertexs, modifiedVertexs2;

                    QPair<QVector2D, VertexUndoData> pair, pair2;

                    QVector<QVector2D> keys        = modVertexs.keys().toVector();
                    QVector<VertexUndoData> values = modVertexs.values().toVector();

                    int kCount = keys.count();

                    QColor value;

                    VertexUndoData val;

                    for(int i = 0; i < kCount; ++i) // lost 99% of time on world->....
                    {
                        pair.first  = keys.at(i);
                        pair.second = values.at(i);

                        modifiedVertexs.append(pair);

                        if(eMode == VertexShading)
                            value = world->getMapChunkAt(QVector3D(pair.first.x(), 0.0f, pair.first.y()))->getVertexShadingData(pair.second.index);
                        else if(eMode == VertexLighting)
                            value = world->getMapChunkAt(QVector3D(pair.first.x(), 0.0f, pair.first.y()))->getVertexLightingData(pair.second.index);

                        val.index    = pair.second.index;
                        val.position = pair.second.position;
                        val.r        = MathHelper::toUChar(value.red());
                        val.g        = MathHelper::toUChar(value.green());
                        val.b        = MathHelper::toUChar(value.blue());
                        val.a        = MathHelper::toUChar(value.alpha());

                        pair2.first  = pair.first;
                        pair2.second = val;

                        modifiedVertexs2.append(pair2);
                    }

                    undoRedoManager->push(new ModifyVertexsCommand(modifiedVertexs, modifiedVertexs2, world, eMode == VertexShading ? true : false)); // 800ms

                    world->clearModifiedTextures();
                }
                break;
        }
    }

    if(object_move != QVector3D())
        object_move = QVector3D();

    wasLeftButtonPressed = wasLeftButtonReleased = wasWheelButtonPressed = wasWheelButtonReleased = false;

    world->update(t);

    // Update status bar
    QString sbMessage = tr("Initialized!");

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
#ifdef Q_OS_WIN32
                    else if(sbDataTypeList[i] == "byte")
                        sbData = *(byte*)sbDataList.at(i);
#endif
                    else if(sbDataTypeList[i] == "char")
                        sbData = *(char*)sbDataList.at(i);
                    else if(sbDataTypeList[i] == "int")
                        sbData = *(int*)sbDataList.at(i);
#ifdef Q_OS_WIN32
                    else if(sbDataTypeList[i] == "long")
                        sbData = *(long*)sbDataList.at(i);
#elif defined(Q_OS_LINUX)
                    else if(sbDataTypeList[i] == "long")
                        sbData = *(qlonglong*)sbDataList.at(i);
#endif
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

    if(!stereoscopic)
    {
        glDrawBuffer(GL_FRONT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if(showCameraCurve)
            camera->drawCurve(modelMatrix);

        if(eMode == Terrain || eMode == Texturing || eMode == VertexShading || eMode == VertexLighting)
            world->draw(this, terrain_pos, modelMatrix, screenSpaceErrorLevel, QVector2D(mouse_position.x(), mouse_position.y()), true);
        else if(eMode == Object && eModel == Insertion)
            world->draw(this, terrain_pos, modelMatrix, screenSpaceErrorLevel, QVector2D(mouse_position.x(), mouse_position.y()), false, true);
        else
            world->draw(this, terrain_pos, modelMatrix, screenSpaceErrorLevel, QVector2D(mouse_position.x(), mouse_position.y()), false, drawModel);
    }
    else
    {
        glDrawBuffer(GL_BACK_LEFT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glFrustum(0.0 - 1.0, width() - 1.0, -0.75, 0.75, 0.65, 4.0);

        if(showCameraCurve)
            camera->drawCurve(modelMatrix);

        if(eMode == Terrain || eMode == Texturing || eMode == VertexShading || eMode == VertexLighting)
            world->draw(this, terrain_pos, modelMatrix, screenSpaceErrorLevel, QVector2D(mouse_position.x(), mouse_position.y()), true);
        else if(eMode == Object && eModel == Insertion)
            world->draw(this, terrain_pos, modelMatrix, screenSpaceErrorLevel, QVector2D(mouse_position.x(), mouse_position.y()), false, true);
        else
            world->draw(this, terrain_pos, modelMatrix, screenSpaceErrorLevel, QVector2D(mouse_position.x(), mouse_position.y()), false, drawModel);

        glDrawBuffer(GL_BACK_RIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glFrustum(0.0 + 1.0, width() + 1.0, -0.75, 0.75, 0.65, 4.0);

        if(showCameraCurve)
            camera->drawCurve(modelMatrix);

        if(eMode == Terrain || eMode == Texturing || eMode == VertexShading || eMode == VertexLighting)
            world->draw(this, terrain_pos, modelMatrix, screenSpaceErrorLevel, QVector2D(mouse_position.x(), mouse_position.y()), true);
        else if(eMode == Object && eModel == Insertion)
            world->draw(this, terrain_pos, modelMatrix, screenSpaceErrorLevel, QVector2D(mouse_position.x(), mouse_position.y()), false, true);
        else
            world->draw(this, terrain_pos, modelMatrix, screenSpaceErrorLevel, QVector2D(mouse_position.x(), mouse_position.y()), false, drawModel);
    }
}

void MapView::resizeGL(int w, int h)
{
    this->makeCurrent();

    // Make sure the viewport covers the entire window
    glViewport(0, 0, w, h);

    viewportSize = QVector2D(float(w), float(h));

    // Update the projection matrix
    aspectRatio = MathHelper::toFloat(w) / MathHelper::toFloat(h);

    camera->setPerspectiveProjection(camera_zoom, aspectRatio, nearPlane, farPlane);

    // Update World FBO
    world->setFboSize(QSize(w, h));

    // Update the viewport matrix
    float w2 = w / 2.0f;
    float h2 = h / 2.0f;

    viewportMatrix.setToIdentity();
    viewportMatrix.setColumn(0, QVector4D(w2, 0.0f, 0.0f, 0.0f));
    viewportMatrix.setColumn(1, QVector4D(0.0f, h2, 0.0f, 0.0f));
    viewportMatrix.setColumn(2, QVector4D(0.0f, 0.0f, 1.0f, 0.0f));
    viewportMatrix.setColumn(3, QVector4D(w2, h2, 0.0f, 1.0f));

    QOpenGLShaderProgram* shader = world->getTerrainShader();
    shader->bind();

    // We need the viewport size to calculate tessellation levels and the geometry shader also needs the viewport matrix
    shader->setUniformValue("viewportSize",   viewportSize);
    shader->setUniformValue("viewportMatrix", viewportMatrix);
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

QVector3D MapView::Rotate(float x, float y, QVector3D& direction, float angle)
{
    float xa = direction.x() - x;
    float ya = direction.z() - y;

    direction.setX(xa * cosf(angle) - ya * sinf(angle) + x);
    direction.setZ(xa * sinf(angle) + ya * cosf(angle) + y);

    return direction;
}

void MapView::setModeEditing(int option)
{
    switch(option)
    {
        case Default:
        case Terrain:
        case Texturing:
        case VertexShading:
        case Object:
        case CameraCurves:
        case VertexLighting:
            eMode = (eEditingMode)option;
            break;

        default:
            eMode = Default;
            break;
    }

    drawModel = false;
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

void MapView::resetCamera()
{
    camera->resetRotation();

    panAngle  = 0.0f;
    tiltAngle = 0.0f;
}

void MapView::lockCamera(bool lock)
{
    camera->setLock(lock);
}

void MapView::setCameraCurvePoint(QVector<QTableWidgetItem*>& item)
{
    cameraCurvePoint = item;
}

void MapView::setCameraCurvePointPosition(QVector<QTableWidgetItem*>& item, const QVector3D& position)
{
    item.at(0)->setData(Qt::DisplayRole, item.at(0)->data(Qt::DisplayRole).toFloat() + position.x()); // X
    item.at(1)->setData(Qt::DisplayRole, item.at(1)->data(Qt::DisplayRole).toFloat() + position.z()); // Y
    item.at(2)->setData(Qt::DisplayRole, item.at(2)->data(Qt::DisplayRole).toFloat() + position.y()); // Z
}

void MapView::setBrushSpeed(double speed)
{
    shaping_speed = MathHelper::toFloat(speed);
}

void MapView::setBrushOuterRadius(double radius)
{
    world->getBrush()->setOuterRadius(MathHelper::toFloat(radius));

    // Todo inner radius
    //world->getBrush()->setRadius(MathHelper::toFloat(radius));
}

void MapView::setBrushInnerRadius(double radius)
{
    world->getBrush()->setInnerRadius(MathHelper::toFloat(radius));
}

void MapView::setBrush(int brush)
{
    world->getBrush()->BrushTypes().setShaping((Brush::ShapingType::Formula)brush);
}

void MapView::setBrushType(int type)
{
    Brush::Types types = world->getBrush()->BrushTypes();

    switch(eTerrain)
    {
        case Shaping:
        default:
            types.setShaping(static_cast<Brush::ShapingType::Formula>(type));
            break;

        case Smoothing:
            types.setSmoothing(static_cast<Brush::SmoothingType::Formula>(type));
            break;
    }

    world->getBrush()->setBrush(types);
}

void MapView::setBrushMode(int mode)
{
    eMMode = (eMouseMode)mode;
}

void MapView::setTexturingFlow(double flow)
{
    texturing_flow = flow;
}

void MapView::setVertexShading(QColor color)
{
    vertexShadingColor = color;
}

void MapView::setCameraShowCurve(bool show)
{
    showCameraCurve = show;
}

void MapView::setCameraRepeatPlay(bool repeat)
{
    camera->setPlayRepeat(repeat);
}

void MapView::setTerrainMaximumHeight(double value)
{
    world->setTerrainMaximumHeight(MathHelper::toFloat(value));
}

void MapView::setTerrainUniformHeight(double value)
{
    uniform_height = MathHelper::toFloat(value);
}

void MapView::setPaintMaximumAlpha(double value)
{
    world->setPaintMaximumAlpha(MathHelper::toFloat(value));
}

void MapView::setTerrainMode(int mode)
{
    eTerrain = (eTerrainMode)mode;
}

void MapView::setBrushColor(QColor* color, bool outer)
{
    switch(outer)
    {
        case true:
            {
                world->getBrush()->setOuterColor(*color);

                app().setSetting("outerBrushColor", *color);
            }
            break;

        case false:
            {
                world->getBrush()->setInnerColor(*color);

                app().setSetting("innerBrushColor", *color);
            }
            break;
    }
}

void MapView::setWireframeColor(QColor* color, bool terrain)
{
    QOpenGLShaderProgram* shader = world->getTerrainShader();
    shader->bind();

    switch(terrain)
    {
        case true:
            {
                shader->setUniformValue("line.color2", MathHelper::toVector4D(*color));

                app().setSetting("terrainWireframe", MathHelper::toVector4D(*color));
            }
            break;

        case false:
            {
                shader->setUniformValue("line.color", MathHelper::toVector4D(*color));

                app().setSetting("wireframe", MathHelper::toVector4D(*color));
            }
            break;
    }
}

void MapView::setEnvionmentDistance(float value)
{
    farPlane = value;

    app().setSetting("environmentDistance", farPlane);

    QOpenGLShaderProgram* shader = world->getTerrainShader();
    shader->bind();

    shader->setUniformValue("fog.minDistance", farPlane / 2);
    shader->setUniformValue("fog.maxDistance", farPlane - 32.0f);

    camera->setPerspectiveProjection(camera_zoom, aspectRatio, nearPlane, farPlane);
}

void MapView::setTextureScaleOption_(int option)
{
    app().setSetting("textureScaleOption", option);

    QOpenGLShaderProgram* shader = world->getTerrainShader();
    shader->bind();
    shader->setUniformValue("textureScaleOption", option);
}

void MapView::setTextureScaleFar(float value)
{
    app().setSetting("textureScaleFar", value);

    QOpenGLShaderProgram* shader = world->getTerrainShader();
    shader->bind();
    shader->setUniformValue("textureScaleFar", value);
}

void MapView::setTextureScaleNear(float value)
{
    app().setSetting("textureScaleNear", value);

    QOpenGLShaderProgram* shader = world->getTerrainShader();
    shader->bind();
    shader->setUniformValue("textureScaleNear", value);
}

void MapView::setTabletMode(bool enable)
{
    app().setSetting("tabletMode", enable);

    tabletMode = enable;
}

void MapView::setTurnChunkLines(bool on)
{
    app().setSetting("chunkLines", on);

    QOpenGLShaderProgram* shader = world->getTerrainShader();
    shader->bind();
    shader->setUniformValue("chunkLines", on);
}

void MapView::setModelRotationX(double value)
{
    app().setSetting("modelRotationX", value);

    world->getObjectBrush()->rotation_x = value;
}

void MapView::setModelRotationY(double value)
{
    app().setSetting("modelRotationY", value);

    world->getObjectBrush()->rotation_y = value;
}

void MapView::setModelRotationZ(double value)
{
    app().setSetting("modelRotationZ", value);

    world->getObjectBrush()->rotation_z = value;
}

void MapView::setModelScale(double value)
{
    app().setSetting("modelScale", value);

    world->getObjectBrush()->scale = value;
}

void MapView::setModelImpend(double value)
{
    app().setSetting("modelImpend", value);

    world->getObjectBrush()->impend = value;
}

void MapView::set3DStreoscopic(bool enable)
{
    stereoscopic = enable;
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

    world->getGLFunctions()->glReadPixels(MathHelper::toInt(mouseX), MathHelper::toInt(posY), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &posZ);

    QVector4D clickedPointOnScreen(mouseX, posY, 2.0f * posZ - 1.0f, 1.0f);
    QVector4D clickedPointIn3DOrgn = inverted * clickedPointOnScreen;

    clickedPointIn3DOrgn /= clickedPointIn3DOrgn.w();

    return clickedPointIn3DOrgn.toVector3DAffine();
}

void MapView::save()
{
    world->save();
}

void MapView::keyPressEvent(QKeyEvent* e)
{
    switch(e->key())
    {
        case Qt::Key_Escape:
            escapeDown = true;
            break;

        case Qt::Key_Delete:
            deleteDown = true;
            break;

        case Qt::Key_W:
            {
                if(!camera->lock())
                    setForwardSpeed(speed * speed_mult);
            }
            break;

        case Qt::Key_S:
            {
                if(!camera->lock())
                    setForwardSpeed(-speed * speed_mult);
            }
            break;

        case Qt::Key_A:
            {
                if(!camera->lock())
                    setSideSpeed(-speed * speed_mult);
            }
            break;

        case Qt::Key_D:
            {
                if(!camera->lock())
                    setSideSpeed(speed * speed_mult);
            }
            break;

        case Qt::Key_Q:
            {
                if(!camera->lock())
                {
                    setForwardSpeed(speed * speed_mult);
                    setSideSpeed(-speed * speed_mult);
                }
            }
            break;

        case Qt::Key_E:
            {
                if(!camera->lock())
                {
                    setForwardSpeed(speed * speed_mult);
                    setSideSpeed(speed * speed_mult);
                }
            }
            break;

        case Qt::Key_Space:
            {
                if(!camera->lock())
                    setVerticalSpeed(speed * speed_mult);
            }
            break;

        case Qt::Key_X:
            {
                if(!camera->lock())
                    setVerticalSpeed(-speed * speed_mult);
            }
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
            QOpenGLWidget::keyPressEvent(e);
    }

    switch(e->nativeVirtualKey()) // Czech keyboard
    {
        case Qt::Key_2: // 1 is binded for speed :/
            {
                if(eMode == Default)
                    emit setPreviousModel();
            }
            break;

        case Qt::Key_3:
            {
                if(eMode == Default)
                    emit setNextModel();
            }
            break;

        case Qt::Key_4:
            {
                if(eMode == Default) // wait for click then spawn model, and select it
                    spawn_on_click = true;
            }
            break;
    }
}

void MapView::keyReleaseEvent(QKeyEvent* e)
{
    switch (e->key())
    {
        case Qt::Key_Escape:
            escapeDown = false;
            break;

        case Qt::Key_W:
        case Qt::Key_S:
            {
                if(!camera->lock())
                    setForwardSpeed(0.0f);
            }
            break;

        case Qt::Key_D:
        case Qt::Key_A:
            {
                if(!camera->lock())
                    setSideSpeed(0.0f);
            }
            break;

        case Qt::Key_Q:
        case Qt::Key_E:
            {
                if(!camera->lock())
                {
                    setForwardSpeed(0.0f);
                    setSideSpeed(0.0f);
                }
            }
            break;

        case Qt::Key_Space:
        case Qt::Key_X:
            {
                if(!camera->lock())
                    setVerticalSpeed(0.0f);
            }
            break;

        case Qt::Key_Shift:
            //setViewCenterFixed(false);
            shiftDown = false;
            break;

        case Qt::Key_Control:
            ctrlDown = false;
            break;

        case Qt::Key_Alt:
            {
                altDown = false;

                world->unHighlight();
            }
            break;

        default:
            QOpenGLWidget::keyReleaseEvent(e);
    }
}

void MapView::mousePressEvent(QMouseEvent* e)
{
    if(e->button() == Qt::LeftButton)
        leftButtonPressed = wasLeftButtonPressed = true;

    if(e->button() == Qt::RightButton)
        rightButtonPressed = true;

    if(e->button() == Qt::MidButton)
        wheelButtonPressed = wasWheelButtonPressed = true;

    if(leftButtonPressed && rightButtonPressed && !camera->lock())
        setForwardSpeed(speed * speed_mult);

    mousePos = prevMousePos = mousePosStart = e->pos();

    QOpenGLWidget::mousePressEvent(e);
}

void MapView::mouseReleaseEvent(QMouseEvent* e)
{
    if(e->button() == Qt::LeftButton)
    {
        leftButtonPressed     = false;
        wasLeftButtonReleased = true;

        if(m_v.z() > 0.0f)
            setForwardSpeed(0.0f);

        if(cameraCurvePoint.count() != 0)
            cameraCurvePoint.clear();
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

void MapView::mouseMoveEvent(QMouseEvent* e)
{
    mousePos  = e->pos();
    mousePosZ = terrain_pos.z();

    float dx =  0.4f * (mousePos.x() - prevMousePos.x());
    float dy = -0.4f * (mousePos.y() - prevMousePos.y());
    float dz =  0.4f * (mousePosZ    - prevMousePosZ);

    mouseVector.setX(-camera->aspectRatio() * MathHelper::toFloat(mousePos.x() - prevMousePos.x()) / MathHelper::toFloat(width()));
    mouseVector.setY(-MathHelper::toFloat(mousePos.y() - prevMousePos.y()) / MathHelper::toFloat(height()));

    if((leftButtonPressed || rightButtonPressed) && !shiftDown && !altDown && !ctrlDown && !camera->lock())
    {
        pan(dx);
        tilt(dy);
    }
    else if(leftButtonPressed && altDown)
    {
        if(eMode == Terrain || eMode == Texturing || eMode == VertexShading || eMode == VertexLighting)
            updateBrushOuterRadius(dx);
    }
    else if(rightButtonPressed && altDown)
    {
        if(eMode == Terrain || eMode == Texturing || eMode == VertexShading || eMode == VertexLighting)
            updateBrushInnerRadius(dx);
    }

    prevMousePos  = mousePos;
    prevMousePosZ = mousePosZ;
    object_move   = QVector3D(dx / 80.0f, dy / 80.0f, dz / 80.0f);

    mouse_position = this->mapFromGlobal(QCursor::pos());

    QOpenGLWidget::mouseMoveEvent(e);
}

void MapView::tabletEvent(QTabletEvent* e)
{
    if(e->device() != QTabletEvent::Stylus)
        return;

    if(e->type() == QEvent::TabletPress)
    {
        tablet = true;

        wacom.device  = e->device();
        wacom.pointer = e->pointerType();
    }
    else if(e->type() == QEvent::TabletRelease)
    {
        tablet = false;

        wacom.pressure = 1.0f;
    }
    else if(e->type() == QEvent::TabletMove)
    {
        wacom.pressure           = e->pressure();
        wacom.rotation           = e->rotation();
        wacom.tangentialPressure = e->tangentialPressure();
        wacom.x                  = e->x();
        wacom.y                  = e->y();
        wacom.xTilt              = e->xTilt();
        wacom.yTilt              = e->yTilt();

        mousePos  = e->pos();
        mousePosZ = terrain_pos.z();

        float dx =  0.4f * (mousePos.x() - prevMousePos.x());
        float dy = -0.4f * (mousePos.y() - prevMousePos.y());
        float dz =  0.4f * (mousePosZ    - prevMousePosZ);

        if(e->pointerType() == QTabletEvent::Cursor)
        {
            if(rightButtonPressed && !shiftDown && !altDown && !ctrlDown && !camera->lock())
            {
                pan(dx);
                tilt(dy);
            }
            else if(altDown)
            {
                if(eMode == Terrain || eMode == Texturing || eMode == VertexShading || eMode == VertexLighting)
                    updateBrushOuterRadius(dx);
            }
            else if(rightButtonPressed && altDown)
            {
                if(eMode == Terrain || eMode == Texturing || eMode == VertexShading || eMode == VertexLighting)
                    updateBrushInnerRadius(dx);
            }
        }

        prevMousePos  = mousePos;
        prevMousePosZ = mousePosZ;
        object_move   = QVector3D(dx / 40.0f, dy / 40.0f, dz / 40.0f);

        mouse_position = this->mapFromGlobal(QCursor::pos());
    }

    QOpenGLWidget::tabletEvent(e);
}

void MapView::wheelEvent(QWheelEvent* e)
{
    if(!shiftDown && !altDown && !ctrlDown)
        dynamicZoom.push_back(-(MathHelper::toFloat(e->delta()) / 240));

    QOpenGLWidget::wheelEvent(e);
}

void MapView::focusInEvent(QFocusEvent* e)
{
    altDown   = false;
    ctrlDown  = false;
    shiftDown = false;

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

void MapView::focusOutEvent(QFocusEvent* e)
{
    altDown   = false;
    ctrlDown  = false;
    shiftDown = false;

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
