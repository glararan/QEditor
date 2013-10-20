#include "mapchunk.h"

#include "maptile.h"
#include "mathhelper.h"

#include <QImage>

MapChunk::MapChunk(World* mWorld, MapTile* tile, QFile* file, int x, int y)
: world(mWorld)
, terrainData(tile->terrainMapData.data())
, terrainSampler(tile->terrainSampler.data())
, chunkX(x)
, chunkY(y)
, baseX(chunkIndex() / CHUNKS * MAP_WIDTH / CHUNKS)
, baseY(chunkIndex() % CHUNKS * MAP_WIDTH / CHUNKS)
{
    /// Todo load from file

    /*QFile file("map01.map");

    if(file.exists() && file.open(QIODevice::ReadOnly))
    {
        QDataStream data(&file);
        data.setVersion(QDataStream::Qt_5_0);
        data.setFloatingPointPrecision(QDataStream::SinglePrecision);

        for(int i = 0; i < 1024 * 1024 * 4; ++i)
            data >> mapData[i];
    }
    else
    {*/
        for(int i = 0; i < CHUNK_ARRAY_SIZE; ++i) // !!!!!!!! generování terenu
            mapData[i] = 0.1f * (qrand() % 5);
    //}

    terrainData->updateImage(mapData, chunkIndex(), CHUNK_ARRAY_SIZE); // přepsání textury s novými daty

    //tile->terrainMapData->updateImage(mapData, chunkIndex, CHUNK_ARRAY_SIZE);

    /*SamplerPtr tilingSampler(new Sampler);
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
    world->material->setTextureUnitConfiguration(1, grassTexture, tilingSampler, QByteArrayLiteral("grassTexture"));*/

    /*QImage rockImage("rock.png");
    world->getGLFunctions()->glActiveTexture(GL_TEXTURE2);

    TexturePtr rockTexture(new Texture);
    rockTexture->create();
    rockTexture->bind();
    rockTexture->setImage(rockImage);
    rockTexture->generateMipMaps();
    world->material->setTextureUnitConfiguration(2, rockTexture, tilingSampler, QByteArrayLiteral("rockTexture"));

    QImage snowImage("snowrocks.png");
    world->getGLFunctions()->glActiveTexture(GL_TEXTURE3);

    TexturePtr snowTexture(new Texture);
    snowTexture->create();
    snowTexture->bind();
    snowTexture->setImage(snowImage);
    snowTexture->generateMipMaps();
    world->material->setTextureUnitConfiguration(3, snowTexture, tilingSampler, QByteArrayLiteral("snowTexture"));*/

    //world->getGLFunctions()->glActiveTexture(GL_TEXTURE0);
}

MapChunk::~MapChunk()
{
    for(int i = 0; i < CHUNK_ARRAY_SIZE; ++i)
        mapData[i] = 0;
}

void MapChunk::test()
{
    for(int i = 0; i < CHUNK_ARRAY_SIZE; ++i)
        mapData[i] += 10.0f;

    terrainData->updateImage(mapData, chunkIndex(), CHUNK_ARRAY_SIZE);
}

void MapChunk::draw()
{
    // Empty now
}

bool MapChunk::isInVisibleRange(const float& distance, const QVector3D& camera) const
{
    static const float chunkRadius = sqrtf(CHUNKSIZE * CHUNKSIZE / 2.0f);

    return (camera.length() - chunkRadius) < distance;
}

const float MapChunk::getHeight(const float& x, const float& y) const
{
    int X = static_cast<int>(x) % MAP_WIDTH;
    int Y = MAP_HEIGHT - static_cast<int>(y) % MAP_HEIGHT;

    int index = (Y * MAP_WIDTH) + X;

    return mapData[index];
}

const float MapChunk::getMapData(const int& index) const
{
    return mapData[index];
}

const int MapChunk::chunkIndex() const
{
    return ((chunkY % CHUNKS) * CHUNKS) + (chunkX % CHUNKS);
}

