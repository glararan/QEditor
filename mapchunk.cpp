#include "mapchunk.h"

#include "maptile.h"
#include "qeditor.h"
#include "mathhelper.h"

#include <QImage>

MapChunk::MapChunk(World* mWorld, MapTile* tile, int x, int y) // Cache MapChunk
: world(mWorld)
, patchBuffer(QOpenGLBuffer::VertexBuffer)
, patchCount(0)
, positionData(0)
, terrainSampler(tile->terrainSampler.data())
, terrainData(new Texture(MAP_WIDTH / CHUNKS, MAP_HEIGHT / CHUNKS))
, displaySubroutines(world->DisplayModeCount)
, broadcast(NULL)
, chunkX(x)
, chunkY(y)
, baseX(chunkX * CHUNKSIZE)
, baseY(chunkY * CHUNKSIZE)
, chunkBaseX((tile->coordX * TILESIZE) + baseX)
, chunkBaseY((tile->coordY * TILESIZE) + baseY)
{
    chunkMaterial = ChunkMaterialPtr(new ChunkMaterial);
    chunkMaterial->setShaders(":/shaders/qeditor.vert",
                              ":/shaders/qeditor.tcs",
                              ":/shaders/qeditor.tes",
                              ":/shaders/qeditor.geom",
                              ":/shaders/qeditor.frag");

    SamplerPtr tilingSampler(new Sampler);
    tilingSampler->create();
    tilingSampler->setMinificationFilter(GL_LINEAR_MIPMAP_LINEAR);
    world->getGLFunctions()->glSamplerParameterf(tilingSampler->samplerId(), GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f);
    tilingSampler->setMagnificationFilter(GL_LINEAR);
    tilingSampler->setWrapMode(Sampler::DirectionS, GL_REPEAT);
    tilingSampler->setWrapMode(Sampler::DirectionT, GL_REPEAT);

    QImage grassImage("grass.png");
    world->getGLFunctions()->glActiveTexture(GL_TEXTURE1);

    TexturePtr grassTexture(new Texture);
    grassTexture->create();
    grassTexture->bind();
    grassTexture->setImage(grassImage);
    grassTexture->generateMipMaps();

    chunkMaterial->setTextureUnitConfiguration(1, grassTexture, tilingSampler, QByteArrayLiteral("grassTexture"));

    QImage rockImage("rock.png");
    world->getGLFunctions()->glActiveTexture(GL_TEXTURE2);

    TexturePtr rockTexture(new Texture);
    rockTexture->create();
    rockTexture->bind();
    rockTexture->setImage(rockImage);
    rockTexture->generateMipMaps();

    chunkMaterial->setTextureUnitConfiguration(2, rockTexture, tilingSampler, QByteArrayLiteral("rockTexture"));

    QImage snowImage("snowrocks.png");
    world->getGLFunctions()->glActiveTexture(GL_TEXTURE3);

    TexturePtr snowTexture(new Texture);
    snowTexture->create();
    snowTexture->bind();
    snowTexture->setImage(snowImage);
    snowTexture->generateMipMaps();

    chunkMaterial->setTextureUnitConfiguration(3, snowTexture, tilingSampler, QByteArrayLiteral("snowTexture"));

    world->getGLFunctions()->glActiveTexture(GL_TEXTURE0);

    mapData = new float[CHUNK_ARRAY_SIZE];

    // fill empty terrain
    for(int i = 0; i < CHUNK_ARRAY_SIZE; ++i)
        mapData[i] = 0.0f;

    terrainData->create();
    terrainData->bind();
    terrainData->setImageAlpha(mapData);

    chunkMaterial->setTextureUnitConfiguration(0, terrainData, terrainSampler, QByteArrayLiteral("heightMap"));

    initialize();
}

