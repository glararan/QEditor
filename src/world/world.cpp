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

#include "world.h"

#include "maptile.h"
#include "watertile.h"
#include "mathhelper.h"
#include "qeditor.h"
#include "texturemanager.h"

// Recent projects
#include "ui/startup.h"

#include <QMessageBox>
#include <QDir>

World::World(const ProjectFileData& projectFile)
: camera(0)
, projectData(projectFile)
, brush(new Brush(Brush::Types(), 3.0f, 10.0f, app().getSetting("outerBrushColor", QColor(0, 255, 0)).value<QColor>(), app().getSetting("innerBrushColor", QColor(0, 255, 0)).value<QColor>(), 5.3f))
, objectBrush(new ObjectBrush())
, textureManager(NULL)
, highlightChunk(NULL)
, possibleModel(NULL)
, modelManager(NULL)
, terrainMaximumHeight(0.0f)
, terrainMaximumState(false)
, paintMaximumAlpha(1.0f)
, paintMaximumState(false)
, sunTheta(30.0f)
, alphaMapSize(MAP_WIDTH / CHUNKS)
, eDisplay(TexturedAndLit)
, GLfuncs(0)
{
    eDisplayNames << QStringLiteral("shadeSimpleWireFrame")
                  << QStringLiteral("shadeWorldHeight")
                  << QStringLiteral("shadeWorldNormal")
                  << QStringLiteral("shaderBaseLayer")
                  << QStringLiteral("shadeBaseAndLayer1")
                  << QStringLiteral("shadeBaseLayer1AndLayer2")
                  << QStringLiteral("shadeLightingFactors")
                  << QStringLiteral("shadeTexturedAndLit")
                  << QStringLiteral("shadeWorldTexturedWireframed")
                  << QStringLiteral("shadeHidden");
}

World::~World()
{    
    for(int x = 0; x < TILES; ++x)
    {
        for(int y = 0; y < TILES; ++y)
        {
            delete mapTiles[x][y].tile;

            mapTiles[x][y].tile = NULL;
        }
    }

    delete textureManager;
    delete modelManager;
    delete objectBrush;
    delete possibleModel;
    delete modelShader;
    delete terrainShader;
    delete waterShader;
}

void World::deleteMe()
{
    delete this;
}

