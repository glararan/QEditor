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

#include "maptile.h"

#include "watertile.h"
#include "mapcleft.h"
#include "perlingenerator.h"
#include "qeditor.h"

#include <QImage>
#include <QRgb>
#include <QDir>

MapTile::MapTile(World* mWorld, const QString& mapFile, int x, int y) // Cache MapTile
: coordX(x)
, coordY(y)
, fileName(mapFile)
, terrainSampler(new Sampler)
, world(mWorld) // todo objects
{
    for(int x = 0; x < CHUNKS; ++x)
    {
        for(int y = 0; y < CHUNKS; ++y)
            mapChunks[x][y] = NULL;
    }

    terrainSampler->create();
    terrainSampler->setMinificationFilter(GL_LINEAR);
    terrainSampler->setMagnificationFilter(GL_LINEAR);
    terrainSampler->setWrapMode(Sampler::DirectionS, GL_CLAMP_TO_EDGE);
    terrainSampler->setWrapMode(Sampler::DirectionT, GL_CLAMP_TO_EDGE);

    // load chunks
    for(int i = 0; i < CHUNKS * CHUNKS; ++i)
    {
        mapChunks[i / CHUNKS][i % CHUNKS] = new MapChunk(world, this, i / CHUNKS, i % CHUNKS);

        qDebug() << QString(QObject::tr("chunk[%1, %2]: %3 bases: [%4, %5]")).arg(i / CHUNKS).arg(i % CHUNKS).arg(i).arg(mapChunks[i / CHUNKS][i % CHUNKS]->getBases().x()).arg(mapChunks[i / CHUNKS][i % CHUNKS]->getBases().y());
    }

    /// create map clefts
    /*// horizontal
    for(int x = 0; x < CHUNKS; ++x)
    {
        for(int y = 0; y < CHUNKS - 1; ++y)
        {
            mapCleftHorizontal[x][y] = new MapCleft(world, this, MapChunk::Horizontal, mapChunks[x][y], mapChunks[x][y + 1]);

            qDebug() << QString(QObject::tr("cleft[%1, %2] - horizontal")).arg(x).arg(y);
        }
    }

    // vertical
    for(int x = 0; x < CHUNKS - 1; ++x)
    {
        for(int y = 0; y < CHUNKS; ++y)
        {
            mapCleftVertical[x][y] = new MapCleft(world, this, MapChunk::Vertical, mapChunks[x][y], mapChunks[x + 1][y]);

            qDebug() << QString(QObject::tr("cleft[%1, %2] - vertical")).arg(x).arg(y);
        }
    }*/

    waterTile = new WaterTile(this);
}

MapTile::MapTile(World* mWorld, int x, int y, const QString& mapFile) // File based MapTile
: coordX(x)
, coordY(y)
, fileName(mapFile)
, terrainSampler(new Sampler)
, world(mWorld) // todo objects
{
    for(int x = 0; x < CHUNKS; ++x)
    {
        for(int y = 0; y < CHUNKS; ++y)
            mapChunks[x][y] = NULL;
    }

    QFile file(QDir(world->getProjectData().projectRootDir + "/maps").filePath(fileName));

    if(file.exists() && !file.open(QIODevice::ReadOnly))
        qCritical(QString(QObject::tr("Can't open file %1, map will be generated for tile %2_%3!")).arg(file.fileName()).arg(coordX).arg(coordY).toLatin1().constData());
    else
    {
        QDataStream dataStream(&file);
        dataStream.setVersion(QDataStream::Qt_5_0);

        dataStream >> tileHeader;
    }

    terrainSampler->create();
    terrainSampler->setMinificationFilter(GL_LINEAR);
    terrainSampler->setMagnificationFilter(GL_LINEAR);
    terrainSampler->setWrapMode(Sampler::DirectionS, GL_CLAMP_TO_EDGE);
    terrainSampler->setWrapMode(Sampler::DirectionT, GL_CLAMP_TO_EDGE);

    // load chunks
    for(int i = 0; i < CHUNKS * CHUNKS; ++i)
    {
        mapChunks[i / CHUNKS][i % CHUNKS] = new MapChunk(world, this, file, i / CHUNKS, i % CHUNKS);

        qDebug() << QString(QObject::tr("chunk[%1, %2]: %3 bases: [%4, %5]")).arg(i / CHUNKS).arg(i % CHUNKS).arg(i).arg(mapChunks[i / CHUNKS][i % CHUNKS]->getBases().x()).arg(mapChunks[i / CHUNKS][i % CHUNKS]->getBases().y());
    }

    /// create map clefts
    /*// horizontal
    for(int x = 0; x < CHUNKS; ++x)
    {
        for(int y = 0; y < CHUNKS - 1; ++y)
        {
            mapCleftHorizontal[x][y] = new MapCleft(world, this, MapChunk::Horizontal, mapChunks[x][y], mapChunks[x][y + 1]);

            qDebug() << QString(QObject::tr("cleft[%1, %2] - horizontal")).arg(x).arg(y);
        }
    }

    // vertical
    for(int x = 0; x < CHUNKS - 1; ++x)
    {
        for(int y = 0; y < CHUNKS; ++y)
        {
            mapCleftVertical[x][y] = new MapCleft(world, this, MapChunk::Vertical, mapChunks[x][y], mapChunks[x + 1][y]);

            qDebug() << QString(QObject::tr("cleft[%1, %2] - vertical")).arg(x).arg(y);
        }
    }*/

    waterTile = new WaterTile(this, file);

    file.close();
}

