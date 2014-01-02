#include "world.h"

#include "maptile.h"
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
, textureManager(NULL)
, highlightChunk(NULL)
, terrainMaximumHeight(0.0f)
, terrainMaximumState(false)
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
}

void World::deleteMe()
{
    delete this;
}

void World::initialize(QOpenGLContext* context)
{
    GLfuncs = context->versionFunctions<QOpenGLFunctions_4_2_Core>();

    if(!GLfuncs)
    {
        qFatal(QObject::tr("Requires OpenGL >= 4.2").toLatin1().data());
        exit(1);
    }

    GLfuncs->initializeOpenGLFunctions();

    qDebug() << "OpenGL Version: " << QString::fromLocal8Bit((char*)GLfuncs->glGetString(GL_VERSION));
    qDebug() << "OpenGL Vendor:"   << QString::fromLocal8Bit((char*)GLfuncs->glGetString(GL_VENDOR));
    qDebug() << "OpenGL Rendered:" << QString::fromLocal8Bit((char*)GLfuncs->glGetString(GL_RENDERER));

    app().setGraphics(QString::fromLocal8Bit((char*)GLfuncs->glGetString(GL_VENDOR)));

    // initialize texture manager
    textureManager = new TextureManager(this, app().getSetting("antialiasing", 1.0f).toFloat());

    // prepare Shaders
    material = MaterialPtr(new Material);
    material->setShaders(":/shaders/qeditor_world.vert", ":/shaders/qeditor_world.frag");

    for(int x = 0; x < TILES; ++x)
    {
        for(int y = 0; y < TILES; ++y)
        {
            int index = y * TILES + x;

            if(projectData.maps[index].exists == 0)
                mapTiles[x][y].tile = NULL;
            else
                loadTile(x, y);
        }
    }

    createNeighbours();
}

void World::update(float dt)
{
    Q_UNUSED(dt);

    /*QOpenGLShaderProgramPtr shader = material->shader();
    shader->setUniformValue("waterTime", dt);*/

    // load tiles around + load neighbours
}

