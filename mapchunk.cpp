#include "mapchunk.h"

#include "maptile.h"
#include "qeditor.h"
#include "mathhelper.h"
#include "brush.h"

#include <QImage>
#include <QOpenGLPixelTransferOptions>
#include <QMultiMap>

MapChunk::MapChunk(World* mWorld, MapTile* tile, int x, int y) // Cache MapChunk
: world(mWorld)
, patchBuffer(QOpenGLBuffer::VertexBuffer)
, patchCount(NULL)
, positionData(NULL)
, terrainSampler(tile->terrainSampler.data())
, terrainData(new Texture(QOpenGLTexture::Target2D))
, displaySubroutines(world->DisplayModeCount)
, bottomNeighbour(NULL)
, leftNeighbour(NULL)
, chunkX(x)
, chunkY(y)
, baseX(chunkX * CHUNKSIZE)
, baseY(chunkY * CHUNKSIZE)
, chunkBaseX((tile->coordX * TILESIZE) + baseX)
, chunkBaseY((tile->coordY * TILESIZE) + baseY)
{
    chunkMaterial = new ChunkMaterial();
    chunkMaterial->setShaders(":/shaders/qeditor.vert",
                              ":/shaders/qeditor.tcs",
                              ":/shaders/qeditor.tes",
                              ":/shaders/qeditor.geom",
                              ":/shaders/qeditor.frag");

    /// Textures
    if(!world->getTextureManager()->hasTexture("grassTexture", ""))
        world->getTextureManager()->loadTexture("grassTexture", "grass.png");

    if(!world->getTextureManager()->hasTexture("rockTexture", ""))
        world->getTextureManager()->loadTexture("rockTexture", "rock.png");

    if(!world->getTextureManager()->hasTexture("snowTexture", ""))
        world->getTextureManager()->loadTexture("snowTexture", "snowrocks.png");

    textures[0] = world->getTextureManager()->getTexture("grassTexture");
    textures[1] = world->getTextureManager()->getTexture("rockTexture");
    textures[2] = world->getTextureManager()->getTexture("snowTexture");
    //textures[3] = world->getTextureManager()->getTexture("grassTexture");

    for(int i = 1; i < MAX_TEXTURES + 1; ++i) // GL_TEXTURE1+
    {
        world->getGLFunctions()->glActiveTexture(GL_TEXTURE0 + static_cast<GLuint>(i));

        QString uniformName = QString("layer%1Texture").arg(i - 1);

        if(i == 1)
            uniformName = "baseTexture";

        chunkMaterial->setTextureUnitConfiguration(i, textures[i - 1], world->getTextureManager()->getSampler(), uniformName.toLatin1());
    }

    /// Alphamaps
    for(int i = 0; i < ALPHAMAPS; ++i)
    {
        world->getGLFunctions()->glActiveTexture(GL_TEXTURE4 + static_cast<GLuint>(i));

        alphaMapsData[i] = new unsigned char[MAP_WIDTH / CHUNKS * MAP_HEIGHT / CHUNKS];

        memset(alphaMapsData[i], 0, MAP_WIDTH / CHUNKS * MAP_HEIGHT / CHUNKS);

        TexturePtr alphamap(new Texture(QOpenGLTexture::Target2D));
        alphamap->setSize(MAP_WIDTH / CHUNKS, MAP_HEIGHT / CHUNKS);
        alphamap->setAlphamap(alphaMapsData[i]);

        alphaMaps[i] = alphamap;

        QString alphaMapLayer = QString("layer%1Alpha").arg(i + 1);

        chunkMaterial->setTextureUnitConfiguration(4 + i, alphaMaps[i], terrainSampler, alphaMapLayer.toLatin1());
    }

    /// Terrain
    mapData = new float[CHUNK_ARRAY_SIZE];

    // fill empty terrain
    for(int i = 0; i < CHUNK_ARRAY_SIZE; ++i)
        mapData[i] = 0.0f;

    world->getGLFunctions()->glActiveTexture(GL_TEXTURE0);

    terrainData->setSize(MAP_WIDTH / CHUNKS, MAP_HEIGHT / CHUNKS);
    terrainData->setHeightmap(mapData);

    chunkMaterial->setTextureUnitConfiguration(0, terrainData, terrainSampler, QByteArrayLiteral("heightMap"));

    initialize();
}