void World::initialize(QOpenGLContext* context, QSize fboSize)
{
    GLfuncs = context->versionFunctions<QOpenGLFunctions_4_2_Core>();

    if(!GLfuncs)
    {
        qDebug() << "OpenGL Version: " << QString::fromLocal8Bit((char*)glGetString(GL_VERSION));
        qDebug() << "OpenGL Vendor:"   << QString::fromLocal8Bit((char*)glGetString(GL_VENDOR));
        qDebug() << "OpenGL Rendered:" << QString::fromLocal8Bit((char*)glGetString(GL_RENDERER));

        qDebug() << QObject::tr("Requires OpenGL >= 4.2");

        qFatal(QObject::tr("Requires OpenGL >= 4.2").toLatin1().data());

        exit(1);
    }

    GLfuncs->initializeOpenGLFunctions();

    if(!QOpenGLFramebufferObject::hasOpenGLFramebufferObjects())
    {
        qFatal(QObject::tr("Requires OpenGL framebuffer object.").toLatin1().data());
        exit(1);
    }

    qDebug() << "OpenGL Version: " << QString::fromLocal8Bit((char*)GLfuncs->glGetString(GL_VERSION));
    qDebug() << "OpenGL Vendor:"   << QString::fromLocal8Bit((char*)GLfuncs->glGetString(GL_VENDOR));
    qDebug() << "OpenGL Rendered:" << QString::fromLocal8Bit((char*)GLfuncs->glGetString(GL_RENDERER));

    app().setGraphics(QString::fromLocal8Bit((char*)GLfuncs->glGetString(GL_VENDOR)));

    // initialize texture manager
    textureManager = new TextureManager(this, app().getSetting("antialiasing", 1.0f).toFloat());

    // initialize model manager
    modelManager = new IModelManager();
    modelManager->loadModels(projectData.projectRootDir);

    for(int x = 0; x < TILES; ++x)
    {
        for(int y = 0; y < TILES; ++y)
        {
            int index = y * TILES + x;

            if(projectData.maps[index].exists == 0)
                mapTiles[x][y].tile = NULL;
            else
            {
                loadTile(x, y);

                mapTiles[x][y].tile->setFboSize(fboSize);
            }
        }
    }

    // initialize fbo
    QOpenGLFramebufferObjectFormat format;
    format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
    format.setMipmap(true);

    fbo = new QOpenGLFramebufferObject(QSize(8192, 4608), format);

    viewportSize = fboSize;

    //// initialize shaders
    /// Model Shader
    modelShader = new QOpenGLShaderProgram();

    if(!modelShader->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/data/shaders/model.vert"))
        qCritical() << QObject::tr("Could not compile vertex shader. Log:") << modelShader->log();

    if(!modelShader->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/data/shaders/model.frag"))
        qCritical() << QObject::tr("Could not compile fragment shader. Log:") << modelShader->log();

    if(!modelShader->link())
        qCritical() << QObject::tr("Could not link shader program. Log:") << modelShader->log();

    /// Terrain Shader
    terrainShader = new QOpenGLShaderProgram();

    if(!terrainShader->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/data/shaders/qeditor.vert"))
        qCritical() << QObject::tr("Could not compile vertex shader. Log:") << terrainShader->log();

    if(!terrainShader->addShaderFromSourceFile(QOpenGLShader::TessellationControl, ":/data/shaders/qeditor.tcs"))
        qCritical() << QObject::tr("Could not compile tessellation control shader. Log:") << terrainShader->log();

    if(!terrainShader->addShaderFromSourceFile(QOpenGLShader::TessellationEvaluation, ":/data/shaders/qeditor.tes"))
        qCritical() << QObject::tr("Could not compile tessellation evaluation shader. Log:") << terrainShader->log();

    if(!terrainShader->addShaderFromSourceFile(QOpenGLShader::Geometry, ":/data/shaders/qeditor.geom"))
        qCritical() << QObject::tr("Could not compile geometry shader. Log:") << terrainShader->log();

    if(!terrainShader->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/data/shaders/qeditor.frag"))
        qCritical() << QObject::tr("Could not compile fragment shader. Log:") << terrainShader->log();

    if(!terrainShader->link())
        qCritical() << QObject::tr("Could not link shader program. Log:") << terrainShader->log();

    /// Water Shader
    waterShader = new QOpenGLShaderProgram();

    if(!waterShader->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/data/shaders/qeditor.vert"))
        qCritical() << QObject::tr("Could not compile vertex shader. Log:") << waterShader->log();

    if(!waterShader->addShaderFromSourceFile(QOpenGLShader::TessellationControl, ":/data/shaders/qeditor.tcs"))
        qCritical() << QObject::tr("Could not compile tessellation control shader. Log:") << waterShader->log();

    if(!waterShader->addShaderFromSourceFile(QOpenGLShader::TessellationEvaluation, ":/data/shaders/qeditor.tes"))
        qCritical() << QObject::tr("Could not compile tessellation evaluation shader. Log:") << waterShader->log();

    if(!waterShader->addShaderFromSourceFile(QOpenGLShader::Geometry, ":/data/shaders/qeditor.geom"))
        qCritical() << QObject::tr("Could not compile geometry shader. Log:") << waterShader->log();

    if(!waterShader->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/data/shaders/qeditor_water.frag"))
        qCritical() << QObject::tr("Could not compile fragment shader. Log:") << waterShader->log();

    if(!waterShader->link())
        qCritical() << QObject::tr("Could not link shader program. Log:") << waterShader->log();


    /// Shader data - terrain
    terrainShader->bind();

    // Set line parameters
    terrainShader->setUniformValue("line.width", 0.2f);
    terrainShader->setUniformValue("line.color", app().getSetting("wireframe", QVector4D(0.5f, 1.0f, 0.0f, 1.0f)).value<QVector4D>());
    //terrainShader->setUniformValue("line.color", QVector4D(0.17f, 0.50f, 1.0f, 1.0f)); // blue
    terrainShader->setUniformValue("line.color2", app().getSetting("terrainWireframe", QVector4D(0.0f, 0.0f, 0.0f, 0.0f)).value<QVector4D>());

    // Set the fog parameters
    terrainShader->setUniformValue("fog.color"      , QVector4D(0.65f, 0.77f, 1.0f, 1.0f));
    terrainShader->setUniformValue("fog.minDistance", app().getSetting("environmentDistance", 256.0f).toFloat() / 2.0f);
    terrainShader->setUniformValue("fog.maxDistance", app().getSetting("environmentDistance", 256.0f).toFloat() - 32.0f);

    // Set the horizontal and vertical scales applied in the tess eval shader
    terrainShader->setUniformValue("horizontalScale", CHUNKSIZE);

    // Set the lighting parameters
    terrainShader->setUniformValue("light.intensity", QVector3D(1.0f, 1.0f, 1.0f));

    // Set the material properties
    terrainShader->setUniformValue("material.Ka", QVector3D(0.1f, 0.1f, 0.1f));
    terrainShader->setUniformValue("material.Kd", QVector3D(1.0f, 1.0f, 1.0f));
    terrainShader->setUniformValue("material.Ks", QVector3D(0.3f, 0.3f, 0.3f));
    terrainShader->setUniformValue("material.shininess", 10.0f);

    /// Shader data - water
    waterShader->bind();

    // Set line parameters
    waterShader->setUniformValue("line.width", 0.2f);
    waterShader->setUniformValue("line.color", app().getSetting("wireframe", QVector4D(0.5f, 1.0f, 0.0f, 1.0f)).value<QVector4D>());
    //waterShader->setUniformValue("line.color", QVector4D(0.17f, 0.50f, 1.0f, 1.0f)); // blue
    waterShader->setUniformValue("line.color2", app().getSetting("terrainWireframe", QVector4D(0.0f, 0.0f, 0.0f, 0.0f)).value<QVector4D>());

    // Set the fog parameters
    waterShader->setUniformValue("fog.color"      , QVector4D(0.65f, 0.77f, 1.0f, 1.0f));
    waterShader->setUniformValue("fog.minDistance", app().getSetting("environmentDistance", 256.0f).toFloat() / 2.0f);
    waterShader->setUniformValue("fog.maxDistance", app().getSetting("environmentDistance", 256.0f).toFloat() - 32.0f);

    // Set the horizontal and vertical scales applied in the tess eval shader
    waterShader->setUniformValue("horizontalScale", CHUNKSIZE);

    // Set the lighting parameters
    waterShader->setUniformValue("light.intensity", QVector3D(1.0f, 1.0f, 1.0f));

    // Set the material properties
    waterShader->setUniformValue("material.Ka", QVector3D(0.1f, 0.1f, 0.1f));
    waterShader->setUniformValue("material.Kd", QVector3D(1.0f, 1.0f, 1.0f));
    waterShader->setUniformValue("material.Ks", QVector3D(0.3f, 0.3f, 0.3f));
    waterShader->setUniformValue("material.shininess", 10.0f);

    /// MapChunk neighbours
    createNeighbours();
}

void World::update(float dt)
{
    Q_UNUSED(dt);

    QOpenGLShaderProgram* shader = getWaterShader();
    shader->bind();

    shader->setUniformValue("deltaTime", dt);

    shader = getTerrainShader();
    shader->bind();

    shader->setUniformValue("deltaTime", dt);

    // load tiles around + load neighbours
    int tileX = MathHelper::toInt(camera->pos().x() / TILESIZE);
    int tileY = MathHelper::toInt(camera->pos().z() / TILESIZE);

    for(int tx = tileX - 1; tx < tileX + 2; ++tx)
    {
        for(int ty = tileY - 1; ty < tileY + 2; ++ty)
        {
            if(!hasTile(tx, ty) || (tx == tileX && ty == tileY) || !tileLoaded(tx, ty))
                continue;

            QFile file(QString("%1_%2_%3.qem").arg(projectData.mapName).arg(tx).arg(ty));

            if(file.exists())
                loadTile(tx, ty);
        }
    }

    // unload tiles around
    /*for(int tx = 0; tx < TILES; ++tx)
    {
        for(int ty = 0; tx < TILES; ++ty)
        {
            if(hasTile(tx, ty) && tileLoaded(tx, ty) && (tx > tileX + 1 || tx < tileX - 1) && (ty > tileY + 1 && ty < tileY - 1))
            {
                delete mapTiles[tx][ty].tile;

                mapTiles[tx][ty].tile = NULL;
            }
        }
    }*/
}

void World::draw(MapView* mapView, QVector3D& terrain_pos, QMatrix4x4 modelMatrix, float triangles, QVector2D mousePosition, bool drawBrush, bool drawNewModel)
{
    QMatrix4x4 viewMatrix        = camera->viewMatrix();
    QMatrix4x4 modelViewMatrix   = viewMatrix * modelMatrix;
    QMatrix3x3 worldNormalMatrix = modelMatrix.normalMatrix();
    QMatrix3x3 normalMatrix      = modelViewMatrix.normalMatrix();
    QMatrix4x4 mvp               = camera->projectionMatrix() * modelViewMatrix;

    // Set the lighting parameters
    QVector4D worldLightDirection(sinf(sunTheta * MathHelper::degreesToRadians(1.0f)), cosf(sunTheta * MathHelper::degreesToRadians(1.0f)), 0.0f, 0.0f);
    QMatrix4x4 worldToEyeNormal(normalMatrix);

    QVector4D lightDirection = worldToEyeNormal * worldLightDirection;

    ///GLfuncs->glBlendFunc(GL_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA);
    GLfuncs->glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    for(int i = 0; i < 2; ++i)
    {
        QOpenGLShaderProgram* shader = (i == 0) ? getTerrainShader() : getWaterShader();
        shader->bind();

        // Set the horizontal and vertical scales applied in the tess eval shader
        shader->setUniformValue("pixelsPerTriangleEdge", triangles);

        // Pass in the usual transformation matrices
        shader->setUniformValue("modelMatrix"      , modelMatrix);
        shader->setUniformValue("modelViewMatrix"  , modelViewMatrix);
        shader->setUniformValue("worldNormalMatrix", worldNormalMatrix);
        shader->setUniformValue("normalMatrix"     , normalMatrix);
        shader->setUniformValue("mvp"              , mvp);

        // Set the lighting parameters
        shader->setUniformValue("light.position", lightDirection);

        if(drawBrush && i == 0)
            brush->draw(shader, QVector2D(worldCoordinates.x(), worldCoordinates.z()));
    }

    for(int tx = 0; tx < TILES; ++tx)
    {
        for(int ty = 0; ty < TILES; ++ty)
        {
            if(tileLoaded(tx, ty))
            {
                mapTiles[tx][ty].tile->draw(MAP_DRAW_DISTANCE, camera->position());
                mapTiles[tx][ty].tile->drawObjects(MAP_DRAW_DISTANCE, camera->position(), camera->viewMatrix(), camera->projectionMatrix());
            }
        }
    }

    //get world coord
    worldCoordinates = terrain_pos = mapView->getWorldCoordinates(mousePosition.x(), mousePosition.y());

    for(int tx = 0; tx < TILES; ++tx)
    {
        for(int ty = 0; ty < TILES; ++ty)
        {
            if(tileLoaded(tx, ty))
            {
                GLfuncs->glEnable(GL_BLEND);
                    GLfuncs->glDisable(GL_DEPTH_TEST);
                    mapTiles[tx][ty].tile->drawWater(MAP_DRAW_DISTANCE, camera->position());
                    GLfuncs->glEnable(GL_DEPTH_TEST);
                GLfuncs->glDisable(GL_BLEND);
            }
        }
    }

    GLfuncs->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if(possibleModel && drawNewModel && modelManager->isModelSelected() && getTileAt(worldCoordinates.x(), worldCoordinates.z()))
    {
        IPipeline Pipeline(modelMatrix, camera->viewMatrix(), camera->projectionMatrix());
        Pipeline.translate(worldCoordinates.x(), worldCoordinates.y() + (getObjectBrush()->impend * 10.0f), worldCoordinates.z());
        Pipeline.scale(getObjectBrush()->scale);
        Pipeline.rotateX(getObjectBrush()->rotation_x * 360.0f);
        Pipeline.rotateY(getObjectBrush()->rotation_y * 360.0f);
        Pipeline.rotateZ(getObjectBrush()->rotation_z * 360.0f);

        modelShader->bind();

        Pipeline.updateMatrices(modelShader);

        possibleModel->draw(modelShader);
    }

    //material->bind();
}

inline bool okTile(int x, int y)
{
    return !(x < 0 || y < 0 || x > TILES || y > TILES);
}

bool World::hasTile(int pX, int pY) const
{
    return okTile(pX, pY);
}

bool World::tileLoaded(int x, int y) const
{
    return hasTile(x, y) && mapTiles[x][y].tile;
}

MapTile* World::loadTile(int x, int y, bool fileExists)
{
    if(!hasTile(x, y))
        return NULL;

    if(tileLoaded(x, y))
        return mapTiles[x][y].tile;

    const QString filename(QString("%1_%2_%3.qem").arg(projectData.mapName).arg(x).arg(y));

    if(fileExists)
    {
        // todo
        // check file in archive
        // load from archive
        mapTiles[x][y].tile = new MapTile(this, x, y, filename);
    }
    else
        mapTiles[x][y].tile = new MapTile(this, filename, x, y);

    return mapTiles[x][y].tile;
}

void World::loadNewProjectMapTilesIntoMemory(bool** mapCoords, QSize size)
{
    for(int x = 0; x < TILES; ++x)
    {
        for(int y = 0; y < TILES; ++y)
        {
            if(mapCoords[x][y])
            {
                loadTile(x, y, false);

                mapTiles[x][y].tile->setFboSize(size);
            }
        }
    }

    createNeighbours();
}

void World::changeTerrain(float x, float z, float change)
{
    for(int tx = 0; tx < TILES; ++tx)
    {
        for(int ty = 0; ty < TILES; ++ty)
        {
            if(tileLoaded(tx, ty))
            {
                for(int cx = 0; cx < CHUNKS; ++cx)
                {
                    for(int cy = 0; cy < CHUNKS; ++cy)
                    {
                        if(mapTiles[tx][ty].tile->getChunk(cx, cy)->changeTerrain(x, z, change))
                        {
                            // push changed chunks
                            // set changed i, j
                        }
                    }
                }
            }
        }
    }
}

void World::flattenTerrain(float x, float z, float y, float change)
{
    for(int tx = 0; tx < TILES; ++tx)
    {
        for(int ty = 0; ty < TILES; ++ty)
        {
            if(tileLoaded(tx, ty))
            {
                for(int cx = 0; cx < CHUNKS; ++cx)
                {
                    for(int cy = 0; cy < CHUNKS; ++cy)
                    {
                        if(mapTiles[tx][ty].tile->getChunk(cx, cy)->flattenTerrain(x, z, y, change))
                        {
                            // push changed chunks
                            // set changed i, j
                        }
                    }
                }
            }
        }
    }
}

void World::blurTerrain(float x, float z, float change)
{
    for(int tx = 0; tx < TILES; ++tx)
    {
        for(int ty = 0; ty < TILES; ++ty)
        {
            if(tileLoaded(tx, ty))
            {
                for(int cx = 0; cx < CHUNKS; ++cx)
                {
                    for(int cy = 0; cy < CHUNKS; ++cy)
                    {
                        if(mapTiles[tx][ty].tile->getChunk(cx, cy)->blurTerrain(x, z, change))
                        {
                            // push changed chunks
                            // set changed i, j
                        }
                    }
                }
            }
        }
    }
}

void World::paintTerrain(float x, float z, float flow)
{
    for(int tx = 0; tx < TILES; ++tx)
    {
        for(int ty = 0; ty < TILES; ++ty)
        {
            if(tileLoaded(tx, ty))
            {
                for(int cx = 0; cx < CHUNKS; ++cx)
                {
                    for(int cy = 0; cy < CHUNKS; ++cy)
                    {
                        if(mapTiles[tx][ty].tile->getChunk(cx, cy)->paintTerrain(x, z, flow, textureManager->getSelectedTexture()))
                        {
                            // push changed chunks
                            // set changed i, j
                        }
                    }
                }
            }
        }
    }
}

void World::paintVertexShading(float x, float z, float flow, QColor& color)
{
    for(int tx = 0; tx < TILES; ++tx)
    {
        for(int ty = 0; ty < TILES; ++ty)
        {
            if(tileLoaded(tx, ty))
            {
                for(int cx = 0; cx < CHUNKS; ++cx)
                {
                    for(int cy = 0; cy < CHUNKS; ++cy)
                    {
                        if(mapTiles[tx][ty].tile->getChunk(cx, cy)->paintVertexShading(x, z, flow, color))
                        {
                            // push changed chunks
                            // set changed i, j
                        }
                    }
                }
            }
        }
    }
}

void World::removeObject(float x, float z)
{
    for(int tx = 0; tx < TILES; ++tx)
    {
        for(int ty = 0; ty < TILES; ++ty)
        {
            if(tileLoaded(tx, ty))
                mapTiles[tx][ty].tile->deleteModel(x, z);
        }
    }
}

void World::highlightMapChunkAt(const QVector3D& position)
{
    int tx = floor(position.x() / TILESIZE);
    int ty = floor(position.z() / TILESIZE);

    if(!tileLoaded(tx, ty))
        return;

    int cx = floor((position.x() - TILESIZE * tx) / CHUNKSIZE);
    int cy = floor((position.z() - TILESIZE * ty) / CHUNKSIZE);

    if(cx >= CHUNKS || cy >= CHUNKS)
    {
        qDebug() << QObject::tr("highlightMapChunkAt cx or cy is wrong calculated!");

        return;
    }

    if(highlightChunk == mapTiles[tx][ty].tile->getChunk(cx, cy))
        return;

    if(highlightChunk != NULL)
        highlightChunk->setHighlight(false);

    highlightChunk = mapTiles[tx][ty].tile->getChunk(cx, cy);
    highlightChunk->setHighlight(true);
}

void World::unHighlight()
{
    if(highlightChunk != NULL)
    {
        highlightChunk->setHighlight(false);

        highlightChunk = NULL;
    }
}

void World::updateNewModel(bool shiftDown, bool leftButtonPressed)
{
    int current_index = modelManager->getCurrentModel();

    if(possibleModel && current_index >= 0)
    {
        QString path = possibleModel->getModelInterface()->getFilePath() + possibleModel->getModelInterface()->getFileName();

        if(modelManager->getIndex(path) != current_index)
        {
            delete possibleModel;

            possibleModel = new IModel(modelManager, current_index);
        }
    }
    else if(current_index >= 0)
        possibleModel = new IModel(modelManager, current_index);

    if(leftButtonPressed && shiftDown) //insert new model
    {
        MapTile* tile = getTileAt(worldCoordinates.x(), worldCoordinates.z());

        if(tile)
        {
            MapObject* object = new MapObject();

            object->model         = new IModel(modelManager, current_index);
            object->translate     = worldCoordinates;
            object->height_offset = getObjectBrush()->impend * 10.0f;
            object->rotation      = QVector3D(getObjectBrush()->rotation_x, getObjectBrush()->rotation_y, getObjectBrush()->rotation_z);
            object->scale         = QVector3D(getObjectBrush()->scale, getObjectBrush()->scale, getObjectBrush()->scale);

            tile->insertModel(object);
        }
    }
}

MapChunk* World::getMapChunkAt(const QVector3D& position) const
{
    int tx = floor(position.x() / TILESIZE);
    int ty = floor(position.z() / TILESIZE);

    if(!tileLoaded(tx, ty))
    {
        // disable selected chunk
        for(int tx = 0; tx < TILES; ++tx)
        {
            for(int ty = 0; ty < TILES; ++ty)
            {
                if(tileLoaded(tx, ty))
                {
                    for(int cx = 0; cx < CHUNKS; ++cx)
                    {
                        for(int cy = 0; cy < CHUNKS; ++cy)
                        {
                            mapTiles[tx][ty].tile->getChunk(cx, cy)->setSelected(false);
                        }
                    }
                }
            }
        }

        return NULL;
    }

    int cx = floor((position.x() - TILESIZE * tx) / CHUNKSIZE);
    int cy = floor((position.z() - TILESIZE * ty) / CHUNKSIZE);

    if(cx >= CHUNKS || cy >= CHUNKS)
    {
        qDebug() << QObject::tr("getMapChunkAt cx or cy is wrong calculated!");

        return NULL;
    }

    // disable selected chunk
    for(int tx = 0; tx < TILES; ++tx)
    {
        for(int ty = 0; ty < TILES; ++ty)
        {
            if(tileLoaded(tx, ty))
            {
                for(int cx = 0; cx < CHUNKS; ++cx)
                {
                    for(int cy = 0; cy < CHUNKS; ++cy)
                    {
                        mapTiles[tx][ty].tile->getChunk(cx, cy)->setSelected(false);
                    }
                }
            }
        }
    }

    // enable selected chunk
    mapTiles[tx][ty].tile->getChunk(cx, cy)->setSelected(true);

    return mapTiles[tx][ty].tile->getChunk(cx, cy);
}

WaterChunk* World::getWaterChunkAt(const QVector3D& position) const
{
    int tx = floor(position.x() / TILESIZE);
    int ty = floor(position.z() / TILESIZE);

    if(!tileLoaded(tx, ty))
        return NULL;

    int cx = floor((position.x() - TILESIZE * tx) / CHUNKSIZE);
    int cy = floor((position.z() - TILESIZE * ty) / CHUNKSIZE);

    if(cx >= CHUNKS || cy >= CHUNKS)
    {
        qDebug() << QObject::tr("getWaterChunkAt cx or cy is wrong calculated!");

        return NULL;
    }

    return mapTiles[tx][ty].tile->getWater()->getChunk(cx, cy);
}

MapTile* World::getTileAt(float x, float z) const
{
    int tx = floor(x / TILESIZE);
    int ty = floor(z / TILESIZE);

    if(tx >= TILES || ty >= TILES || tx < 0 || ty < 0)
        return 0;

    if(!tileLoaded(tx, ty))
        return 0;

    return mapTiles[tx][ty].tile;
}

QVector3D World::getWorldCoordinates() const
{
    return worldCoordinates;
}

void World::save()
{
    QFile projectFile(QDir(projectData.projectRootDir).filePath(projectData.projectFile + ".qep"));

    bool rewrite = false;

    QVector<QPair<int, int>> tileContainer;

    if(!projectFile.exists())
        rewrite = true;
    else
    {
        if(!projectFile.open(QIODevice::ReadOnly))
        {
            qCritical(QString(QObject::tr("We couldn't save project %1, because we can't open him.")).arg(projectData.projectName).toLatin1().constData());

            return;
        }

        QDataStream data(&projectFile);
        data.setVersion(QDataStream::Qt_5_0);

        data >> projectData;

        for(int i = 0; i < (int)projectData.mapsCount; ++i)
            tileContainer.append(qMakePair<int, int>(projectData.maps[i].x, projectData.maps[i].y));
    }

    if(projectFile.exists() && (projectData.version < PROJECT_VERSION || projectData.version > PROJECT_VERSION))
    {
        QMessageBox msg;

        if(projectData.version < PROJECT_VERSION)
        {
            msg.setWindowTitle(QObject::tr("Save"));
            msg.setIcon(QMessageBox::Warning);
            msg.setText(QObject::tr("Your project file is older, you have to upgrade your project file to save current work!"));
            msg.setInformativeText(QObject::tr("Warning: You not be able to open this project in older version of QEditor!"));
            msg.setDefaultButton(QMessageBox::Save);
            msg.setStandardButtons(QMessageBox::Save | QMessageBox::Cancel);

            int result = msg.exec();

            if(result != QMessageBox::Save)
                return;
        }
        else
        {
            msg.setWindowTitle("Error");
            msg.setIcon(QMessageBox::Critical);
            msg.setText(QObject::tr("Your project file is newer, you cannot use older version of QEditor to edit!"));

            msg.exec();

            return;
        }
    }

    for(int x = 0; x < TILES; ++x)
    {
        for(int y = 0; y < TILES; ++y)
        {
            if(tileLoaded(x, y))
            {
                bool found = false;

                for(int i = 0; i < tileContainer.count(); ++i)
                {
                    if(tileContainer.at(i).first == x && tileContainer.at(i).second == y)
                    {
                        found = true;

                        break;
                    }
                }

                if(!found)
                {
                    rewrite = true;

                    tileContainer.append(qMakePair<int, int>(x, y));
                }

                mapTiles[x][y].tile->saveTile();
            }
        }
    }

    if(rewrite)
    {
        projectFile.close();

        if(!projectFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
        {
            qCritical(QString(QObject::tr("We couldn't save project %1, because we can't open him.")).arg(projectData.projectName).toLatin1().constData());

            return;
        }

        projectData.version   = PROJECT_VERSION;
        projectData.mapsCount = tileContainer.count();

        for(int i = 0; i < TILES * TILES; ++i)
        {
            projectData.maps[i].exists = 0;
            projectData.maps[i].x      = -1;
            projectData.maps[i].y      = -1;
        }

        for(int i = 0; i < tileContainer.count(); ++i)
        {
            int index = tileContainer.at(i).second * TILES + tileContainer.at(i).first;

            projectData.maps[index].exists = 1;

            projectData.maps[index].x = tileContainer.at(i).first;
            projectData.maps[index].y = tileContainer.at(i).second;
        }

        QDataStream data(&projectFile);
        data.setVersion(QDataStream::Qt_5_0);

        data << projectData;

        projectFile.flush();

        RecentProject recentProject;
        recentProject.projectName = projectData.projectName;
        recentProject.projectPath = projectData.projectRootDir;
        recentProject.projectFile = projectData.projectFile;

        StartUp::addRecentProject(recentProject);
    }

    projectFile.close();
}

void World::setFboSize(QSize size)
{
    for(int x = 0; x < TILES; ++x)
    {
        for(int y = 0; y < TILES; ++y)
        {
            if(tileLoaded(x, y))
                mapTiles[x][y].tile->setFboSize(size);
        }
    }

    viewportSize = size;
}

void World::setCamera(Camera* cam)
{
    camera = cam;
}

void World::setProjectData(ProjectFileData& data)
{
    projectData = data;
}

void World::setTerrainMaximumHeight(float value)
{
    terrainMaximumHeight = value;
}

void World::setTerrainMaximumState(bool state)
{
    terrainMaximumState = state;
}

void World::setPaintMaximumAlpha(float value)
{
    paintMaximumAlpha = value;
}

void World::setPaintMaximumState(bool state)
{
    paintMaximumState = state;
}

void World::setDisplayMode(int displayMode)
{
    if(displayMode == SimpleWireFrame || displayMode == WorldHeight || displayMode == WorldTexturedWireframed)
    {
        terrainShader->bind();

        if(displayMode == WorldTexturedWireframed)
            terrainShader->setUniformValue("line.width", 0.5f);
        else
            terrainShader->setUniformValue("line.width", 0.2f);
    }

    switch(displayMode)
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
            eDisplay = (eDisplayMode)displayMode;
            break;

        default:
            eDisplay = TexturedAndLit;
            break;
    }
}

void World::createNeighbours()
{
    for(int tx = 0; tx < TILES; ++tx)
    {
        for(int ty = 0; ty < TILES; ++ty)
        {
            if(tileLoaded(tx, ty))
            {
                for(int cx = 0; cx < CHUNKS; ++cx)
                {
                    for(int cy = 0; cy < CHUNKS; ++cy)
                    {
                        if(cy != 0)
                            mapTiles[tx][ty].tile->getChunk(cx, cy)->setBottomNeighbour(mapTiles[tx][ty].tile->getChunk(cx, cy - 1));
                        else if(ty != 0)
                        {
                            if(tileLoaded(tx, ty - 1))
                                mapTiles[tx][ty].tile->getChunk(cx, cy)->setBottomNeighbour(mapTiles[tx][ty - 1].tile->getChunk(cx, CHUNKS - 1));
                            else
                                mapTiles[tx][ty].tile->getChunk(cx, cy)->setBottomNeighbour(0);
                        }
                        else
                            mapTiles[tx][ty].tile->getChunk(cx, cy)->setBottomNeighbour(0);

                        if(cx != 0)
                            mapTiles[tx][ty].tile->getChunk(cx, cy)->setLeftNeighbour(mapTiles[tx][ty].tile->getChunk(cx - 1, cy));
                        else if(tx != 0)
                        {
                            if(tileLoaded(tx - 1, ty))
                                mapTiles[tx][ty].tile->getChunk(cx, cy)->setLeftNeighbour(mapTiles[tx - 1][ty].tile->getChunk(CHUNKS - 1, cy));
                            else
                                mapTiles[tx][ty].tile->getChunk(cx, cy)->setLeftNeighbour(0);
                        }
                        else
                            mapTiles[tx][ty].tile->getChunk(cx, cy)->setLeftNeighbour(0);
                    }
                }
            }
        }
    }
}

void World::test()
{
    /// Enable water on each tile and chunk
    /*for(int x = 0; x < TILES; ++x)
    {
        for(int y = 0; y < TILES; ++y)
        {
            if(tileLoaded(x, y))
            {
                for(int wx = 0; wx < CHUNKS; ++wx)
                {
                    for(int wy = 0; wy < CHUNKS; ++wy)
                        mapTiles[x][y].tile->getWater()->getChunk(wx, wy)->setData(true);
                }
            }
        }
    }*/

    /// Fix NaN values in heightmap
    /*MapChunk* chunk = getMapChunkAt(worldCoordinates);

    chunk->test();*/

    /// Take capture of screen in fbo
    for(int tx = 0; tx < TILES; ++tx)
    {
        for(int ty = 0; ty < TILES; ++ty)
        {
            if(tileLoaded(tx, ty))
            {
                fbo->bind();

                GLfuncs->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                GLfuncs->glViewport(0, 0, fbo->width(), fbo->height());

                mapTiles[tx][ty].tile->draw(MAP_DRAW_DISTANCE, camera->position());

                fbo->release();

                GLfuncs->glViewport(0, 0, viewportSize.width(), viewportSize.height());
            }
        }
    }

    int width, height;

    GLfuncs->glBindTexture(GL_TEXTURE_2D, fbo->texture());

    GLfuncs->glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH,  &width);
    GLfuncs->glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);

    if(width <= 0 || height <= 0)
        return;

    GLint bytes = width * height * 4;

    unsigned char* data = (unsigned char*)malloc(bytes);

    GLfuncs->glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    QImage img = QImage(data, width, height, QImage::Format_RGBA8888);

    img.save("test.png");

    /// Height fix
    //mapTiles[0][0].tile->getChunk(3, 0)->test();
    //mapTiles[0][0].tile->getChunk(2, 0)->test();
}