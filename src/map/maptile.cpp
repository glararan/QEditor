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

    waterTile = new WaterTile(this);
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

    waterTile = new WaterTile(this, file);

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

void MapTile::drawWater(const float& distance, const QVector3D& camera)
{
    fbo->bind();

    world->getGLFunctions()->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    world->getGLFunctions()->glDisable(GL_DEPTH_TEST);

    draw(distance, camera);

    world->getGLFunctions()->glEnable(GL_DEPTH_TEST);
    world->getGLFunctions()->glClear(GL_DEPTH_BUFFER_BIT);

    fbo->release();

    QOpenGLShaderProgram* shader = world->getWaterShader();
    shader->bind();

    for(int x = 0; x < CHUNKS; ++x)
    {
        for(int y = 0; y < CHUNKS; ++y)
        {
            if(mapChunks[x][y]->isInVisibleRange(distance, camera))
            {
                /*fbo->bind();

                world->getGLFunctions()->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                mapChunks[x][y]->draw();

                fbo->release();*/

                waterTile->getChunk(x, y)->draw(shader, fbo->texture(), fbo->depthTexture());

                //waterTile->getChunk(x, y)->draw(0);
            }
        }
    }
}

void MapTile::update(qreal time)
{
    Q_UNUSED(time);
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
    /*QOpenGLFramebufferObjectFormat format;
    format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
    format.setMipmap(true); // http://www.opengl.org/wiki/Common_Mistakes#Render_To_Texture

    if(fbo == NULL)
        fbo = new QOpenGLFramebufferObject(size, format);
    else
    {
        delete fbo;

        fbo = new QOpenGLFramebufferObject(size, format);
    }*/

    if(fbo == NULL)
        fbo = new Framebuffer(size.height(), size.height());//Framebuffer(size.width(), size.height());
    else
    {
        delete fbo;

        fbo = new Framebuffer(size.height(), size.height());//Framebuffer(size.width(), size.height());
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

void MapTile::insertModel(MapObject *object)
{
    objects.push_back(object);
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