MapChunk::MapChunk(World* mWorld, MapTile* tile, QFile& file, int x, int y) // File based MapChunk
: world(mWorld)
, patchBuffer(QOpenGLBuffer::VertexBuffer)
, patchCount(NULL)
, positionData(NULL)
, terrainSampler(tile->terrainSampler.data())
, terrainData(new Texture(QOpenGLTexture::Target2D))
, displaySubroutines(world->DisplayModeCount)
, bottomNeighbour(NULL)
, leftNeighbour(NULL)
, chunkX(x)
, chunkY(y)
, baseX(chunkX * CHUNKSIZE)
, baseY(chunkY * CHUNKSIZE)
, chunkBaseX((tile->coordX * TILESIZE) + baseX)
, chunkBaseY((tile->coordY * TILESIZE) + baseY)
{
    chunkMaterial = new ChunkMaterial();
    chunkMaterial->setShaders(":/shaders/qeditor.vert",
                              ":/shaders/qeditor.tcs",
                              ":/shaders/qeditor.tes",
                              ":/shaders/qeditor.geom",
                              ":/shaders/qeditor.frag");
    //chunkMaterial->link();

    /// Textures // todo dynamic from mapfile
    if(!world->getTextureManager()->hasTexture("grassTexture", ""))
        world->getTextureManager()->loadTexture("grassTexture", "grass.png");

    if(!world->getTextureManager()->hasTexture("rockTexture", ""))
        world->getTextureManager()->loadTexture("rockTexture", "rock.png");

    if(!world->getTextureManager()->hasTexture("snowTexture", ""))
        world->getTextureManager()->loadTexture("snowTexture", "snowrocks.png");

    textures[0] = world->getTextureManager()->getTexture("grassTexture");
    textures[1] = world->getTextureManager()->getTexture("rockTexture");
    textures[2] = world->getTextureManager()->getTexture("snowTexture");
    textures[3] = TexturePtr(new Texture());
    //textures[3] = world->getTextureManager()->getTexture("grassTexture");

    for(int i = 1; i < MAX_TEXTURES + 1; ++i) // GL_TEXTURE1+
    {
        world->getGLFunctions()->glActiveTexture(GL_TEXTURE0 + static_cast<GLuint>(i));

        QString uniformName = QString("layer%1Texture").arg(i - 1);

        if(i == 1)
            uniformName = "baseTexture";

        chunkMaterial->setTextureUnitConfiguration(i, textures[i - 1], world->getTextureManager()->getSampler(), uniformName.toLatin1());
    }

    /// Alphamaps
    for(int i = 0; i < ALPHAMAPS; ++i)
    {
        world->getGLFunctions()->glActiveTexture(GL_TEXTURE4 + static_cast<GLuint>(i));

        // todo load from file
        alphaMapsData[i] = new unsigned char[MAP_WIDTH / CHUNKS * MAP_HEIGHT / CHUNKS];

        memset(alphaMapsData[i], 0, MAP_WIDTH / CHUNKS * MAP_HEIGHT / CHUNKS);

        TexturePtr alphamap(new Texture(QOpenGLTexture::Target2D));
        alphamap->setSize(MAP_WIDTH / CHUNKS, MAP_HEIGHT / CHUNKS);
        alphamap->setAlphamap(alphaMapsData[i]);

        alphaMaps[i] = alphamap;

        QString alphaMapLayer = QString("layer%1Alpha").arg(i + 1);

        chunkMaterial->setTextureUnitConfiguration(4 + i, alphaMaps[i], terrainSampler, alphaMapLayer.toLatin1());
    }

    /// Terrain
    // Load from file
    if(file.isOpen())
        mapData = tile->getHeader().mcin->entries[chunkIndex()].mcnk->heightOffset->height;
    else
    {
        mapData = new float[CHUNK_ARRAY_SIZE];

        for(int i = 0; i < CHUNK_ARRAY_SIZE; ++i)
            mapData[i] = 0.0f;
    }

    world->getGLFunctions()->glActiveTexture(GL_TEXTURE0);

    terrainData->setSize(MAP_WIDTH / CHUNKS, MAP_HEIGHT / CHUNKS);
    terrainData->setHeightmap(mapData);

    chunkMaterial->setTextureUnitConfiguration(0, terrainData, terrainSampler, QByteArrayLiteral("heightMap"));

    initialize();
}

MapChunk::~MapChunk()
{
    delete[] mapData;

    mapData = NULL;

    patchBuffer.destroy();
    vao.destroy();
}