MapTile::~MapTile()
{
    qDebug() << QObject::tr("Unloading tile:") << coordX << coordY;

    terrainSampler->destroy();

    /*for(int x = 0; x < CHUNKS; ++x)
    {
        for(int y = 0; y < CHUNKS - 1; ++y)
        {
            if(mapCleftHorizontal[x][y])
            {
                delete mapCleftHorizontal[x][y];

                mapCleftHorizontal[x][y] = NULL;
            }
        }
    }

    for(int x = 0; x < CHUNKS - 1; ++x)
    {
        for(int y = 0; y < CHUNKS; ++y)
        {
            if(mapCleftVertical[x][y])
            {
                delete mapCleftVertical[x][y];

                mapCleftVertical[x][y] = NULL;
            }
        }
    }*/

    for(int x = 0; x < CHUNKS; ++x)
    {
        for(int y = 0; y < CHUNKS; ++y)
        {
            if(mapChunks[x][y])
            {
                delete mapChunks[x][y];

                mapChunks[x][y] = NULL;
            }
        }
    }

    for(int i = 0; i < objects.size(); ++i)
        delete objects.at(i);
}

void MapTile::draw(const float& distance, const QVector3D& camera)
{
    QOpenGLShaderProgram* shader = world->getTerrainShader();
    shader->bind();

    for(int x = 0; x < CHUNKS; ++x)
    {
        for(int y = 0; y < CHUNKS; ++y)
        {
            if(mapChunks[x][y]->isInVisibleRange(distance, camera))
                mapChunks[x][y]->draw(shader);
        }
    }

    /*QOpenGLShaderProgram* shader2 = world->getCleftShader();
    shader2->bind();*/

    // horiz & vertical settings
    /*shader2->setUniformValue("textureScaleOption", QVector4D(0, 0, 0, 0));
    shader2->setUniformValue("textureScaleFar",    QVector4D(0.4f, 0.4f, 0.4f, 0.4f));
    shader2->setUniformValue("textureScaleNear",   QVector4D(0.4f, 0.4f, 0.4f, 0.4f));

    // horizontal settings
    shader2->setUniformValue("horizontalScaleX", CHUNKSIZE);
    shader2->setUniformValue("horizontalScaleY", CHUNKSIZE / CHUNK_WIDTH * 7);//0.01f);
    shader2->setUniformValue("vertical",         false);*/

    // horizontal draw
    /*for(int x = 0; x < CHUNKS; ++x)
    {
        for(int y = 0; y < CHUNKS - 1; ++y)
        {
            if(mapCleftHorizontal[x][y]->isInVisibleRange(distance, camera))
                mapCleftHorizontal[x][y]->draw(shader2);
        }
    }*/

    // vertical settings
    /*shader2->setUniformValue("horizontalScaleX", CHUNKSIZE / CHUNK_WIDTH * 10);//0.01f);//CHUNKSIZE / CHUNK_WIDTH);
    shader2->setUniformValue("horizontalScaleY", CHUNKSIZE);*/
    //shader2->setUniformValue("vertical",         true);

    // vertical draw
    /*for(int x = 0; x < CHUNKS - 1; ++x)
    {
        for(int y = 0; y < CHUNKS; ++y)
        {
            if(mapCleftVertical[x][y]->isInVisibleRange(distance, camera))
                mapCleftVertical[x][y]->draw(shader2);
        }
    }*/
}