void World::draw(QMatrix4x4 modelMatrix, float triangles, QVector2D mousePosition, bool drawBrush)
{
    for(int x = 0; x < TILES; ++x)
    {
        for(int y = 0; y < TILES; ++y)
        {
            if(tileLoaded(x, y))
            {
                for(int tx = 0; tx < CHUNKS; ++tx)
                {
                    for(int ty = 0; ty < CHUNKS; ++ty)
                    {
                        QOpenGLShaderProgramPtr shader = mapTiles[x][y].tile->getChunk(tx, ty)->getShader();
                        shader->bind();

                        // Set the horizontal and vertical scales applied in the tess eval shader
                        shader->setUniformValue("horizontalScale"      , CHUNKSIZE);
                        shader->setUniformValue("pixelsPerTriangleEdge", triangles);

                        // Pass in the usual transformation matrices
                        QMatrix4x4 viewMatrix        = camera->viewMatrix();
                        QMatrix4x4 modelViewMatrix   = viewMatrix * modelMatrix;
                        QMatrix3x3 worldNormalMatrix = modelMatrix.normalMatrix();
                        QMatrix3x3 normalMatrix      = modelViewMatrix.normalMatrix();
                        QMatrix4x4 mvp               = camera->projectionMatrix() * modelViewMatrix;

                        shader->setUniformValue("modelMatrix"      , modelMatrix);
                        shader->setUniformValue("modelViewMatrix"  , modelViewMatrix);
                        shader->setUniformValue("worldNormalMatrix", worldNormalMatrix);
                        shader->setUniformValue("normalMatrix"     , normalMatrix);
                        shader->setUniformValue("mvp"              , mvp);

                        // Set the lighting parameters
                        QVector4D worldLightDirection(sinf(sunTheta * MathHelper::degreesToRadians(1.0f)), cosf(sunTheta * MathHelper::degreesToRadians(1.0f)), 0.0f, 0.0f);
                        QMatrix4x4 worldToEyeNormal(normalMatrix);

                        QVector4D lightDirection = worldToEyeNormal * worldLightDirection;

                        shader->setUniformValue("light.position" , lightDirection);
                        shader->setUniformValue("light.intensity", QVector3D(1.0f, 1.0f, 1.0f));

                        // Set the material properties
                        shader->setUniformValue("material.Ka", QVector3D(0.1f, 0.1f, 0.1f));
                        shader->setUniformValue("material.Kd", QVector3D(1.0f, 1.0f, 1.0f));
                        shader->setUniformValue("material.Ks", QVector3D(0.3f, 0.3f, 0.3f));
                        shader->setUniformValue("material.shininess", 10.0f);

                        if(drawBrush)
                            brush->draw(shader, mousePosition);
                    }
                }

                mapTiles[x][y].tile->draw(MAP_DRAW_DISTANCE, camera->position());
            }
        }
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

void World::loadNewProjectMapTilesIntoMemory(bool** mapCoords)
{
    for(int x = 0; x < TILES; ++x)
    {
        for(int y = 0; y < TILES; ++y)
        {
            if(mapCoords[x][y])
                loadTile(x, y, false);
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

MapChunk* World::getMapChunkAt(const QVector3D& position) const
{
    int tx = floor(position.x() / TILESIZE);
    int ty = floor(position.z() / TILESIZE);

    if(!tileLoaded(tx, ty))
        return NULL;

    int cx = floor((position.x() - TILESIZE * tx) / CHUNKSIZE);
    int cy = floor((position.z() - TILESIZE * ty) / CHUNKSIZE);

    if(cx >= CHUNKS || cy >= CHUNKS)
    {
        qDebug() << QObject::tr("getMapChunkAt cx or cy is wrong calculated!");

        return NULL;
    }

    return mapTiles[tx][ty].tile->getChunk(cx, cy);
}

void World::setChunkShaderUniform(const char* name, const QVector2D& value)
{
    for(int x = 0; x < TILES; ++x)
    {
        for(int y = 0; y < TILES; ++y)
        {
            if(tileLoaded(x, y))
            {
                for(int x2 = 0; x2 < CHUNKS; ++x2)
                {
                    for(int y2 = 0; y2 < CHUNKS; ++y2)
                    {
                        QOpenGLShaderProgramPtr shader = mapTiles[x][y].tile->getChunk(x2, y2)->getShader();
                        shader->bind();

                        shader->setUniformValue(name, value);
                    }
                }
            }
        }
    }
}
void World::setChunkShaderUniform(const char* name, const QVector4D& value)
{
    for(int x = 0; x < TILES; ++x)
    {
        for(int y = 0; y < TILES; ++y)
        {
            if(tileLoaded(x, y))
            {
                for(int x2 = 0; x2 < CHUNKS; ++x2)
                {
                    for(int y2 = 0; y2 < CHUNKS; ++y2)
                    {
                        QOpenGLShaderProgramPtr shader = mapTiles[x][y].tile->getChunk(x2, y2)->getShader();
                        shader->bind();

                        shader->setUniformValue(name, value);
                    }
                }
            }
        }
    }
}

void World::setChunkShaderUniform(const char* name, const QMatrix4x4& value)
{
    for(int x = 0; x < TILES; ++x)
    {
        for(int y = 0; y < TILES; ++y)
        {
            if(tileLoaded(x, y))
            {
                for(int x2 = 0; x2 < CHUNKS; ++x2)
                {
                    for(int y2 = 0; y2 < CHUNKS; ++y2)
                    {
                        QOpenGLShaderProgramPtr shader = mapTiles[x][y].tile->getChunk(x2, y2)->getShader();
                        shader->bind();

                        shader->setUniformValue(name, value);
                    }
                }
            }
        }
    }
}

void World::setChunkShaderUniform(const char* name, float value)
{
    for(int x = 0; x < TILES; ++x)
    {
        for(int y = 0; y < TILES; ++y)
        {
            if(tileLoaded(x, y))
            {
                for(int x2 = 0; x2 < CHUNKS; ++x2)
                {
                    for(int y2 = 0; y2 < CHUNKS; ++y2)
                    {
                        QOpenGLShaderProgramPtr shader = mapTiles[x][y].tile->getChunk(x2, y2)->getShader();
                        shader->bind();

                        shader->setUniformValue(name, value);
                    }
                }
            }
        }
    }
}

void World::setChunkShaderUniform(const char* name, int value)
{
    for(int x = 0; x < TILES; ++x)
    {
        for(int y = 0; y < TILES; ++y)
        {
            if(tileLoaded(x, y))
            {
                for(int x2 = 0; x2 < CHUNKS; ++x2)
                {
                    for(int y2 = 0; y2 < CHUNKS; ++y2)
                    {
                        QOpenGLShaderProgramPtr shader = mapTiles[x][y].tile->getChunk(x2, y2)->getShader();
                        shader->bind();

                        shader->setUniformValue(name, value);
                    }
                }
            }
        }
    }
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

void World::setDisplayMode(int displayMode)
{
    if(displayMode == SimpleWireFrame || displayMode == WorldHeight || displayMode == WorldTexturedWireframed)
    {
        if(displayMode == WorldTexturedWireframed)
            setChunkShaderUniform("line.width", 0.5f);
        else
            setChunkShaderUniform("line.width", 0.2f);
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
    for(int x = 0; x < TILES; ++x)
    {
        for(int y = 0; y < TILES; ++y)
        {
            if(tileLoaded(x, y))
                mapTiles[x][y].tile->test();
        }
    }
}