void MapChunk::initialize()
{
    /// Create a Vertex Buffers
    const int maxTessellationLevel     = 64;
    const int trianglesPerHeightSample = 10;

    const int xDivisions = trianglesPerHeightSample * MAP_WIDTH  / CHUNKS / maxTessellationLevel;
    const int zDivisions = trianglesPerHeightSample * MAP_HEIGHT / CHUNKS / maxTessellationLevel;

    patchCount = xDivisions * zDivisions;

    positionData.resize(2 * patchCount); // 2 floats per vertex

    qDebug() << QObject::tr("Total number of patches for mapchunk =") << patchCount;

    const float dx = 1.0f / MathHelper::toFloat(xDivisions);
    const float dz = 1.0f / MathHelper::toFloat(zDivisions);

    for(int j = 0; j < 2 * zDivisions; j += 2)
    {
        float z = MathHelper::toFloat(j) * dz * 0.5;

        for(int i = 0; i < 2 * xDivisions; i += 2)
        {
            float x         = MathHelper::toFloat(i) * dx * 0.5;
            const int index = xDivisions * j + i;

            positionData[index]     = x;
            positionData[index + 1] = z;
        }
    }

    patchBuffer.create();
    patchBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    patchBuffer.bind();
    patchBuffer.allocate(positionData.data(), positionData.size() * sizeof(float));
    patchBuffer.release();

    /// Create a VAO for this "object"
    vao.create();
    {
        QOpenGLVertexArrayObject::Binder binder(&vao);
        QOpenGLShaderProgramPtr shader = chunkMaterial->shader();

        shader->bind();
        patchBuffer.bind();

        shader->enableAttributeArray("vertexPosition");
        shader->setAttributeBuffer("vertexPosition", GL_FLOAT, 0, 2);
    }

    /// Default shader values
    QOpenGLShaderProgramPtr shader2 = chunkMaterial->shader();
    shader2->bind();

    // Get subroutine indices
    for(int i = 0; i < world->DisplayModeCount; ++i)
        displaySubroutines[i] = world->getGLFunctions()->glGetSubroutineIndex(shader2->programId(), GL_FRAGMENT_SHADER, world->displayName(i).toLatin1());

    shader2->setUniformValue("line.width", 0.2f);
    shader2->setUniformValue("line.color", QVector4D(0.17f, 0.50f, 1.0f, 1.0f)); // blue

    shader2->setUniformValue("baseX", chunkBaseX);
    shader2->setUniformValue("baseY", chunkBaseY);

    shader2->setUniformValue("chunkX", chunkX);
    shader2->setUniformValue("chunkY", chunkY);

    shader2->setUniformValue("chunkLines", app().getSetting("chunkLines", false).toBool());

    shader2->setUniformValue("textureScaleOption", app().getSetting("textureScaleOption", 0).toInt());
    shader2->setUniformValue("textureScaleFar",    app().getSetting("textureScaleFar",    0.4f).toFloat());
    shader2->setUniformValue("textureScaleNear",   app().getSetting("textureScaleNear",   0.4f).toFloat());

    // Set the fog parameters
    shader2->setUniformValue("fog.color"      , QVector4D(0.65f, 0.77f, 1.0f, 1.0f));
    shader2->setUniformValue("fog.minDistance", app().getSetting("environmentDistance", 256.0f).toFloat() / 2.0f);
    shader2->setUniformValue("fog.maxDistance", app().getSetting("environmentDistance", 256.0f).toFloat() - 32.0f);
}

void MapChunk::test()
{
    /*terrainData->bind();

    for(int j = 0; j < 1; ++j)
    {
        for(int i = 0; i < MAP_HEIGHT / CHUNKS; ++i)
            terrainData->updatePixel(1.0f, QVector2D(j, i));
    }

    chunkMaterial->setTextureUnitConfiguration(0, terrainData, terrainSampler, QByteArrayLiteral("heightMap"));*/
}

void MapChunk::draw()
{
    chunkMaterial->bind();

    QOpenGLShaderProgramPtr shader = chunkMaterial->shader();
    shader->bind();

    // Set the fragment shader display mode subroutine
    world->getGLFunctions()->glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &getDisplaySubroutines());

    if(world->displayMode() != world->Hidden)
    {
        // Render the quad as a patch
        {
            QOpenGLVertexArrayObject::Binder binder(&vao);
            shader->setPatchVertexCount(1);

            world->getGLFunctions()->glDrawArrays(GL_PATCHES, 0, patchCount);
        }
    }
}

bool MapChunk::isInVisibleRange(const float& distance, const QVector3D& camera) const
{
    static const float chunkRadius = sqrtf(CHUNKSIZE * CHUNKSIZE / 2.0f);

    return (camera.length() - chunkRadius) < distance;
}

const float MapChunk::getHeight(const float& x, const float& y) const
{
    int X = MathHelper::toInt(x) % MAP_WIDTH;
    int Y = MathHelper::toInt(y) % MAP_HEIGHT;

    int index = (Y * MAP_WIDTH / CHUNKS) + X;

    return mapData[index * sizeof(float)];
}

const float MapChunk::getHeight(const int& x, const int& y) const
{
    int X = x % MAP_WIDTH;
    int Y = y % MAP_HEIGHT;

    int index = (Y * MAP_WIDTH / CHUNKS) + X;

    return mapData[index * sizeof(float)];
}

const float MapChunk::getMapData(const int& index) const
{
    return mapData[index];
}

const int MapChunk::chunkIndex() const
{
    return (chunkY * CHUNKS) + chunkX;
}

