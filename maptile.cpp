#include "maptile.h"

#include <QImage>
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

        qDebug() << QString("chunk[%1, %2]: %3 bases: [%4, %5]").arg(i / CHUNKS).arg(i % CHUNKS).arg(i).arg(mapChunks[i / CHUNKS][i % CHUNKS]->getBases().x()).arg(mapChunks[i / CHUNKS][i % CHUNKS]->getBases().y());
    }
}

MapTile::MapTile(World* mWorld, int x, int y, const QString& mapFile) // File based MapTile
: coordX(x)
, coordY(y)
, fileName(mapFile)
, terrainSampler(new Sampler)
, waterReflection(new QOpenGLFramebufferObject(QSize(MAP_WIDTH / 2, MAP_HEIGHT / 2)))
, depthMap( new QOpenGLFramebufferObject(QSize(MAP_WIDTH * 2, MAP_HEIGHT * 2)))
, depthMap2(new QOpenGLFramebufferObject(QSize(MAP_WIDTH * 2, MAP_HEIGHT * 2)))
, world(mWorld) // todo objects
{
    for(int x = 0; x < CHUNKS; ++x)
    {
        for(int y = 0; y < CHUNKS; ++y)
            mapChunks[x][y] = NULL;
    }

    QFile file(QDir(world->getProjectData().projectRootDir + "/maps").filePath(fileName));

    if(file.exists() && !file.open(QIODevice::ReadOnly))
        qCritical(QString("Can't open file %1, map will be generated for tile %2_%3!").arg(file.fileName()).arg(coordX).arg(coordY).toLatin1().constData());
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

        qDebug() << QString("chunk[%1, %2]: %3 bases: [%4, %5]").arg(i / CHUNKS).arg(i % CHUNKS).arg(i).arg(mapChunks[i / CHUNKS][i % CHUNKS]->getBases().x()).arg(mapChunks[i / CHUNKS][i % CHUNKS]->getBases().y());
    }

    file.close();

    /// water
    /*QOpenGLFramebufferObjectFormat frameBufferFormat;
    frameBufferFormat.setSamples(4);
    frameBufferFormat.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);

    waterReflection->setAttachment(frameBufferFormat);

    frameBufferFormat.setAttachment(QOpenGLFramebufferObject::Depth);

    depthMap->setAttachment(frameBufferFormat);
    depthMap->setAttachment(frameBufferFormat);

    world->material->setFramebufferUnitConfiguration(4, waterReflection, tilingSampler, QByteArrayLiteral("waterReflection"));
    world->material->setFramebufferUnitConfiguration(6, depthMap,        tilingSampler, QByteArrayLiteral("depthMap"));
    world->material->setFramebufferUnitConfiguration(7, depthMap2,       tilingSampler, QByteArrayLiteral("depthMap2"));

    QImage waterNoise("waterNoise.png");
    world->getGLFunctions()->glActiveTexture(GL_TEXTURE0 + 5);

    TexturePtr waterNoiseTexture(new Texture);
    waterNoiseTexture->create();
    waterNoiseTexture->bind();
    waterNoiseTexture->setImage(waterNoise);
    waterNoiseTexture->generateMipMaps();
    world->material->setTextureUnitConfiguration(5, waterNoiseTexture, tilingSampler, QByteArrayLiteral("waterNoise"));

    QOpenGLShaderProgramPtr shader = world->material->shader();

    shader->setUniformValue("waterMapSize",     QVector2D(MAP_WIDTH / 2, MAP_HEIGHT / 2));
    shader->setUniformValue("waterNoiseTile",   10.0f);
    shader->setUniformValue("waterNoiseFactor", 0.1f);
    shader->setUniformValue("waterShininess",   50.0f);
    shader->setUniformValue("waterHeight",      0.0f);*/
}

MapTile::~MapTile()
{
    qDebug() << "Unloading tile:" << QVector2D(coordX, coordY);

    terrainSampler->destroy();

    for(int x = 0; x < CHUNKS; ++x)
    {
        for(int y = 0; y < CHUNKS; ++y)
        {
            if(mapChunks[x][y])
            {
                //delete mapChunks[x][y]; // !!! Memory leaks! Can be caused by World or by QSharedPointer

                mapChunks[x][y] = NULL;
            }
        }
    }
}

void MapTile::draw(const float& distance, const QVector3D& camera)
{
    for(int x = 0; x < CHUNKS; ++x)
    {
        for(int y = 0; y < CHUNKS; ++y)
        {
            if(mapChunks[x][y]->isInVisibleRange(distance, camera))
            {
                const Broadcast* broadcast;

                if((broadcast = mapChunks[x][y]->getBroadcast()) != NULL && broadcast->isBroadcasting())
                {
                    MapChunk* chunk;

                    if((chunk = getChunk(x, y - 1)) != NULL && broadcast->getTopData().count() > 0)
                        chunk->setBorderHeight(broadcast->getTopData(), MapChunkBorder::Top);

                    if((chunk = getChunk(x + 1, y)) != NULL && broadcast->getRightData().count() > 0)
                        chunk->setBorderHeight(broadcast->getRightData(), MapChunkBorder::Right);

                    if((chunk = getChunk(x, y + 1)) != NULL && broadcast->getBottomData().count() > 0)
                        chunk->setBorderHeight(broadcast->getBottomData(), MapChunkBorder::Bottom);

                    if((chunk = getChunk(x - 1, y)) != NULL && broadcast->getLeftData().count() > 0)
                        chunk->setBorderHeight(broadcast->getLeftData(), MapChunkBorder::Left);
                }

                mapChunks[x][y]->draw();
            }
        }
    }

    /// water
    /*world->getGLFunctions()->glActiveTexture(GL_TEXTURE0 + 4);
    world->getGLFunctions()->glEnable(GL_TEXTURE_2D);
    waterReflection->bind();

    int wrID = world->getGLFunctions()->glGetUniformLocation(shader->programId(), "waterReflection");

    shader->setUniformValue(wrID, 4);

    world->getGLFunctions()->glActiveTexture(GL_TEXTURE0 + 4);
    waterReflection->release();
    world->getGLFunctions()->glDisable(GL_TEXTURE_2D);*/
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
            qCritical("Couldn't create maps folder");

            return;
        }
    }

    if(!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        qCritical(QString("Could not open %1 to be written").arg(fileName).toLatin1().constData());

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

    qDebug() << "Successfully written to " << fileName;
}

void MapTile::test()
{
    //mapChunks[0][0]->test();
    //mapChunks[1][1]->test();

    QVector<QPair<int, float>> data;

    for(int i = 0; i < MAP_HEIGHT / CHUNKS; ++i)
        data.append(qMakePair<int, float>(i, mapChunks[0][0]->getHeight(MAP_WIDTH / CHUNKS - 1, i)));

    mapChunks[1][0]->setBorderHeight(data, MapChunkBorder::Right);
}