void MapTile::drawObjects(const float& distance, const QVector3D& camera, QMatrix4x4 viewMatrix, QMatrix4x4 projectionMatrix)
{
    Q_UNUSED(distance);
    Q_UNUSED(camera);

    QOpenGLShaderProgram* shader = world->getModelShader();
    shader->bind();

    for(int i = 0; i < objects.size(); ++i)
    {
        MapObject* mapObject = objects.at(i);

        QVector3D translate = mapObject->translate;
        QVector3D rotation  = mapObject->rotation;
        QVector3D scale     = mapObject->scale;

        IPipeline Pipeline(QMatrix4x4(),viewMatrix, projectionMatrix);
        Pipeline.translate(translate.x(), translate.y() + mapObject->height_offset, translate.z());
        Pipeline.scale(scale.x(),scale.y(), scale.z());
        Pipeline.rotateX(rotation.x() * 360.0f);
        Pipeline.rotateY(rotation.y() * 360.0f);
        Pipeline.rotateZ(rotation.z() * 360.0f);

        if(mapObject->model)
        {
            Pipeline.updateMatrices(shader);
            mapObject->model->draw(shader);
        }
    }
}

void MapTile::drawWater(const float& distance, const QVector3D& camera, QOpenGLFramebufferObject* refraction)
{
    QOpenGLShaderProgram* shader = world->getWaterShader();
    shader->bind();

    for(int x = 0; x < CHUNKS; ++x)
    {
        for(int y = 0; y < CHUNKS; ++y)
        {
            if(mapChunks[x][y]->isInVisibleRange(distance, camera))
                waterTile->getChunk(x, y)->draw(shader, refraction->texture(), NULL); // depthTexture missing
        }
    }
}

void MapTile::update(qreal time)
{
    Q_UNUSED(time);
}

void MapTile::generateMap(MapGenerationData& data)
{
    // generate tile map
    float** tileData = new float*[TILE_WIDTH];

    for(int i = 0; i < TILE_HEIGHT; ++i)
        tileData[i] = new float[TILE_HEIGHT];

    // todo objects for these cycles
    PerlinGenerator perlin(data.seed);

    MathHelper::setRandomSeed(perlin.getSeed());

    for(int x = 0; x < TILE_WIDTH; ++x)
    {
        for(int y = 0; y < TILE_HEIGHT; ++y)
            tileData[x][y] = perlin.noise(QVector3D(data.perlinNoiseLevel * x / MathHelper::toFloat(TILE_WIDTH),
                                                    data.perlinNoiseLevel * y / MathHelper::toFloat(TILE_HEIGHT),
                                                    data.perlinNoiseHeight));
    }

    for(int i = 0; i < data.perlinNoiseMultiple; ++i)
    {
        for(int x = 0; x < TILE_WIDTH; ++x)
        {
            for(int y = 0; y < TILE_HEIGHT; ++y)
                tileData[x][y] += perlin.noise(QVector3D(data.perlinNoiseLevel * x / MathHelper::toFloat(TILE_WIDTH),
                                                         data.perlinNoiseLevel * y / MathHelper::toFloat(TILE_HEIGHT),
                                                         data.perlinNoiseHeight));
        }
    }

    float** tempData = new float*[TILE_WIDTH];

    for(int i = 0; i < TILE_HEIGHT; ++i)
        tempData[i] = new float[TILE_HEIGHT];

    float f = data.perturbFrequency;
    float d = data.perturbD;

    int u, v;

    for(int x = 0; x < TILE_WIDTH; ++x)
    {
        for(int y = 0; y < TILE_HEIGHT; ++y)
        {
            u = x + MathHelper::toInt(perlin.noise(QVector3D(f * x / MathHelper::toFloat(TILE_WIDTH),
                                                             f * y / MathHelper::toFloat(TILE_HEIGHT),
                                                             0.0f)) * d);
            v = y + MathHelper::toInt(perlin.noise(QVector3D(f * x / MathHelper::toFloat(TILE_WIDTH),
                                                             f * y / MathHelper::toFloat(TILE_HEIGHT),
                                                             1.0f)) * d);

            if(u < 0)
                u = 0;

            if(u >= TILE_WIDTH)
                u = TILE_WIDTH - 1;

            if(v < 0)
                v = 0;

            if(v >= TILE_HEIGHT)
                v = TILE_HEIGHT - 1;

            tempData[x][y] = tileData[u][v];
        }
    }

    tileData = tempData; // temp data are jumbled tileData, correcting tileData pointers

    float smoothness = data.erode;

    for(int i = 0; i < data.erodeSmoothenLevel; ++i)
    {
        for(int x = 1; x < TILE_WIDTH - 1; ++x)
        {
            for(int y = 1; y < TILE_HEIGHT - 1; ++y)
            {
                float d_max = 0.0f;

                int match[] = {0, 0};

                for(int U = -1; U <= 1; ++U)
                {
                    for(int V = -1; V <= 1; ++V)
                    {
                        if(abs(U) + abs(V) > 0)
                        {
                            float d_i = tileData[x][y] - tileData[x + U][y + V];

                            if(d_i > d_max)
                            {
                                d_max = d_i;

                                match[0] = U;
                                match[1] = V;
                            }
                        }
                    }
                }

                if(0 < d_max && d_max <= (smoothness / MathHelper::toFloat(TILE_WIDTH)))
                {
                    float d_h = 0.5f * d_max;

                    tileData[x][y]                       -= d_h;
                    tileData[x + match[0]][y + match[1]] += d_h;
                }
            }
        }

        for(int x = 1; x < TILE_WIDTH - 1; ++x)
        {
            for(int y = 1; y < TILE_HEIGHT - 1; ++y)
            {
                float total = 0.0f;

                for(int U = -1; U <= 1; ++U)
                {
                    for(int V = -1; V <= 1; ++V)
                        total += tileData[x + U][y + V];
                }

                tileData[x][y] = total / 8.0f;
            }
        }
    }

    MathHelper::setTimeSeed();

    // set map data
    for(int x = 0; x < CHUNKS; ++x)
    {
        for(int y = 0; y < CHUNKS; ++y)
        {
            float* mapData = new float[CHUNK_ARRAY_UC_SIZE];

            for(int tx = 0; tx < CHUNK_WIDTH; ++tx)
            {
                for(int ty = 0; ty < CHUNK_WIDTH; ++ty)
                    mapData[ty * CHUNK_WIDTH + tx] = tileData[x * CHUNK_WIDTH + tx][y * CHUNK_HEIGHT + ty];
            }

            mapChunks[x][y]->setGeneratedHeightmap(mapData);

            delete[] mapData;
        }
    }

    for(int i = 0; i < TILE_HEIGHT; ++i)
        delete[] tileData[i];

    delete[] tileData; // delete also tempData(source tileData)
}