bool MapChunk::changeTerrain(float x, float z, float change, const Brush* brush)
{    
    bool changed = false;

    float xdiff = baseX - x + CHUNKSIZE / 2;
    float ydiff = baseY - z + CHUNKSIZE / 2;

    float dist = sqrt(pow(xdiff, 2) + pow(ydiff, 2));

    if(dist > (brush->OuterRadius() + CHUNK_DIAMETER))
        return changed;

    QList<QPair<float, QVector2D>> changing;

    int minX = horizToHMapSize(x - brush->OuterRadius());
    int maxX = horizToHMapSize(x + brush->OuterRadius());

    int minY = horizToHMapSize(z - brush->OuterRadius());
    int maxY = horizToHMapSize(z + brush->OuterRadius());

    for(int _x = minX; _x < maxX; ++_x)
    {
        for(int _y = minY; _y < maxY; ++_y) // _y mean z!
        {
            switch(brush->Shape())
            {
                case Brush::Circle: // Circle
                default:
                    {
                        xdiff = HMapSizeToHoriz(_x) - x;
                        ydiff = HMapSizeToHoriz(_y) - z;

                        dist = sqrt(pow(xdiff, 2) + pow(ydiff, 2));

                        if(dist < brush->OuterRadius())
                        {
                            int X = _x % MAP_WIDTH;
                            int Y = _y % MAP_HEIGHT;

                            X -= chunkX * (MAP_WIDTH / CHUNKS);
                            Y -= chunkY * (MAP_HEIGHT / CHUNKS);

                            int index = (Y * (MAP_WIDTH / CHUNKS)) + X;

                            if(index < 0 || index >= CHUNK_ARRAY_SIZE || X < 0 || Y < 0 || X >= (MAP_WIDTH / CHUNKS) || Y >= (MAP_HEIGHT / CHUNKS))
                                continue;

                            index *= sizeof(float);

                            switch(brush->BrushTypes().shaping)
                            {
                                case Brush::ShapingType::Linear:
                                default:
                                    {
                                        float changeFormula = change * (1.0f - dist / brush->OuterRadius());
                                        //float changeFormula = change * (1.0f - dist / brush->OuterRadius() * (1.0f / brush->InnerRadius()));

                                        mapData[index] += changeFormula;
                                    }
                                    break;

                                case Brush::ShapingType::Flat:
                                    {
                                        float changeFormula = change;

                                        mapData[index] += changeFormula;
                                    }
                                    break;

                                case Brush::ShapingType::Smooth:
                                    {
                                        float changeFormula = change / (1.0f + dist / brush->OuterRadius());

                                       mapData[index] += changeFormula;
                                    }
                                    break;

                                case Brush::ShapingType::Polynomial:
                                    {
                                        float changeFormula = change * ((dist / brush->OuterRadius()) * (dist / brush->OuterRadius()) + dist / brush->OuterRadius() + 1.0f);

                                        mapData[index] += changeFormula;
                                    }
                                    break;

                                case Brush::ShapingType::Trigonometric:
                                    {
                                        float changeFormula = change * cos(dist / brush->OuterRadius());

                                        mapData[index] += changeFormula;
                                    }
                                    break;
                            }

                            changing.append(qMakePair<float, QVector2D>(mapData[index], QVector2D(X, Y)));

                            changed = true;
                        }
                    }
                    break;
            }
        }
    }

    if(changed)
    {
        QVector<QPair<int, float>> horizontalData;
        QVector<QPair<int, float>> verticalData;

        terrainData->bind();

        for(int i = 0; i < changing.count(); ++i)
        {
            if(changing.value(i).second.y() == 0)
                horizontalData.append(QPair<int, float>(changing.value(i).second.x(), changing.value(i).first));

            if(changing.value(i).second.x() == 0)
                verticalData.append(QPair<int, float>(changing.value(i).second.y(), changing.value(i).first));

            terrainData->setHeight(changing.value(i).first, changing.value(i).second);
        }

        chunkMaterial->setTextureUnitConfiguration(0, terrainData, terrainSampler, QByteArrayLiteral("heightMap"));

        if(bottomNeighbour && !horizontalData.isEmpty())
            bottomNeighbour->setBorder(Horizontal, horizontalData);

        if(leftNeighbour && !verticalData.isEmpty())
            leftNeighbour->setBorder(Vertical, verticalData);
    }

    return changed;
}