MapChunk::MapChunk(World* mWorld, MapTile* tile, QFile& file, int x, int y) // File based MapChunk
: world(mWorld)
, patchBuffer(QOpenGLBuffer::VertexBuffer)
, patchCount(0)
, positionData(0)
, terrainSampler(tile->terrainSampler.data())
, terrainData(new Texture(MAP_WIDTH / CHUNKS, MAP_HEIGHT / CHUNKS))
, displaySubroutines(world->DisplayModeCount)
, broadcast(NULL)
, chunkX(x)
, chunkY(y)
, baseX(chunkX * CHUNKSIZE)
, baseY(chunkY * CHUNKSIZE)
, chunkBaseX((tile->coordX * TILESIZE) + baseX)
, chunkBaseY((tile->coordY * TILESIZE) + baseY)
{
    chunkMaterial = ChunkMaterialPtr(new ChunkMaterial);
    chunkMaterial->setShaders(":/shaders/qeditor.vert",
                              ":/shaders/qeditor.tcs",
                              ":/shaders/qeditor.tes",
                              ":/shaders/qeditor.geom",
                              ":/shaders/qeditor.frag");
    //chunkMaterial->link();

    /// Textures
    SamplerPtr tilingSampler(new Sampler);
    tilingSampler->create();
    tilingSampler->setMinificationFilter(GL_LINEAR_MIPMAP_LINEAR);
    world->getGLFunctions()->glSamplerParameterf(tilingSampler->samplerId(), GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f);
    tilingSampler->setMagnificationFilter(GL_LINEAR);
    tilingSampler->setWrapMode(Sampler::DirectionS, GL_REPEAT);
    tilingSampler->setWrapMode(Sampler::DirectionT, GL_REPEAT);

    QImage grassImage("grass.png");
    world->getGLFunctions()->glActiveTexture(GL_TEXTURE1);

    TexturePtr grassTexture(new Texture);
    grassTexture->create();
    grassTexture->bind();
    grassTexture->setImage(grassImage);
    grassTexture->generateMipMaps();

    chunkMaterial->setTextureUnitConfiguration(1, grassTexture, tilingSampler, QByteArrayLiteral("grassTexture"));

    QImage rockImage("rock.png");
    world->getGLFunctions()->glActiveTexture(GL_TEXTURE2);

    TexturePtr rockTexture(new Texture);
    rockTexture->create();
    rockTexture->bind();
    rockTexture->setImage(rockImage);
    rockTexture->generateMipMaps();

    chunkMaterial->setTextureUnitConfiguration(2, rockTexture, tilingSampler, QByteArrayLiteral("rockTexture"));

    QImage snowImage("snowrocks.png");
    world->getGLFunctions()->glActiveTexture(GL_TEXTURE3);

    TexturePtr snowTexture(new Texture);
    snowTexture->create();
    snowTexture->bind();
    snowTexture->setImage(snowImage);
    snowTexture->generateMipMaps();

    chunkMaterial->setTextureUnitConfiguration(3, snowTexture, tilingSampler, QByteArrayLiteral("snowTexture"));

    world->getGLFunctions()->glActiveTexture(GL_TEXTURE0);

    /// Load from file
    if(file.isOpen())
        mapData = tile->getHeader().mcin->entries[chunkIndex()].mcnk->heightOffset->height;
    else
    {
        mapData = new float[CHUNK_ARRAY_SIZE];

        for(int i = 0; i < CHUNK_ARRAY_SIZE; ++i)
            mapData[i] = 0.0f;
    }

    world->getGLFunctions()->glActiveTexture(GL_TEXTURE0);

    terrainData->create();
    terrainData->bind();
    terrainData->setImageAlpha(mapData);

    chunkMaterial->setTextureUnitConfiguration(0, terrainData, terrainSampler, QByteArrayLiteral("heightMap"));

    initialize();
}

MapChunk::~MapChunk()
{
    delete[] mapData;

    mapData = NULL;

    patchBuffer.destroy();
    vao.destroy();

    if(broadcast != NULL)
        delete broadcast;
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

    qDebug() << "Total number of patches for mapchunk =" << patchCount;

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
    if(broadcast != NULL && broadcast->isBroadcasting())
        broadcast->stop();

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
    int X = static_cast<int>(x) % MAP_WIDTH;
    int Y = static_cast<int>(y) % MAP_HEIGHT;

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
    // (Y * X_MAX) + X
    //return ((chunkY % CHUNKS) * CHUNKS) + (chunkX / CHUNKS);
    return (chunkY * CHUNKS) + chunkX;
}

