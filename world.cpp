#include "world.h"
#include "maptile.h"
#include "mathhelper.h"

World::World(const QString& map_name)
: camera(0)
, mapName(map_name)
, brush(new Brush())
, sunTheta(30.0f)
, eDisplay(TexturedAndLit)
, eDisplaySubroutines(DisplayModeCount)
, GLfuncs(0)
{
    eDisplayNames << QStringLiteral("shadeSimpleWireFrame")
                  << QStringLiteral("shadeWorldHeight")
                  << QStringLiteral("shadeWorldNormal")
                  << QStringLiteral("shadeGrass")
                  << QStringLiteral("shadeGrassAndRocks")
                  << QStringLiteral("shadeGrassRocksAndSnow")
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
}

void World::initialize(QOpenGLContext* context)
{
    GLfuncs = context->versionFunctions<QOpenGLFunctions_4_2_Core>();

    if(!GLfuncs)
    {
        qFatal("Requires OpenGL >= 4.2");
        exit(1);
    }

    GLfuncs->initializeOpenGLFunctions();

    qDebug() << "OpenGL Version: " << QString::fromLocal8Bit((char*)GLfuncs->glGetString(GL_VERSION));
    qDebug() << "OpenGL Vendor:"   << QString::fromLocal8Bit((char*)GLfuncs->glGetString(GL_VENDOR));
    qDebug() << "OpenGL Rendered:" << QString::fromLocal8Bit((char*)GLfuncs->glGetString(GL_RENDERER));

    // prepare Shaders
    material = MaterialPtr(new Material);
    material->setShaders(":/shaders/qeditor.vert",
                         ":/shaders/qeditor.tcs",
                         ":/shaders/qeditor.tes",
                         ":/shaders/qeditor.geom",
                         ":/shaders/qeditor.frag");

    QOpenGLShaderProgramPtr shader = material->shader();
    shader->bind();

    // Get subroutine indices
    for(int i = 0; i < DisplayModeCount; ++i)
        eDisplaySubroutines[i] = GLfuncs->glGetSubroutineIndex(shader->programId(), GL_FRAGMENT_SHADER, eDisplayNames.at(i).toLatin1());

    for(int x = 0; x < TILES; ++x)
    {
        for(int y = 0; y < TILES; ++y)
            mapTiles[x][y].tile = NULL;
    }

    // todo dynamic...
    loadTile(1, 1);
}

void World::update(float dt)
{
    // load tiles around
}

void World::draw(QMatrix4x4 modelMatrix, float triangles, QVector2D mousePosition, bool drawBrush)
{
    material->bind();

    QOpenGLShaderProgramPtr shader = material->shader();
    shader->bind();

    // Set the horizontal and vertical scales applied in the tess eval shader
    shader->setUniformValue("horizontalScale"      , TILESIZE);
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

    for(int x = 0; x < TILES; ++x)
    {
        for(int y = 0; y < TILES; ++y)
        {
            if(tileLoaded(x, y))
                mapTiles[x][y].tile->draw(MAP_DRAW_DISTANCE, camera->position());
        }
    }
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

MapTile* World::loadTile(int x, int y)
{
    if(!hasTile(x, y))
        return NULL;

    if(tileLoaded(x, y))
        return mapTiles[x][y].tile;

    const QString filename(QString("%1_%2_%3.map").arg(mapName).arg(x).arg(y));

    // todo
    // check file in archive
    // load from archive
    mapTiles[x][y].tile = new MapTile(this, x, y, filename);

    return mapTiles[x][y].tile;
}

void World::changeTerrain(float x, float z, float change, int brush_type)
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
                        if(mapTiles[tx][ty].tile->getChunk(cx, cy)->changeTerrain(x, z, change, brush->Radius(), brush->Type(), brush_type))
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

void World::flattenTerrain(float x, float z, float y, float change, int brush_type)
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
                        if(mapTiles[tx][ty].tile->getChunk(cx, cy)->flattenTerrain(x, z, y, change, brush->Radius(), brush->Type(), brush_type))
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

void World::blurTerrain(float x, float z, float change, int brush_type)
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
                        if(mapTiles[tx][ty].tile->getChunk(cx, cy)->blurTerrain(x, z, change, brush->Radius(), brush->Type(), brush_type))
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

void World::paintTerrain()
{
    return;
}

void World::save()
{
    for(int x = 0; x < TILES; ++x)
    {
        for(int y = 0; y < TILES; ++y)
        {
            if(tileLoaded(x, y))
                mapTiles[x][y].tile->saveTile();
        }
    }
}

void World::setCamera(Camera* cam)
{
    camera = cam;
}

void World::setDisplayMode(int displayMode)
{
    if(displayMode == SimpleWireFrame || displayMode == WorldHeight || displayMode == WorldTexturedWireframed)
    {
        QOpenGLShaderProgramPtr shader = material->shader();
        shader->bind();

        if(displayMode == WorldTexturedWireframed)
            shader->setUniformValue("line.width", 0.5f);
        else
            shader->setUniformValue("line.width", 0.2f);
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