bool MapChunk::flattenTerrain(float x, float z, float y, float change, const Brush* brush)
{
    bool changed = false;

    float xdiff = baseX - x + CHUNKSIZE / 2;
    float ydiff = baseY - z + CHUNKSIZE / 2;

    float dist = sqrt(pow(xdiff, 2) + pow(ydiff, 2));

    if(dist > (brush->OuterRadius() + CHUNK_DIAMETER))
        return changed;

    QList<QPair<float, QVector2D>> changing;

    int minX = horizToHMapSize(x - brush->OuterRadius());
    int maxX = horizToHMapSize(x + brush->OuterRadius());

    int minY = horizToHMapSize(z - brush->OuterRadius());
    int maxY = horizToHMapSize(z + brush->OuterRadius());

    for(int _x = minX; _x < maxX; ++_x)
    {
        for(int _y = minY; _y < maxY; ++_y) // _y mean z!
        {
            switch(brush->Shape())
            {
                case Brush::Circle:
                default:
                    {
                        xdiff = HMapSizeToHoriz(_x) - x;
                        ydiff = HMapSizeToHoriz(_y) - z;

                        dist = sqrt(pow(xdiff, 2) + pow(ydiff, 2));

                        if(dist < brush->OuterRadius())
                        {
                            int X = _x % MAP_WIDTH;
                            int Y = _y % MAP_HEIGHT;

                            X -= chunkX * (MAP_WIDTH / CHUNKS);
                            Y -= chunkY * (MAP_HEIGHT / CHUNKS);

                            int index = (Y * (MAP_WIDTH / CHUNKS)) + X;

                            if(index < 0 || index >= CHUNK_ARRAY_SIZE || X < 0 || Y < 0 || X >= (MAP_WIDTH / CHUNKS) || Y >= (MAP_HEIGHT / CHUNKS))
                                continue;

                            index *= sizeof(float);

                            switch(brush->BrushTypes().smoothing)
                            {
                                case Brush::SmoothingType::Linear:
                                default:
                                    {
                                        float changeFormula = 1.0f - (1.0f - change) * (1.0f - dist / brush->OuterRadius());

                                        mapData[index] = changeFormula * mapData[index] + (1 - changeFormula) * y;
                                    }
                                    break;

                                case Brush::SmoothingType::Flat:
                                        mapData[index] = change * mapData[index] + (1 - change) * y;
                                    break;

                                case Brush::SmoothingType::Smooth:
                                    {
                                        float changeFormula = 1.0f - pow(1.0f - change, 1.0f + dist / brush->OuterRadius());

                                        mapData[index] = changeFormula * mapData[index] + (1 - changeFormula) * y;
                                    }
                                    break;
                            }

                            changing.append(qMakePair<float, QVector2D>(mapData[index], QVector2D(X, Y)));

                            changed = true;
                        }
                    }
                    break;
            }
        }
    }

    if(changed)
    {
        QVector<QPair<int, float>> horizontalData;
        QVector<QPair<int, float>> verticalData;

        terrainData->bind();

        for(int i = 0; i < changing.count(); ++i)
        {
            if(changing.value(i).second.y() == 0)
                horizontalData.append(QPair<int, float>(changing.value(i).second.x(), changing.value(i).first));

            if(changing.value(i).second.x() == 0)
                verticalData.append(QPair<int, float>(changing.value(i).second.y(), changing.value(i).first));

            terrainData->setHeight(changing.value(i).first, changing.value(i).second);
        }

        chunkMaterial->setTextureUnitConfiguration(0, terrainData, terrainSampler, QByteArrayLiteral("heightMap"));

        if(bottomNeighbour && !horizontalData.isEmpty())
            bottomNeighbour->setBorder(Horizontal, horizontalData);

        if(leftNeighbour && !verticalData.isEmpty())
            leftNeighbour->setBorder(Vertical, verticalData);
    }

    return changed;
}