bool MapChunk::changeTerrain(float x, float z, float change, float radius, int brush, int brush_type)
{    
    /// !! TODO remove radius, brush
    bool changed = false;

    float xdiff = baseX - x + CHUNKSIZE / 2;
    float ydiff = baseY - z + CHUNKSIZE / 2;

    float dist = sqrt(pow(xdiff, 2) + pow(ydiff, 2));

    if(dist > (radius + CHUNK_DIAMETER))
        return changed;

    QList<QPair<float, QVector2D>> changing;

    int minX = horizToHMapSize(x - radius);
    int maxX = horizToHMapSize(x + radius);

    int minY = horizToHMapSize(z - radius);
    int maxY = horizToHMapSize(z + radius);

    for(int _x = minX; _x < maxX; ++_x)
    {
        for(int _y = minY; _y < maxY; ++_y) // _y mean z!
        {
            switch(brush)
            {
                case 1: // Circle
                default:
                    {
                        xdiff = HMapSizeToHoriz(_x) - x;
                        ydiff = HMapSizeToHoriz(_y) - z;

                        dist = sqrt(pow(xdiff, 2) + pow(ydiff, 2));

                        if(dist < radius)
                        {
                            int X = _x % MAP_WIDTH;
                            int Y = _y % MAP_HEIGHT;

                            X -= chunkX * (MAP_WIDTH / CHUNKS);
                            Y -= chunkY * (MAP_HEIGHT / CHUNKS);

                            int index = (Y * (MAP_WIDTH / CHUNKS)) + X;

                            if(index < 0 || index >= CHUNK_ARRAY_SIZE || X < 0 || Y < 0 || X >= (MAP_WIDTH / CHUNKS) || Y >= (MAP_HEIGHT / CHUNKS))
                                continue;

                            index *= sizeof(float);

                            switch(brush_type)
                            {
                                case 1: // Linear
                                default:
                                    {
                                        float changeFormula = change * (1.0f - dist / radius);

                                        mapData[index] += changeFormula;
                                    }
                                    break;

                                case 2: // Flat
                                    {
                                        float changeFormula = change;

                                        mapData[index] += changeFormula;
                                    }
                                    break;

                                case 3: // Smooth
                                    {
                                        float changeFormula = change / (1.0f + dist / radius);

                                       mapData[index] += changeFormula;
                                    }
                                    break;

                                case 4: // ...
                                    {
                                        float changeFormula = change * ((dist / radius) * (dist / radius) + dist / radius + 1.0f);

                                        mapData[index] += changeFormula;
                                    }
                                    break;

                                case 5: // ...
                                    {
                                        float changeFormula = change * cos(dist / radius);

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
        QVector<QPair<int, float>> broadcastTop;
        QVector<QPair<int, float>> broadcastRight;
        QVector<QPair<int, float>> broadcastBottom;
        QVector<QPair<int, float>> broadcastLeft;

        terrainData->bind();

        for(int i = 0; i < changing.count(); ++i)
        {
            if(changing.value(i).second.y() == 0)
                broadcastTop.append(QPair<int, float>(changing.value(i).second.x(), changing.value(i).first));

            if(changing.value(i).second.x() == MAP_WIDTH / CHUNKS - 1)
                broadcastRight.append(QPair<int, float>(changing.value(i).second.y(), changing.value(i).first));

            if(changing.value(i).second.y() == MAP_HEIGHT / CHUNKS - 1)
                broadcastBottom.append(QPair<int, float>(changing.value(i).second.x(), changing.value(i).first));

            if(changing.value(i).second.x() == 0)
                broadcastLeft.append(QPair<int, float>(changing.value(i).second.y(), changing.value(i).first));

            terrainData->updatePixel(changing.value(i).first, changing.value(i).second);
        }

        chunkMaterial->setTextureUnitConfiguration(0, terrainData, terrainSampler, QByteArrayLiteral("heightMap"));

        bool broadcasting = false;

        if(broadcastTop.count() > 0 || broadcastRight.count() > 0 || broadcastBottom.count() > 0 || broadcastLeft.count() > 0)
            broadcasting = true;

        if(!broadcasting)
            broadcast = new Broadcast();
        else
            broadcast = new Broadcast(broadcastTop, broadcastRight, broadcastBottom, broadcastLeft);
    }

    return changed;
}

bool MapChunk::flattenTerrain(float x, float z, float y, float change, float radius, int brush, int brush_type)
{
    bool changed = false;

    float xdiff = baseX - x + CHUNKSIZE / 2;
    float ydiff = baseY - z + CHUNKSIZE / 2;

    float dist = sqrt(pow(xdiff, 2) + pow(ydiff, 2));

    if(dist > (radius + CHUNK_DIAMETER))
        return changed;

    QList<QPair<float, QVector2D>> changing;

    int minX = horizToHMapSize(x - radius);
    int maxX = horizToHMapSize(x + radius);

    int minY = horizToHMapSize(z - radius);
    int maxY = horizToHMapSize(z + radius);

    for(int _x = minX; _x < maxX; ++_x)
    {
        for(int _y = minY; _y < maxY; ++_y) // _y mean z!
        {
            switch(brush)
            {
                case 1:
                default:
                    {
                        xdiff = HMapSizeToHoriz(_x) - x;
                        ydiff = HMapSizeToHoriz(_y) - z;

                        dist = sqrt(pow(xdiff, 2) + pow(ydiff, 2));

                        if(dist < radius)
                        {
                            int X = _x % MAP_WIDTH;
                            int Y = _y % MAP_HEIGHT;

                            X -= chunkX * (MAP_WIDTH / CHUNKS);
                            Y -= chunkY * (MAP_HEIGHT / CHUNKS);

                            int index = (Y * (MAP_WIDTH / CHUNKS)) + X;

                            if(index < 0 || index >= CHUNK_ARRAY_SIZE || X < 0 || Y < 0 || X >= (MAP_WIDTH / CHUNKS) || Y >= (MAP_HEIGHT / CHUNKS))
                                continue;

                            index *= sizeof(float);

                            switch(brush_type)
                            {
                                case 1: // Linear
                                default:
                                    {
                                        float changeFormula = 1.0f - (1.0f - change) * (1.0f - dist / radius);

                                        mapData[index] = changeFormula * mapData[index] + (1 - changeFormula) * y;
                                    }
                                    break;

                                case 2: // Flat
                                        mapData[index] = change * mapData[index] + (1 - change) * y;
                                    break;

                                case 3: // Smooth
                                    {
                                        float changeFormula = 1.0f - pow(1.0f - change, 1.0f + dist / radius);

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
        QVector<QPair<int, float>> broadcastTop;
        QVector<QPair<int, float>> broadcastRight;
        QVector<QPair<int, float>> broadcastBottom;
        QVector<QPair<int, float>> broadcastLeft;

        terrainData->bind();

        for(int i = 0; i < changing.count(); ++i)
        {
            if(changing.value(i).second.y() == 0)
                broadcastTop.append(QPair<int, float>(changing.value(i).second.x(), changing.value(i).first));

            if(changing.value(i).second.x() == MAP_WIDTH / CHUNKS - 1)
                broadcastRight.append(QPair<int, float>(changing.value(i).second.y(), changing.value(i).first));

            if(changing.value(i).second.y() == MAP_HEIGHT / CHUNKS - 1)
                broadcastBottom.append(QPair<int, float>(changing.value(i).second.x(), changing.value(i).first));

            if(changing.value(i).second.x() == 0)
                broadcastLeft.append(QPair<int, float>(changing.value(i).second.y(), changing.value(i).first));

            terrainData->updatePixel(changing.value(i).first, changing.value(i).second);
        }

        chunkMaterial->setTextureUnitConfiguration(0, terrainData, terrainSampler, QByteArrayLiteral("heightMap"));

        bool broadcasting = false;

        if(broadcastTop.count() > 0 || broadcastRight.count() > 0 || broadcastBottom.count() > 0 || broadcastLeft.count() > 0)
            broadcasting = true;

        if(!broadcasting)
            broadcast = new Broadcast();
        else
            broadcast = new Broadcast(broadcastTop, broadcastRight, broadcastBottom, broadcastLeft);
    }

    return changed;
}

bool MapChunk::blurTerrain(float x, float z, float change, float radius, int brush, int brush_type)
{
    bool changed = false;

    float xdiff = baseX - x + CHUNKSIZE / 2;
    float ydiff = baseY - z + CHUNKSIZE / 2;

    float dist = sqrt(pow(xdiff, 2) + pow(ydiff, 2));

    if(dist > (radius + CHUNK_DIAMETER))
        return changed;

    QList<QPair<float, QVector2D>> changing;

    int minX = horizToHMapSize(x - radius);
    int maxX = horizToHMapSize(x + radius);

    int minY = horizToHMapSize(z - radius);
    int maxY = horizToHMapSize(z + radius);

    for(int _x = minX; _x < maxX; ++_x)
    {
        for(int _y = minY; _y < maxY; ++_y) // _y mean z!
        {
            switch(brush)
            {
                case 1:
                default:
                    {
                        xdiff = HMapSizeToHoriz(_x) - x;
                        ydiff = HMapSizeToHoriz(_y) - z;

                        dist = sqrt(pow(xdiff, 2) + pow(ydiff, 2));

                        if(dist < radius)
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

                            int Rad = (radius / UNITSIZE);

                            for(int j = -Rad * 2; j <= Rad * 2; ++j)
                            {
                                tz = z + j * UNITSIZE / 2;

                                for(int k = -Rad; k <= Rad; ++k)
                                {
                                    tx = x + k * UNITSIZE + (j % 2) * UNITSIZE / 2.0f;

                                    xdiff = tx - HMapSizeToHoriz(_x);
                                    ydiff = tz - HMapSizeToHoriz(_y);

                                    dist2 = sqrt(pow(xdiff, 2) + pow(ydiff, 2));

                                    if(dist2 > radius)
                                        continue;

                                    int X2 = static_cast<int>(tx) % MAP_WIDTH;
                                    int Y2 = static_cast<int>(tz) % MAP_HEIGHT;

                                    X2 -= chunkX * (MAP_WIDTH / CHUNKS);
                                    Y2 -= chunkY * (MAP_HEIGHT / CHUNKS);

                                    int index2 = (Y2 * (MAP_WIDTH / CHUNKS)) + X2;

                                    index2 *= sizeof(float);

                                    const float height = mapData[index2];

                                    if(height)
                                    {
                                        TotalHeight += (1.0f - dist2 / radius) * height;
                                        TotalWidth  += (1.0f - dist2 / radius);
                                    }
                                }
                            }

                            h = TotalHeight / TotalWidth;

                            switch(brush_type)
                            {
                                case 1: // Linear
                                default:
                                    {
                                        float changeFormula = 1.0f - (1.0f - change) * (1.0f - dist / radius);

                                        mapData[index] = changeFormula * mapData[index] + (1 - changeFormula) * h;
                                    }
                                    break;

                                case 2: // Flat
                                        mapData[index] = change * mapData[index] + (1 - change) * h;
                                    break;

                                case 3: // Smooth
                                    {
                                        float changeFormula = 1.0f - pow(1.0f - change, (1.0f + dist / radius));

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
        QVector<QPair<int, float>> broadcastTop;
        QVector<QPair<int, float>> broadcastRight;
        QVector<QPair<int, float>> broadcastBottom;
        QVector<QPair<int, float>> broadcastLeft;

        terrainData->bind();

        for(int i = 0; i < changing.count(); ++i)
        {
            if(changing.value(i).second.y() == 0)
                broadcastTop.append(QPair<int, float>(changing.value(i).second.x(), changing.value(i).first));

            if(changing.value(i).second.x() == MAP_WIDTH / CHUNKS - 1)
                broadcastRight.append(QPair<int, float>(changing.value(i).second.y(), changing.value(i).first));

            if(changing.value(i).second.y() == MAP_HEIGHT / CHUNKS - 1)
                broadcastBottom.append(QPair<int, float>(changing.value(i).second.x(), changing.value(i).first));

            if(changing.value(i).second.x() == 0)
                broadcastLeft.append(QPair<int, float>(changing.value(i).second.y(), changing.value(i).first));

            terrainData->updatePixel(changing.value(i).first, changing.value(i).second);
        }

        chunkMaterial->setTextureUnitConfiguration(0, terrainData, terrainSampler, QByteArrayLiteral("heightMap"));

        bool broadcasting = false;

        if(broadcastTop.count() > 0 || broadcastRight.count() > 0 || broadcastBottom.count() > 0 || broadcastLeft.count() > 0)
            broadcasting = true;

        if(!broadcasting)
            broadcast = new Broadcast();
        else
            broadcast = new Broadcast(broadcastTop, broadcastRight, broadcastBottom, broadcastLeft);
    }

    return changed;
}

bool MapChunk::paintTerrain()
{
    return false;
}

int MapChunk::horizToHMapSize(float position)
{
    return static_cast<int>(position / TILESIZE * (float)MAP_WIDTH);
}

float MapChunk::HMapSizeToHoriz(int position)
{
    return static_cast<float>(position) / static_cast<float>(MAP_WIDTH) * TILESIZE;
}

void MapChunk::setBorderHeight(const QVector<QPair<int, float>> data, MapChunkBorder border)
{
    QPair<int, float> datas;

    switch(border)
    {
        case MapChunkBorder::Top:
            {
                if(data.count() > 1)
                {
                    terrainData->bind();

                    foreach(datas, data)
                    {
                        terrainData->updatePixel(datas.second, QVector2D(datas.first, MAP_HEIGHT / CHUNKS - 1));

                        mapData[((MAP_HEIGHT / CHUNKS - 1) * MAP_HEIGHT / CHUNKS) + datas.first] = datas.second;
                    }
                }
                else
                {
                    terrainData->updatePixel(data[0].second, QVector2D(data[0].first, MAP_HEIGHT / CHUNKS - 1), true);

                    mapData[((MAP_HEIGHT / CHUNKS - 1) * MAP_HEIGHT / CHUNKS) + data[0].first] = data[0].second;
                }
            }
            break;

        case MapChunkBorder::Right:
            {
                if(data.count() > 1)
                {
                    terrainData->bind();

                    foreach(datas, data)
                    {
                        terrainData->updatePixel(datas.second, QVector2D(0, datas.first));

                        mapData[((datas.first * MAP_HEIGHT / CHUNKS) + 0) * sizeof(float)] = datas.second;
                    }
                }
                else
                {
                    terrainData->updatePixel(data[0].second, QVector2D(0, data[0].first), true);

                    mapData[((data[0].first * MAP_HEIGHT / CHUNKS) + 0) * sizeof(float)] = data[0].second;
                }
            }
            break;

        case MapChunkBorder::Bottom:
            {
                if(data.count() > 1)
                {
                    terrainData->bind();

                    foreach(datas, data)
                    {
                        terrainData->updatePixel(datas.second, QVector2D(datas.first, 0));

                        mapData[(0 * MAP_HEIGHT / CHUNKS) + datas.first] = datas.second;
                    }
                }
                else
                {
                    terrainData->updatePixel(data[0].second, QVector2D(data[0].first, 0), true);

                    mapData[(0 * MAP_HEIGHT / CHUNKS) + data[0].first] = data[0].second;
                }
            }
            break;

        case MapChunkBorder::Left:
            {
                if(data.count() > 1)
                {
                    terrainData->bind();

                    foreach(datas, data)
                    {
                        terrainData->updatePixel(datas.second, QVector2D(MAP_WIDTH / CHUNKS - 1, datas.first));

                        mapData[(datas.first * MAP_HEIGHT / CHUNKS) + ((MAP_WIDTH / CHUNKS) - 1)] = datas.second;
                    }
                }
                else
                {
                    terrainData->updatePixel(data[0].second, QVector2D(MAP_WIDTH / CHUNKS - 1, data[0].first), true);

                    mapData[(data[0].first * MAP_HEIGHT / CHUNKS) + ((MAP_WIDTH / CHUNKS) - 1)] = data[0].second;
                }
            }
            break;
    }

    chunkMaterial->setTextureUnitConfiguration(0, terrainData, terrainSampler, QByteArrayLiteral("heightMap"));
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
