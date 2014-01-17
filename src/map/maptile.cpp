#include "maptile.h"

#include "watertile.h"

#include <QImage>
#include <QDir>

MapTile::MapTile(World* mWorld, const QString& mapFile, int x, int y) // Cache MapTile
: coordX(x)
, coordY(y)
, fileName(mapFile)
, fbo(NULL)
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

    waterTile = new WaterTile(this, true);
}

MapTile::MapTile(World* mWorld, int x, int y, const QString& mapFile) // File based MapTile
: coordX(x)
, coordY(y)
, fileName(mapFile)
, fbo(NULL)
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

    waterTile = new WaterTile(this);

    file.close();
}

MapTile::~MapTile()
{
    qDebug() << QObject::tr("Unloading tile:") << coordX << coordY;

    terrainSampler->destroy();

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
}

void MapTile::draw(const float& distance, const QVector3D& camera)
{
    for(int x = 0; x < CHUNKS; ++x)
    {
        for(int y = 0; y < CHUNKS; ++y)
        {
            if(mapChunks[x][y]->isInVisibleRange(distance, camera))
                mapChunks[x][y]->draw();
        }
    }
}
#include <QMessageBox>
void MapTile::preDrawWater()
{
    /*fbo->bind();

    world->getGLFunctions()->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    mapChunks[0][0]->draw();

    fbo->bindDefault();*/

    /*GLuint fb, tex;

    world->getGLFunctions()->glGenFramebuffers(1, &fb);
    world->getGLFunctions()->glGenTextures(1, &tex);

    {
        world->getGLFunctions()->glBindTexture(GL_TEXTURE_2D, tex);
        world->getGLFunctions()->glBindFramebuffer(GL_FRAMEBUFFER, fb);

        world->getGLFunctions()->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        world->getGLFunctions()->glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        world->getGLFunctions()->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, fbo->width(), fbo->height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

        world->getGLFunctions()->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
    }

    {
        world->getGLFunctions()->glBindFramebuffer(GL_FRAMEBUFFER, fb);
        {
            mapChunks[0][0]->draw();
        }

        world->getGLFunctions()->glBindFramebuffer(GL_FRAMEBUFFER, 0);
        {
            waterTile->getChunk(0, 0)->draw(tex);
        }
    }*/
}

void MapTile::drawWater(const float& distance, const QVector3D& camera)
{
    /*for(int x = 0; x < CHUNKS; ++x)
    {
        for(int y = 0; y < CHUNKS; ++y)
        {
            if(mapChunks[x][y]->isInVisibleRange(distance, camera))
            {
                fbo->bind();

                mapChunks[x][y]->draw();

                fbo->release();

                waterTile->getChunk(x, y)->draw(fbo->texture());
            }
        }
    }*/

    fbo->bind();

    mapChunks[0][0]->draw();

    fbo->bindDefault();

    waterTile->getChunk(0, 0)->draw(fbo->texture());

    fbo->release();
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

void MapTile::setFboSize(QSize size)
{
    QOpenGLFramebufferObjectFormat format;
    format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
    //format.setMipmap(true);

    if(fbo == NULL)
        fbo = new QOpenGLFramebufferObject(size, format);
    else
    {
        delete fbo;

        fbo = new QOpenGLFramebufferObject(size, format);
    }
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