bool MapChunk::blurTerrain(float x, float z, float change, const Brush* brush)
{
    bool changed = false;

    float xdiff = baseX - x + CHUNKSIZE / 2;
    float ydiff = baseY - z + CHUNKSIZE / 2;

    float dist = sqrt(pow(xdiff, 2) + pow(ydiff, 2));

    if(dist > (brush->OuterRadius() + CHUNK_DIAMETER))
        return changed;

    QList<QPair<float, QVector2D>> changing;

    int minX = horizToHMapSize(x - brush->OuterRadius());
    int maxX = horizToHMapSize(x + brush->OuterRadius());

    int minY = horizToHMapSize(z - brush->OuterRadius());
    int maxY = horizToHMapSize(z + brush->OuterRadius());

    for(int _x = minX; _x < maxX; ++_x)
    {
        for(int _y = minY; _y < maxY; ++_y) // _y mean z!
        {
            switch(brush->Shape())
            {
                case Brush::Circle:
                default:
                    {
                        xdiff = HMapSizeToHoriz(_x) - x;
                        ydiff = HMapSizeToHoriz(_y) - z;

                        dist = sqrt(pow(xdiff, 2) + pow(ydiff, 2));

                        if(dist < brush->OuterRadius())
                        {
                            int X = _x % MAP_WIDTH;
                            int Y = _y % MAP_HEIGHT;

                            X -= chunkX * (MAP_WIDTH / CHUNKS);
                            Y -= chunkY * (MAP_HEIGHT / CHUNKS);

                            int index = (Y * (MAP_WIDTH / CHUNKS)) + X;

                            if(index < 0 || index >= CHUNK_ARRAY_SIZE || X < 0 || Y < 0 || X >= (MAP_WIDTH / CHUNKS) || Y >= (MAP_HEIGHT / CHUNKS))
                                continue;

                            index *= sizeof(float);

                            float TotalHeight, TotalWidth, tx, tz, h, dist2;

                            TotalHeight = 0;
                            TotalWidth  = 0;

                            int Rad = (brush->OuterRadius() / UNITSIZE);

                            for(int j = -Rad * 2; j <= Rad * 2; ++j)
                            {
                                tz = z + j * UNITSIZE / 2;

                                for(int k = -Rad; k <= Rad; ++k)
                                {
                                    tx = x + k * UNITSIZE + (j % 2) * UNITSIZE / 2.0f;

                                    xdiff = tx - HMapSizeToHoriz(_x);
                                    ydiff = tz - HMapSizeToHoriz(_y);

                                    dist2 = sqrt(pow(xdiff, 2) + pow(ydiff, 2));

                                    if(dist2 > brush->OuterRadius())
                                        continue;

                                    int X2 = MathHelper::toInt(tx) % MAP_WIDTH;
                                    int Y2 = MathHelper::toInt(tz) % MAP_HEIGHT;

                                    X2 -= chunkX * (MAP_WIDTH / CHUNKS);
                                    Y2 -= chunkY * (MAP_HEIGHT / CHUNKS);

                                    int index2 = (Y2 * (MAP_WIDTH / CHUNKS)) + X2;

                                    index2 *= sizeof(float);

                                    const float height = mapData[index2];

                                    if(height)
                                    {
                                        TotalHeight += (1.0f - dist2 / brush->OuterRadius()) * height;
                                        TotalWidth  += (1.0f - dist2 / brush->OuterRadius());
                                    }
                                }
                            }

                            h = TotalHeight / TotalWidth;

                            switch(brush->BrushTypes().smoothing)
                            {
                                case Brush::SmoothingType::Linear: // Linear
                                default:
                                    {
                                        float changeFormula = 1.0f - (1.0f - change) * (1.0f - dist / brush->OuterRadius());

                                        mapData[index] = changeFormula * mapData[index] + (1 - changeFormula) * h;
                                    }
                                    break;

                                case Brush::SmoothingType::Flat: // Flat
                                        mapData[index] = change * mapData[index] + (1 - change) * h;
                                    break;

                                case Brush::SmoothingType::Smooth: // Smooth
                                    {
                                        float changeFormula = 1.0f - pow(1.0f - change, (1.0f + dist / brush->OuterRadius()));

                                        mapData[index] = changeFormula * mapData[index] + (1 - changeFormula) * h;
                                    }
                                    break;
                            }

                            changing.append(qMakePair<float, QVector2D>(mapData[index], QVector2D(X, Y)));

                            changed = true;
                        }
                    }
                    break;
            }
        }
    }

    if(changed)
    {
        QVector<QPair<int, float>> horizontalData;
        QVector<QPair<int, float>> verticalData;

        terrainData->bind();

        for(int i = 0; i < changing.count(); ++i)
        {
            if(changing.value(i).second.y() == 0)
                horizontalData.append(QPair<int, float>(changing.value(i).second.x(), changing.value(i).first));

            if(changing.value(i).second.x() == 0)
                verticalData.append(QPair<int, float>(changing.value(i).second.y(), changing.value(i).first));

            terrainData->setHeight(changing.value(i).first, changing.value(i).second);
        }

        chunkMaterial->setTextureUnitConfiguration(0, terrainData, terrainSampler, QByteArrayLiteral("heightMap"));

        if(bottomNeighbour && !horizontalData.isEmpty())
            bottomNeighbour->setBorder(Horizontal, horizontalData);

        if(leftNeighbour && !verticalData.isEmpty())
            leftNeighbour->setBorder(Vertical, verticalData);
    }

    return changed;
}