const QVector2D MapChunk::getBases() const
{
    return QVector2D(baseX, baseY);
}

bool MapChunk::changeTerrain(float x, float z, float change, float radius, int brush, int brush_type)
{    
    /// !! TODO remove radius, brush
    bool changed = false;

    float xdiff = baseX - x + CHUNKSIZE / 2;
    float ydiff = baseY - z + CHUNKSIZE / 2;

    float dist = sqrt(pow(xdiff, 2) + pow(xdiff, 2)); /// WARNING: second pow should  be "ydiff"

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

                            int index = (Y * (MAP_WIDTH / CHUNKS)) + X;

                            if(chunkIndex() % CHUNKS > 0)
                                index -= (chunkIndex() % CHUNKS) * CHUNK_ARRAY_SIZE;

                            if(index < 0 || index >= CHUNK_ARRAY_SIZE)
                                continue;

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
        terrainData->bind();

        for(int i = 0; i < changing.count(); ++i)
            terrainData->updatePixel(changing.value(i).first, changing.value(i).second);

        world->material->setTextureArrayUnitConfiguration(0, terrainData, terrainSampler, QByteArrayLiteral("heightMap"));
    }

    return changed;
}

bool MapChunk::flattenTerrain(float x, float z, float y, float change, float radius, int brush, int brush_type)
{
    bool changed = false;

    float xdiff = baseX - x + CHUNKSIZE / 2;
    float ydiff = baseY - z + CHUNKSIZE / 2;

    float dist = sqrt(pow(xdiff, 2) + pow(xdiff, 2));

    if(dist > (radius + CHUNK_DIAMETER))
        return changed;

    QList<QPair<float, QVector2D>> changing;

    int minX = horizToHMapSize(x - radius);
    int maxX = horizToHMapSize(x + radius);

    int minY = horizToHMapSize(z + radius);
    int maxY = horizToHMapSize(z - radius);

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

                            int index = (Y * (MAP_WIDTH / CHUNKS)) + X;

                            if(chunkIndex() % CHUNKS > 0)
                                index -= (chunkIndex() % CHUNKS) * CHUNK_ARRAY_SIZE;

                            if(index < 0 || index >= CHUNK_ARRAY_SIZE)
                                continue;

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
        terrainData->bind();

        for(int i = 0; i < changing.count(); ++i)
            terrainData->updatePixel(changing.value(i).first, changing.value(i).second);

        world->material->setTextureArrayUnitConfiguration(0, terrainData, terrainSampler, QByteArrayLiteral("heightMap"));
    }

    return changed;
}

bool MapChunk::blurTerrain(float x, float z, float change, float radius, int brush, int brush_type)
{
    bool changed = false;

    float xdiff = baseX - x + CHUNKSIZE / 2;
    float ydiff = baseY - z + CHUNKSIZE / 2;

    float dist = sqrt(pow(xdiff, 2) + pow(xdiff, 2));

    if(dist > (radius + CHUNK_DIAMETER))
        return changed;

    QList<QPair<float, QVector2D>> changing;

    int minX = horizToHMapSize(x - radius);
    int maxX = horizToHMapSize(x + radius);

    int minY = horizToHMapSize(z + radius);
    int maxY = horizToHMapSize(z - radius);

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

                            int index = (Y * (MAP_WIDTH / CHUNKS)) + X;

                            if(chunkIndex() % CHUNKS > 0)
                                index -= (chunkIndex() % CHUNKS) * CHUNK_ARRAY_SIZE;

                            if(index < 0 || index >= CHUNK_ARRAY_SIZE)
                                continue;

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

                                    int index2 = (Y2 * (MAP_WIDTH / CHUNKS)) + X2;

                                    const float height = mapData[index2 * sizeof(float)];

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
        terrainData->bind();

        for(int i = 0; i < changing.count(); ++i)
            terrainData->updatePixel(changing.value(i).first, changing.value(i).second);

        world->material->setTextureArrayUnitConfiguration(0, terrainData, terrainSampler, QByteArrayLiteral("heightMap"));
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