void MapTile::importHeightmap(QString path, float scale)
{
    // (red * (2^16) + green * (2^8) + blue) / 256 ==> max 65536
    // load file
    if(!QFile::exists(path))
    {
        qDebug() << "Import file doesn't exists.";

        return;
    }

    QFile file(path);

    if(!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "Import file couldn't be read";

        return;
    }

    file.close();

    QImage image(path);

    if(image.width() != TILE_WIDTH || image.height() != TILE_HEIGHT)
    {
        qDebug() << "Import file don't have resolution" << TILE_WIDTH << "x" << TILE_HEIGHT << "!";

        return;
    }

    if(image.format() != QImage::Format_RGB888 && image.format() != QImage::Format_RGB32)
    {
        qDebug() << "Import file require RGB format: RGB888";

        return;
    }

    // to tileData
    float** tileData = new float*[TILE_WIDTH];

    for(int i = 0; i < TILE_HEIGHT; ++i)
        tileData[i] = new float[TILE_HEIGHT];

    for(int x = 0; x < TILE_WIDTH; ++x)
    {
        for(int y = 0; y < TILE_HEIGHT; ++y)
        {
            QColor color = QColor(image.pixel(x, y));

            tileData[x][y] = (((color.redF() * 255.0f) * pow(2.0f, 16)) + ((color.greenF() * 255.0f) * pow(2.0f, 8)) + (color.blueF() * 255.0f)) / 256.0f;
        }
    }

    // set heightmap
    for(int x = 0; x < CHUNKS; ++x)
    {
        for(int y = 0; y < CHUNKS; ++y)
        {
            float* mapData = new float[CHUNK_ARRAY_UC_SIZE];

            for(int tx = 0; tx < CHUNK_WIDTH; ++tx)
            {
                for(int ty = 0; ty < CHUNK_WIDTH; ++ty)
                    mapData[ty * CHUNK_WIDTH + tx] = tileData[x * CHUNK_WIDTH + tx][y * CHUNK_HEIGHT + ty] * scale;
            }

            mapChunks[x][y]->setGeneratedHeightmap(mapData);

            delete[] mapData;
        }
    }

    for(int i = 0; i < TILE_HEIGHT; ++i)
        delete[] tileData[i];

    delete[] tileData;
}