bool MapChunk::paintTerrain(float x, float z, float flow, const Brush* brush, TexturePtr texture)
{
    bool changed = false;

    int textureIndex = -1;

    float xdiff = baseX - x + CHUNKSIZE / 2;
    float ydiff = baseY - z + CHUNKSIZE / 2;

    float dist = sqrt(pow(xdiff, 2) + pow(ydiff, 2));

    if(dist > (brush->OuterRadius() + CHUNK_DIAMETER))
        return changed;

    // Search for texture index
    for(int i = 0; i < MAX_TEXTURES; ++i)
    {
        if(textures[i]->isNull())
            continue;

        if(textures[i]->getPath() == texture->getPath())
        {
            textureIndex = i;

            break;
        }
    }

    // Search for empty slot
    if(textureIndex == -1)
    {
        bool full = true;

        for(int i = 0; i < MAX_TEXTURES; ++i)
        {
            if(textures[i]->isNull())
            {
                textureIndex = i;

                full = false;

                textures[i] = texture;

                break;
            }
        }

        if(full)
        {
            //qDebug() << "todo ..";

            return changed;
        }
    }

    QList<QPair<unsigned char, QVector2D>>          changing;
    QMultiMap<int, QPair<unsigned char, QVector2D>> changing2;

    int minX = horizToHMapSize(x - brush->OuterRadius());
    int maxX = horizToHMapSize(x + brush->OuterRadius());

    int minY = horizToHMapSize(z - brush->OuterRadius());
    int maxY = horizToHMapSize(z + brush->OuterRadius());

    for(int _x = minX; _x < maxX; ++_x)
    {
        for(int _y = minY; _y < maxY; ++_y) // _y mean z!
        {
            switch(brush->Shape())
            {
                case Brush::Circle:
                default:
                    {
                        xdiff = HMapSizeToHoriz(_x) - x;
                        ydiff = HMapSizeToHoriz(_y) - z;

                        dist = sqrt(pow(xdiff, 2) + pow(ydiff, 2));

                        if(dist < brush->OuterRadius())
                        {
                            int X = _x % MAP_WIDTH;
                            int Y = _y % MAP_HEIGHT;

                            X -= chunkX * (MAP_WIDTH / CHUNKS);
                            Y -= chunkY * (MAP_HEIGHT / CHUNKS);

                            int index = (Y * (MAP_WIDTH / CHUNKS)) + X;

                            if(index < 0 || index >= CHUNK_ARRAY_UB_SIZE || X < 0 || Y < 0 || X >= (MAP_WIDTH / CHUNKS) || Y >= (MAP_HEIGHT / CHUNKS))
                                continue;

                            switch(brush->BrushTypes().texturing)
                            {
                                case Brush::TexturingType::Solid:
                                default:
                                    {
                                        if(textureIndex > 0)
                                        {
                                            --textureIndex;

                                            //alphaMapsData[textureIndex][index] = ToUChar(qMax(qMin((1.0f - flow) * ToFloat(alphaMapsData[textureIndex][index]) + flow + 0.5f, 255.0f), 0.0f));
                                            //alphaMapsData[textureIndex][index] = ToUChar(qMax(qMin((1.0f - flow) * ToFloat(alphaMapsData[textureIndex][index]) + (1.0f - flow) + 0.5f, 255.0f), 0.0f));
                                            alphaMapsData[textureIndex][index] = ToUChar(qMax(qMin(ToFloat(alphaMapsData[textureIndex][index]) + 2.0f * flow, 255.0f), 0.0f));

                                            //alphaMapsData[textureIndex][index] = ToUChar(qMax(qMin(ToFloat(alphaMapsData[textureIndex][index] + 2.0f), 255.0f), 0.0f));

                                            changing.append(qMakePair<unsigned char, QVector2D>(alphaMapsData[textureIndex][index], QVector2D(X, Y)));

                                            ++textureIndex;
                                        }

                                        for(int i = textureIndex; i < ALPHAMAPS; ++i)
                                        {
                                            if(ToFloat(alphaMapsData[i][index]) > 0.0f)
                                            {
                                                alphaMapsData[i][index] = ToUChar(qMax(qMin(ToFloat(alphaMapsData[i][index]) - 2.0f * flow, 255.0f), 0.0f));

                                                changing2.insert(i, qMakePair<unsigned char, QVector2D>(alphaMapsData[i][index], QVector2D(X, Y)));
                                            }
                                        }
                                    }
                                    break;
                            }

                            changed = true;
                        }
                    }
                    break;
            }
        }
    }

    if(changed)
    {
        if(textureIndex > 0)
        {
            --textureIndex;

            alphaMaps[textureIndex]->bind();

            for(int i = 0; i < changing.count(); ++i)
                alphaMaps[textureIndex]->setAlpha(changing.value(i).first, changing.value(i).second);

            QString uniformName = QString("layer%1Alpha").arg(textureIndex + 1);

            chunkMaterial->setTextureUnitConfiguration(4 + textureIndex, alphaMaps[textureIndex], terrainSampler, uniformName.toLatin1());

            ++textureIndex;
        }

        for(int i = textureIndex; i < ALPHAMAPS; ++i)
        {
            changing = changing2.values(i);

            if(changing.count() <= 0)
                continue;

            alphaMaps[i]->bind();

            for(int j = 0; j < changing.count(); ++j)
                alphaMaps[i]->setAlpha(changing.value(j).first, changing.value(j).second);

            QString uniformName = QString("layer%1Alpha").arg(i + 1);

            chunkMaterial->setTextureUnitConfiguration(4 + i, alphaMaps[i], terrainSampler, uniformName.toLatin1());
        }
    }

    return changed;
}

