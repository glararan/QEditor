#include "maptile.h"

#include <QImage>
#include <QDir>

MapTile::MapTile(World* mWorld, const QString& mapFile, int x, int y) // Cache MapTile
: coordX(x)
, coordY(y)
, fileName(mapFile)
, patchBuffer(QOpenGLBuffer::VertexBuffer)
, patchCount(0)
, positionData(0)
, terrainSampler(new Sampler)
, terrainMapData(new TextureArray(MAP_WIDTH, MAP_HEIGHT, CHUNKS * CHUNKS))
, world(mWorld) // todo objects
{
    // Create a Vertex Buffers
    const int maxTessellationLevel     = 64;
    const int trianglesPerHeightSample = 10;

    const int xDivisions = trianglesPerHeightSample * MAP_WIDTH  / maxTessellationLevel;
    const int zDivisions = trianglesPerHeightSample * MAP_HEIGHT / maxTessellationLevel;

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

    // Create a VAO for this "object"
    vao.create();
    {
        QOpenGLVertexArrayObject::Binder binder(&vao);
        QOpenGLShaderProgramPtr shader = world->material->shader();

        shader->bind();
        patchBuffer.bind();

        shader->enableAttributeArray("vertexPosition");
        shader->setAttributeBuffer("vertexPosition", GL_FLOAT, 0, 2);
    }

    for(int x = 0; x < CHUNKS; ++x)
    {
        for(int y = 0; y < CHUNKS; ++y)
            mapChunks[x][y] = NULL;
    }

    terrainMapData->create();
    terrainMapData->bind(true);

    terrainSampler->create();
    terrainSampler->setMinificationFilter(GL_LINEAR);
    terrainSampler->setMagnificationFilter(GL_LINEAR);
    terrainSampler->setWrapMode(Sampler::DirectionS, GL_CLAMP_TO_EDGE);
    terrainSampler->setWrapMode(Sampler::DirectionT, GL_CLAMP_TO_EDGE);

    // load chunks
    for(int i = 0; i < CHUNKS * CHUNKS; ++i)
    {
        mapChunks[i / CHUNKS][i % CHUNKS] = new MapChunk(world, this, i / CHUNKS, i % CHUNKS);
        qDebug() << "chunk["<< i / CHUNKS << ", " << i % CHUNKS << "]: " << i << " bases: " << mapChunks[i / CHUNKS][i % CHUNKS]->getBases();
    }

    world->material->setTextureArrayUnitConfiguration(0, terrainMapData, terrainSampler, QByteArrayLiteral("heightMap"));

    /// in future in mapchunks each own 8 custom textures
    // Textures
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
    world->material->setTextureUnitConfiguration(1, grassTexture, tilingSampler, QByteArrayLiteral("grassTexture"));

    QImage rockImage("rock.png");
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
    world->material->setTextureUnitConfiguration(3, snowTexture, tilingSampler, QByteArrayLiteral("snowTexture"));
}

MapTile::MapTile(World* mWorld, int x, int y, const QString& mapFile) // File based MapTile
: coordX(x)
, coordY(y)
, fileName(mapFile)
, patchBuffer(QOpenGLBuffer::VertexBuffer)
, patchCount(0)
, positionData(0)
, terrainSampler(new Sampler)
, terrainMapData(new TextureArray(MAP_WIDTH, MAP_HEIGHT, CHUNKS * CHUNKS))
, world(mWorld) // todo objects
{
    // Create a Vertex Buffers
    const int maxTessellationLevel     = 64;
    const int trianglesPerHeightSample = 10;

    const int xDivisions = trianglesPerHeightSample * MAP_WIDTH  / maxTessellationLevel;
    const int zDivisions = trianglesPerHeightSample * MAP_HEIGHT / maxTessellationLevel;

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

    // Create a VAO for this "object"
    vao.create();
    {
        QOpenGLVertexArrayObject::Binder binder(&vao);
        QOpenGLShaderProgramPtr shader = world->material->shader();

        shader->bind();
        patchBuffer.bind();

        shader->enableAttributeArray("vertexPosition");
        shader->setAttributeBuffer("vertexPosition", GL_FLOAT, 0, 2);
    }

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

    terrainMapData->create();
    terrainMapData->bind(true);

    terrainSampler->create();
    terrainSampler->setMinificationFilter(GL_LINEAR);
    terrainSampler->setMagnificationFilter(GL_LINEAR);
    terrainSampler->setWrapMode(Sampler::DirectionS, GL_CLAMP_TO_EDGE);
    terrainSampler->setWrapMode(Sampler::DirectionT, GL_CLAMP_TO_EDGE);

    // load chunks
    for(int i = 0; i < CHUNKS * CHUNKS; ++i)
    {
        mapChunks[i / CHUNKS][i % CHUNKS] = new MapChunk(world, this, file, i / CHUNKS, i % CHUNKS);

        qDebug() << "chunk["<< i / CHUNKS << ", " << i % CHUNKS << "]: " << i << " bases: " << mapChunks[i / CHUNKS][i % CHUNKS]->getBases();
    }

    world->material->setTextureArrayUnitConfiguration(0, terrainMapData, terrainSampler, QByteArrayLiteral("heightMap"));

    /// in future in mapchunks each own 8 custom textures
    // Textures
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
    world->material->setTextureUnitConfiguration(1, grassTexture, tilingSampler, QByteArrayLiteral("grassTexture"));

    QImage rockImage("rock.png");
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
    world->material->setTextureUnitConfiguration(3, snowTexture, tilingSampler, QByteArrayLiteral("snowTexture"));

    file.close();
}

MapTile::~MapTile()
{
    qDebug() << "Unloading tile: " << coordX << ":" << coordY;

    patchBuffer.destroy();
    vao.destroy();

    terrainSampler->destroy();
    terrainMapData->destroy();

    for(int i = 0; i < CHUNKS; ++i)
    {
        for(int j = 0; j < CHUNKS; ++j)
            mapChunks[i][j] = NULL;
    }
}

void MapTile::draw(const float& distance, const QVector3D& camera)
{
    QOpenGLShaderProgramPtr shader = world->material->shader();
    shader->bind();

    // Set the fragment shader display mode subroutine
    world->getGLFunctions()->glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &world->getDisplaySubroutines());

    if(world->displayMode() != world->Hidden)
    {
        // Render the quad as a patch
        {
            QOpenGLVertexArrayObject::Binder binder(&vao);
            shader->setPatchVertexCount(1);

            world->getGLFunctions()->glDrawArrays(GL_PATCHES, 0, patchCount);
        }
    }

    for(int i = 0; i < CHUNKS; ++i)
    {
        for(int j = 0; j < CHUNKS; ++j)
        {
            if(mapChunks[i][j]->isInVisibleRange(distance, camera))
                mapChunks[i][j]->draw();
        }
    }
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

    terrainMapData->bind();

    for(int i = MAP_WIDTH / CHUNKS * 0; i < MAP_WIDTH / CHUNKS * 1; ++i)
    {
        for(int j = MAP_HEIGHT / CHUNKS * 0; j < MAP_HEIGHT / CHUNKS * 1; ++j)
            terrainMapData->updatePixel(3.0f, QVector2D(i, j));
    }

    //terrainMapData->update();

    world->material->setTextureArrayUnitConfiguration(0, terrainMapData, terrainSampler, QByteArrayLiteral("heightMap"));
}