void MapTile::exportHeightmap(QString path, float scale)
{
    QImage output(TILE_WIDTH, TILE_HEIGHT, QImage::Format_RGB888);

    // find lowest val
    float lowest = 0.0f;

    for(int x = 0; x < CHUNKS; ++x)
    {
        for(int y = 0; y < CHUNKS; ++y)
        {
            MapChunk* chunk = mapChunks[x][y];

            for(int i = 0; i < CHUNK_ARRAY_UC_SIZE; ++i)
            {
                if(chunk->mapData[i] < lowest)
                    lowest = chunk->mapData[i];
            }
        }
    }

    if(lowest > 0.0f)
        lowest = 0.0f;

    for(int x = 0; x < CHUNKS; ++x)
    {
        for(int y = 0; y < CHUNKS; ++y)
        {
            MapChunk* chunk = mapChunks[x][y];

            for(int mx = 0; mx < CHUNK_WIDTH; ++mx)
            {
                for(int my = 0; my < CHUNK_HEIGHT; ++my)
                {
                    // correcting heightmap to start from 0.0f, not from negative
                    float height = (chunk->mapData[my * CHUNK_WIDTH + mx] + lowest * -1.0f) * scale;

                    int red, green, blue;
                    red = green = blue = 0;

                    // rgb export
                    /*if(height > 65536.0f)
                        height = 65536.0f;

                    height *= 256.0f;

                    red   = MathHelper::toInt(height) >> 16;
                    green = (MathHelper::toInt(height) >> 8) & 0xff;
                    blue  = MathHelper::toInt(height) & 0xff;*/

                    // grayscale export
                    if(height > 256.0f)
                        height = 256.0f;

                    red = green = blue = MathHelper::toInt(height);

                    QColor color = QColor(red, green, blue);

                    output.setPixel(x * CHUNK_WIDTH + mx, y * CHUNK_HEIGHT + my, color.rgb());
                }
            }
        }
    }

    qDebug() << "Tile" << coordX << coordY << ": export -" << output.save(path, "PNG", 100);
}

MapChunk* MapTile::getChunk(int x, int y)
{
    if(x < CHUNKS && y < CHUNKS)
        return mapChunks[x][y];
    else
        return NULL;
}

bool MapTile::isTile(int pX, int pY)
{
    return pX == coordX && pY == coordY;
}

void MapTile::saveTile()
{
    QFile file(QDir(world->getProjectData().projectRootDir + "/maps").filePath(fileName));

    if(!QDir(world->getProjectData().projectRootDir + "/maps").exists())
    {
        if(!QDir(world->getProjectData().projectRootDir).mkdir("maps"))
        {
            qCritical(QObject::tr("Couldn't create maps folder").toLatin1().data());

            return;
        }
    }

    if(!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        qCritical(QString(QObject::tr("Could not open %1 to be written")).arg(fileName).toLatin1().constData());

        return;
    }

    MapHeader mapHeader;
    mapHeader.version = MAP_HEADER_VERSION;
    mapHeader.mcin    = new MCIN;

    for(int x = 0; x < CHUNKS; ++x)
    {
        for(int y = 0; y < CHUNKS; ++y)
        {
            int index = y * CHUNKS + x;

            mapHeader.mcin->entries[index].mcnk = new MCNK;

            mapChunks[x][y]->save(mapHeader.mcin->entries[index].mcnk);
        }
    }

    QDataStream data(&file);
    data.setVersion(QDataStream::Qt_5_0);

    data << mapHeader;

    file.flush();
    file.close();

    qDebug() << QObject::tr("Successfully written to ") << fileName;
}

void MapTile::test()
{
    //mapChunks[0][0]->test();
    //mapChunks[1][1]->test();
}

void MapTile::insertModel(MapObject *object)
{
    objects.push_back(object);
}

void MapTile::deleteModel(float x, float z)
{
    for(int i = objects.size() - 1; i >= 0; --i)
    {
        MapObject* object = objects.at(i);

        float radius = 5.0f;

        if(object->translate.x() - radius < x && object->translate.x() + radius > x
        && object->translate.z() - radius < z && object->translate.z() + radius > z)
        {
            delete object;

            object = NULL;

            objects.removeAt(i);
        }
    }
}

void MapTile::updateModelHeight()
{
    for(int i = 0; i < objects.size(); ++i)
    {
        MapObject* mapObject = objects.at(i);

        QVector3D position = mapObject->translate;

        mapObject->translate.setY(getHeight(position.x(), position.z()));
    }
}

MapChunk* MapTile::getChunkAt(float x, float z)
{
    int cx = floor((x - TILESIZE * coordX) / CHUNKSIZE);
    int cz = floor((z - TILESIZE * coordY) / CHUNKSIZE);

    if(cx >= CHUNKS || cz >= CHUNKS || cx < 0 || cz < 0)
        return 0;

    return mapChunks[cx][cz];
}

float MapTile::getHeight(float x, float z)
{
    MapChunk* chunk = getChunkAt(x, z);

    if(chunk)
        return chunk->getHeightFromWorld(x, z);

    return 0.0f;
}