int MapChunk::horizToHMapSize(float position)
{
    return MathHelper::toInt(position / TILESIZE * MathHelper::toFloat(MAP_WIDTH));
}

float MapChunk::HMapSizeToHoriz(int position)
{
    return MathHelper::toFloat(position) / MathHelper::toFloat(MAP_WIDTH) * TILESIZE;
}

void MapChunk::moveAlphaMap(int index, bool up)
{
    QString uniformName;

    switch(up)
    {
        case true:
            {
                TexturePtr temp = textures[index - 1];

                textures[index - 1] = textures[index];
                textures[index]     = temp;

                world->getGLFunctions()->glActiveTexture(GL_TEXTURE1 + static_cast<GLuint>(index - 1));

                uniformName = QString("layer%1Texture").arg(index - 1);

                if(index - 1 == 0)
                    uniformName = "baseTexture";

                chunkMaterial->setTextureUnitConfiguration(index, textures[index - 1], world->getTextureManager()->getSampler(), uniformName.toLatin1());
            }
            break;

        case false:
            {
                TexturePtr temp = textures[index + 1];

                textures[index + 1] = textures[index];
                textures[index]     = temp;

                world->getGLFunctions()->glActiveTexture(GL_TEXTURE1 + static_cast<GLuint>(index + 1));

                uniformName = QString("layer%1Texture").arg(index + 1);

                chunkMaterial->setTextureUnitConfiguration(index + 2, textures[index + 1], world->getTextureManager()->getSampler(), uniformName.toLatin1());
            }
            break;
    }

    world->getGLFunctions()->glActiveTexture(GL_TEXTURE1 + static_cast<GLuint>(index));

    uniformName = QString("layer%1Texture").arg(index);

    if(index == 0)
        uniformName = "baseTexture";

    chunkMaterial->setTextureUnitConfiguration(index + 1, textures[index], world->getTextureManager()->getSampler(), uniformName.toLatin1());
}

void MapChunk::deleteAlphaMap(int index)
{
    textures[index] = TexturePtr(new Texture());

    memset(alphaMapsData[index], 0, MAP_WIDTH / CHUNKS * MAP_HEIGHT / CHUNKS); // clear alpha map

    alphaMaps[index]->setAlphamap(alphaMapsData[index]);

    QString uniformName = QString("layer%1Alpha").arg(index);

    chunkMaterial->setTextureUnitConfiguration(4 + index, alphaMaps[index], terrainSampler, uniformName.toLatin1());
}

void MapChunk::setBorder(Border border, const QVector<QPair<int, float>>& data)
{
    terrainData->bind();

    switch(border)
    {
        case Horizontal:
            {
                for(int i = 0; i < data.count(); ++i)
                {
                    terrainData->setHeight(data.at(i).second, QVector2D(data.at(i).first, MAP_HEIGHT / CHUNKS - 1));

                    int index = (((MAP_HEIGHT / CHUNKS - 1) * (MAP_WIDTH / CHUNKS)) + data.at(i).first) * sizeof(float);

                    mapData[index] = data.at(i).second;
                }
            }
            break;

        case Vertical:
            {
                for(int i = 0; i < data.count(); ++i)
                {
                    terrainData->setHeight(data.at(i).second, QVector2D(MAP_HEIGHT / CHUNKS - 1, data.at(i).first));

                    int index = ((data.at(i).first * (MAP_WIDTH / CHUNKS)) + (MAP_HEIGHT / CHUNKS - 1)) * sizeof(float);

                    mapData[index] = data.at(i).second;
                }
            }
            break;
    }

    chunkMaterial->setTextureUnitConfiguration(0, terrainData, terrainSampler, QByteArrayLiteral("heightMap"));
}

void MapChunk::setBottomNeighbour(MapChunk* chunk)
{
    bottomNeighbour = chunk;
}

void MapChunk::setLeftNeighbour(MapChunk* chunk)
{
    leftNeighbour = chunk;
}

void MapChunk::save(MCNK* chunk)
{
    chunk->areaID  = 0;
    chunk->doodads = 0;
    chunk->flags   = 0;
    chunk->indexX  = chunkX;
    chunk->indexY  = chunkY;
    chunk->layers  = 0;

    chunk->heightOffset = new MCVT;

    for(int i = 0; i < CHUNK_ARRAY_SIZE; i++)
        chunk->heightOffset->height[i] = mapData[i];
}