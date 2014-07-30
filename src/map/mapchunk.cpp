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

#include "mapchunk.h"

#include "maptile.h"
#include "qeditor.h"
#include "mathhelper.h"
#include "brush.h"

#include <QImage>
#include <QOpenGLPixelTransferOptions>
#include <QMultiMap>
#include <QFileInfo>

MapChunk::MapChunk(World* mWorld, MapTile* tile, int x, int y) // Cache MapChunk
: world(mWorld)
, terrainSampler(tile->terrainSampler.data())
, terrainData(new Texture(QOpenGLTexture::Target2D))
, mapDataCache(NULL)
, vertexShadingMap(NULL)
, displaySubroutines(world->DisplayModeCount)
, highlight(false)
, selected(false)
, mapGeneration(false)
, mapScale(false)
, bottomNeighbour(NULL)
, leftNeighbour(NULL)
, chunkX(x)
, chunkY(y)
, baseX(chunkX * CHUNKSIZE)
, baseY(chunkY * CHUNKSIZE)
, chunkBaseX((tile->coordX * TILESIZE) + baseX)
, chunkBaseY((tile->coordY * TILESIZE) + baseY)
{
    chunkMaterial = new Material();

    /// Textures
    if(!world->getTextureManager()->hasTexture("groundTexture", "textures/ground.png"))
        world->getTextureManager()->loadTexture("groundTexture", "textures/ground.png");

    if(!world->getTextureManager()->hasTexture("grassTexture", "textures/grass.png"))
        world->getTextureManager()->loadTexture("grassTexture", "textures/grass.png");

    if(!world->getTextureManager()->hasTexture("rockTexture", "textures/rock.png"))
        world->getTextureManager()->loadTexture("rockTexture", "textures/rock.png");

    if(!world->getTextureManager()->hasTexture("snowTexture", "textures/snowrocks.png"))
        world->getTextureManager()->loadTexture("snowTexture", "textures/snowrocks.png");

    textures[0] = world->getTextureManager()->getTexture("groundTexture");
    textures[1] = world->getTextureManager()->getTexture("grassTexture");
    textures[2] = world->getTextureManager()->getTexture("rockTexture");
    textures[3] = world->getTextureManager()->getTexture("snowTexture");

    for(int i = 0; i < MAX_TEXTURES; ++i)
    {
        world->getGLFunctions()->glActiveTexture(GL_TEXTURE0 + ShaderUnits::Texture1 + ToGLuint(i));

        QString uniformName = QString("layer%1Texture").arg(i);

        if(i == 0)
            uniformName = "baseTexture";

        chunkMaterial->setTextureUnitConfiguration(ShaderUnits::Texture1 + i, textures[i], world->getTextureManager()->getSampler(), uniformName.toLatin1());
    }

    /// Alphamaps
    for(int i = 0; i < ALPHAMAPS; ++i)
    {
        world->getGLFunctions()->glActiveTexture(GL_TEXTURE0 + ShaderUnits::Alphamap1 + ToGLuint(i));

        alphaMapsData[i] = new unsigned char[CHUNK_ARRAY_UC_SIZE];

        memset(alphaMapsData[i], 0, CHUNK_ARRAY_UC_SIZE);

        TexturePtr alphamap(new Texture(QOpenGLTexture::Target2D));
        alphamap->setSize(MAP_WIDTH / CHUNKS, MAP_HEIGHT / CHUNKS);
        alphamap->setAlphamap(alphaMapsData[i]);

        alphaMaps[i] = alphamap;

        QString alphaMapLayer = QString("layer%1Alpha").arg(i + 1);

        chunkMaterial->setTextureUnitConfiguration(ShaderUnits::Alphamap1 + i, alphaMaps[i], terrainSampler, alphaMapLayer.toLatin1());
    }

    /// Vertex Shading
    world->getGLFunctions()->glActiveTexture(GL_TEXTURE0 + ShaderUnits::VertexShading);

    vertexShadingData = new unsigned char[CHUNK_ARRAY_SIZE];

    memset(vertexShadingData, 0, CHUNK_ARRAY_SIZE);

    vertexShadingMap = TexturePtr(new Texture(QOpenGLTexture::Target2D));
    vertexShadingMap->setSize(MAP_WIDTH / CHUNKS, MAP_HEIGHT / CHUNKS);
    vertexShadingMap->setVertexShading(vertexShadingData);

    chunkMaterial->setTextureUnitConfiguration(ShaderUnits::VertexShading, vertexShadingMap, terrainSampler, "vertexShading");

    /// Terrain
    mapData = new float[CHUNK_ARRAY_UC_SIZE]; // chunk_array_size

    memset(mapData, 0, sizeof(float) * CHUNK_ARRAY_UC_SIZE);

    world->getGLFunctions()->glActiveTexture(GL_TEXTURE0 + ShaderUnits::Heightmap);

    terrainData->setSize(MAP_WIDTH / CHUNKS, MAP_HEIGHT / CHUNKS);
    terrainData->setHeightmap(mapData);

    chunkMaterial->setTextureUnitConfiguration(ShaderUnits::Heightmap, terrainData, terrainSampler, QByteArrayLiteral("heightMap"));

    initialize();
}

MapChunk::MapChunk(World* mWorld, MapTile* tile, QFile& file, int x, int y) // File based MapChunk
: world(mWorld)
, terrainSampler(tile->terrainSampler.data())
, terrainData(new Texture(QOpenGLTexture::Target2D))
, mapDataCache(NULL)
, vertexShadingMap(NULL)
, displaySubroutines(world->DisplayModeCount)
, highlight(false)
, selected(false)
, mapGeneration(false)
, mapScale(false)
, bottomNeighbour(NULL)
, leftNeighbour(NULL)
, chunkX(x)
, chunkY(y)
, baseX(chunkX * CHUNKSIZE)
, baseY(chunkY * CHUNKSIZE)
, chunkBaseX((tile->coordX * TILESIZE) + baseX)
, chunkBaseY((tile->coordY * TILESIZE) + baseY)
{
    chunkMaterial = new Material();

    // Load from file
    if(file.isOpen())
    {
        /// Load Textures
        for(int i = 0; i < MAX_TEXTURES; ++i)
        {
            QString texturePath = tile->getHeader().mcin->entries[chunkIndex()].mcnk->terrainOffset->textures[i];

            if(texturePath == QString())
            {
                textures[i] = TexturePtr(new Texture());

                continue;
            }

            QString textureName = QFileInfo(texturePath).baseName() + "Texture";

            if(!world->getTextureManager()->hasTexture(textureName, texturePath))
                world->getTextureManager()->loadTexture(textureName, texturePath);

            textures[i] = world->getTextureManager()->getTexture(textureName);
        }

        /// Load Alphamaps
        for(int i = 0; i < ALPHAMAPS; ++i)
            alphaMapsData[i] = tile->getHeader().mcin->entries[chunkIndex()].mcnk->terrainOffset->alphaMaps[i];

        /// Load Vertex Shading
        vertexShadingData = tile->getHeader().mcin->entries[chunkIndex()].mcnk->terrainOffset->vertexShading;

        /// Load Heightmap
        mapData = tile->getHeader().mcin->entries[chunkIndex()].mcnk->terrainOffset->height;
    }
    else
    {
        qWarning(QObject::tr("Loading default chunk for tile %1_%2 chunk %3_%4, because we couldn't open file!").arg(tile->coordX).arg(tile->coordY).arg(chunkX).arg(chunkY).toLatin1().data());

        /// Load Textures
        if(!world->getTextureManager()->hasTexture("groundTexture", "textures/ground.png"))
            world->getTextureManager()->loadTexture("groundTexture", "textures/ground.png");

        if(!world->getTextureManager()->hasTexture("grassTexture", "textures/grass.png"))
            world->getTextureManager()->loadTexture("grassTexture", "textures/grass.png");

        if(!world->getTextureManager()->hasTexture("rockTexture", "textures/rock.png"))
            world->getTextureManager()->loadTexture("rockTexture", "textures/rock.png");

        if(!world->getTextureManager()->hasTexture("snowTexture", "textures/snowrocks.png"))
            world->getTextureManager()->loadTexture("snowTexture", "textures/snowrocks.png");

        textures[0] = world->getTextureManager()->getTexture("groundTexture");
        textures[1] = world->getTextureManager()->getTexture("grassTexture");
        textures[2] = world->getTextureManager()->getTexture("rockTexture");
        textures[3] = world->getTextureManager()->getTexture("snowTexture");

        /// Load Alphamaps
        for(int i = 0; i < ALPHAMAPS; ++i)
        {
            alphaMapsData[i] = new unsigned char[CHUNK_ARRAY_UC_SIZE];

            memset(alphaMapsData[i], 0, CHUNK_ARRAY_UC_SIZE);
        }

        /// Load Vertex Shading
        vertexShadingData = new unsigned char[CHUNK_ARRAY_SIZE];

        memset(vertexShadingData, 0, CHUNK_ARRAY_SIZE);

        /// Load Heightmap
        mapData = new float[CHUNK_ARRAY_UC_SIZE]; // chunk_array_size

        memset(mapData, 0, sizeof(float) * CHUNK_ARRAY_UC_SIZE);
    }

    /// Set Textures
    for(int i = 0; i < MAX_TEXTURES; ++i)
    {
        world->getGLFunctions()->glActiveTexture(GL_TEXTURE0 + ShaderUnits::Texture1 + ToGLuint(i));

        QString uniformName = QString("layer%1Texture").arg(i);

        if(i == 0)
            uniformName = "baseTexture";

        chunkMaterial->setTextureUnitConfiguration(ShaderUnits::Texture1 + i, textures[i], world->getTextureManager()->getSampler(), uniformName.toLatin1());
    }

    /// Set Alphamaps
    for(int i = 0; i < ALPHAMAPS; ++i)
    {
        world->getGLFunctions()->glActiveTexture(GL_TEXTURE0 + ShaderUnits::Alphamap1 + ToGLuint(i));

        TexturePtr alphamap(new Texture(QOpenGLTexture::Target2D));
        alphamap->setSize(MAP_WIDTH / CHUNKS, MAP_HEIGHT / CHUNKS);
        alphamap->setAlphamap(alphaMapsData[i]);

        alphaMaps[i] = alphamap;

        QString alphaMapLayer = QString("layer%1Alpha").arg(i + 1);

        chunkMaterial->setTextureUnitConfiguration(ShaderUnits::Alphamap1 + i, alphaMaps[i], terrainSampler, alphaMapLayer.toLatin1());
    }

    /// Set Vertex Shading
    world->getGLFunctions()->glActiveTexture(GL_TEXTURE0 + ShaderUnits::VertexShading);

    vertexShadingMap = TexturePtr(new Texture(QOpenGLTexture::Target2D));
    vertexShadingMap->setSize(MAP_WIDTH / CHUNKS, MAP_HEIGHT / CHUNKS);
    vertexShadingMap->setVertexShading(vertexShadingData);

    chunkMaterial->setTextureUnitConfiguration(ShaderUnits::VertexShading, vertexShadingMap, terrainSampler, "vertexShading");

    /// Set Terrain
    world->getGLFunctions()->glActiveTexture(GL_TEXTURE0 + ShaderUnits::Heightmap);

    terrainData->setSize(MAP_WIDTH / CHUNKS, MAP_HEIGHT / CHUNKS);
    terrainData->setHeightmap(mapData);

    chunkMaterial->setTextureUnitConfiguration(ShaderUnits::Heightmap, terrainData, terrainSampler, QByteArrayLiteral("heightMap"));

    initialize();
}

MapChunk::~MapChunk()
{
    delete[] mapData;

    mapData = NULL;

    if(mapDataCache != NULL)
    {
        delete[] mapDataCache;

        mapDataCache = NULL;
    }

    /*delete[] vertexShadingData;

    vertexShadingData = NULL;

    for(int i = 0; i < ALPHAMAPS; ++i)
    {
        delete[] alphaMapsData[i];

        alphaMapsData[i] = NULL;
    }*/
}

void MapChunk::initialize()
{
    /// Create a Vertex Buffers
    const int maxTessellationLevel     = 64;
    const int trianglesPerHeightSample = 10;

    const int xDivisions = trianglesPerHeightSample * MAP_WIDTH  / CHUNKS / maxTessellationLevel;
    const int zDivisions = trianglesPerHeightSample * MAP_HEIGHT / CHUNKS / maxTessellationLevel;

    int patchCount = xDivisions * zDivisions;

    QVector<float> positionData(2 * patchCount); // 2 floats per vertex

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

    Mesh.createVertexArrayObject();
    Mesh.createBuffer(IMesh::Vertices, positionData.data(), positionData.size() * sizeof(float));
    Mesh.setNumFaces(patchCount);
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

    /// Fix NaN
    /*terrainData->bind();

    for(int x = 0; x < MAP_WIDTH / CHUNKS; ++x)
    {
        for(int y = 0; y < MAP_HEIGHT / CHUNKS; ++y)
        {
            if(MathHelper::isNaN(getHeight(x, y)))
            {
                mapData[(MAP_WIDTH / CHUNKS) * y + x] = 0.0f;

                terrainData->setHeight(0.0f, QVector2D(QPoint(x, y)));
            }
        }
    }

    chunkMaterial->setTextureUnitConfiguration(ShaderUnits::Heightmap, terrainData, terrainSampler, QByteArrayLiteral("heightMap"));*/

    /// Fix height
    terrainData->bind();

    for(int x = 0; x < MAP_WIDTH / CHUNKS; ++x)
    {
        for(int y = 0; y < MAP_HEIGHT / CHUNKS; ++y)
        {
            if(x < 30 && y < 30)
            {
                if(x % 2 == 0 || y % 2 == 0)
                    continue;

                mapData[(MAP_WIDTH / CHUNKS) * y + x] = x;

                terrainData->setHeight(x, QVector2D(QPoint(x, y)));
            }
        }
    }

    chunkMaterial->setTextureUnitConfiguration(ShaderUnits::Heightmap, terrainData, terrainSampler, QByteArrayLiteral("heightMap"));
}

void MapChunk::draw(QOpenGLShaderProgram* shader)
{
    chunkMaterial->bind(shader);

    shader->setUniformValue("highlight", highlight);
    shader->setUniformValue("selected",  selected);

    // Get subroutine indices
    for(int i = 0; i < world->DisplayModeCount; ++i)
        displaySubroutines[i] = world->getGLFunctions()->glGetSubroutineIndex(shader->programId(), GL_FRAGMENT_SHADER, world->displayName(i).toLatin1());

    shader->setUniformValue("baseX", chunkBaseX);
    shader->setUniformValue("baseY", chunkBaseY);

    shader->setUniformValue("chunkX", chunkX);
    shader->setUniformValue("chunkY", chunkY);

    shader->setUniformValue("chunkLines", app().getSetting("chunkLines", false).toBool());

    shader->setUniformValue("textureScaleOption", app().getSetting("textureScaleOption", 0).toInt());
    shader->setUniformValue("textureScaleFar",    app().getSetting("textureScaleFar",    0.4f).toFloat());
    shader->setUniformValue("textureScaleNear",   app().getSetting("textureScaleNear",   0.4f).toFloat());

    // Set the fragment shader display mode subroutine
    world->getGLFunctions()->glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &getDisplaySubroutines());

    if(world->displayMode() != world->Hidden)
    {
        // Render the quad as a patch
        {
            Mesh.bind();
            Mesh.createAttributeArray(IMesh::Vertices, shader, "vertexPosition", GL_FLOAT, 0, 2);

            shader->setPatchVertexCount(1);

            world->getGLFunctions()->glDrawArrays(GL_PATCHES, 0, Mesh.getNumFaces());
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

    return mapData[index];
}

const float MapChunk::getHeight(const int& x, const int& y) const
{
    int X = x % MAP_WIDTH;
    int Y = y % MAP_HEIGHT;

    int index = (Y * MAP_WIDTH / CHUNKS) + X;    

    return mapData[index];
}

const float MapChunk::getHeightFromWorld(float x, float z)
{
    int X = horizToHMapSize(x) % MAP_WIDTH;
    int Y = horizToHMapSize(z) % MAP_HEIGHT;

    X -= chunkX * (MAP_WIDTH / CHUNKS);
    Y -= chunkY * (MAP_HEIGHT / CHUNKS);

    int index = (Y * MAP_WIDTH / CHUNKS) + X;

    return mapData[index];
}

const float MapChunk::getMapData(const int& index) const
{
    return mapData[index];
}

const int MapChunk::chunkIndex() const
{
    return (chunkY * CHUNKS) + chunkX;
}

bool MapChunk::changeTerrain(float x, float z, float change)
{    
    bool changed = false;

    const Brush* brush = world->getBrush();

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

                            if(index < 0 || index >= CHUNK_ARRAY_UC_SIZE || X < 0 || Y < 0 || X >= (MAP_WIDTH / CHUNKS) || Y >= (MAP_HEIGHT / CHUNKS))
                                continue;

                            if(world->getTerrainMaximumState() && mapData[index] >= world->getTerrainMaximumHeight())
                                continue;

                            float changeFormula = 0.0f;

                            switch(brush->BrushTypes().shaping)
                            {
                                case Brush::ShapingType::Linear:
                                default:
                                    changeFormula = (change * (1.0f - dist / brush->OuterRadius())) * brush->calcLossyMultiplier(dist);
                                    break;

                                case Brush::ShapingType::Flat:
                                    changeFormula = change * brush->calcLossyMultiplier(dist);
                                    break;

                                case Brush::ShapingType::Smooth:
                                    changeFormula = (change / (1.0f + dist / brush->OuterRadius())) * brush->calcLossyMultiplier(dist);
                                    break;

                                case Brush::ShapingType::Polynomial:
                                    changeFormula = (change * (pow(dist / brush->OuterRadius(), 2) + dist / brush->OuterRadius() + 1.0f)) * brush->calcLossyMultiplier(dist);
                                    break;

                                case Brush::ShapingType::Trigonometric:
                                    changeFormula = (change * cos(dist / brush->OuterRadius())) * brush->calcLossyMultiplier(dist);
                                    break;
                            }

                            if(MathHelper::isNaN(changeFormula))
                                continue;

                            if(world->getTerrainMaximumState() && mapData[index] + changeFormula > world->getTerrainMaximumHeight())
                                mapData[index] = world->getTerrainMaximumHeight();
                            else
                                mapData[index] += changeFormula;

                            //if(X == 0 && chunkX != 0)
                                //mapData[index] = world->getTileAt(x, z)->getChunk(chunkX - 1, chunkY)->getHeight(MAP_WIDTH / CHUNKS - 1, Y) + 0.001f;

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

        chunkMaterial->setTextureUnitConfiguration(ShaderUnits::Heightmap, terrainData, terrainSampler, QByteArrayLiteral("heightMap"));

        if(bottomNeighbour && !horizontalData.isEmpty())
            bottomNeighbour->setBorder(Horizontal, horizontalData);

        if(leftNeighbour && !verticalData.isEmpty())
            leftNeighbour->setBorder(Vertical, verticalData);
    }

    return changed;
}

bool MapChunk::flattenTerrain(float x, float z, float y, float change)
{
    bool changed = false;

    const Brush* brush = world->getBrush();

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

                            if(index < 0 || index >= CHUNK_ARRAY_UC_SIZE || X < 0 || Y < 0 || X >= (MAP_WIDTH / CHUNKS) || Y >= (MAP_HEIGHT / CHUNKS))
                                continue;

                            if(world->getTerrainMaximumState() && mapData[index] >= world->getTerrainMaximumHeight())
                                continue;

                            float changeFormula = 0.0f;

                            switch(brush->BrushTypes().smoothing)
                            {
                                case Brush::SmoothingType::Linear:
                                default:
                                    {
                                        changeFormula = 1.0f - ((1.0f - change) * (1.0f - dist / brush->OuterRadius())) * brush->calcLossyMultiplier(dist);
                                        changeFormula = changeFormula * mapData[index] + (1 - changeFormula) * y;
                                    }
                                    break;

                                case Brush::SmoothingType::Flat:
                                    changeFormula = change * mapData[index] + (1 - change) * y;
                                    break;

                                case Brush::SmoothingType::Smooth:
                                    {
                                        changeFormula = 1.0f - (pow(1.0f - change, 1.0f + dist / brush->OuterRadius())) * brush->calcLossyMultiplier(dist);
                                        changeFormula = changeFormula * mapData[index] + (1 - changeFormula) * y;
                                    }
                                    break;
                            }

                            if(MathHelper::isNaN(changeFormula))
                                continue;

                            mapData[index] = changeFormula;

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

        chunkMaterial->setTextureUnitConfiguration(ShaderUnits::Heightmap, terrainData, terrainSampler, QByteArrayLiteral("heightMap"));

        if(bottomNeighbour && !horizontalData.isEmpty())
            bottomNeighbour->setBorder(Horizontal, horizontalData);

        if(leftNeighbour && !verticalData.isEmpty())
            leftNeighbour->setBorder(Vertical, verticalData);
    }

    return changed;
}

bool MapChunk::blurTerrain(float x, float z, float change)
{
    bool changed = false;

    const Brush* brush = world->getBrush();

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

                            if(index < 0 || index >= CHUNK_ARRAY_UC_SIZE || X < 0 || Y < 0 || X >= (MAP_WIDTH / CHUNKS) || Y >= (MAP_HEIGHT / CHUNKS))
                                continue;

                            if(world->getTerrainMaximumState() && mapData[index] >= world->getTerrainMaximumHeight())
                                continue;

                            float changeFormula = 0.0f;

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
                                        changeFormula = 1.0f - ((1.0f - change) * (1.0f - dist / brush->OuterRadius())) * brush->calcLossyMultiplier(dist);
                                        changeFormula = changeFormula * mapData[index] + (1 - changeFormula) * h;
                                    }
                                    break;

                                case Brush::SmoothingType::Flat: // Flat
                                    changeFormula = change * mapData[index] + (1 - change) * h;
                                    break;

                                case Brush::SmoothingType::Smooth: // Smooth
                                    {
                                        changeFormula = 1.0f - (pow(1.0f - change, (1.0f + dist / brush->OuterRadius()))) * brush->calcLossyMultiplier(dist);
                                        changeFormula = changeFormula * mapData[index] + (1 - changeFormula) * h;
                                    }
                                    break;
                            }

                            if(MathHelper::isNaN(changeFormula))
                                continue;

                            mapData[index] = changeFormula;

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

        chunkMaterial->setTextureUnitConfiguration(ShaderUnits::Heightmap, terrainData, terrainSampler, QByteArrayLiteral("heightMap"));

        if(bottomNeighbour && !horizontalData.isEmpty())
            bottomNeighbour->setBorder(Horizontal, horizontalData);

        if(leftNeighbour && !verticalData.isEmpty())
            leftNeighbour->setBorder(Vertical, verticalData);
    }

    return changed;
}

bool MapChunk::paintTerrain(float x, float z, float flow, TexturePtr texture)
{
    bool changed = false;

    const Brush* brush = world->getBrush();

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

                // destroy textures[i]

                textures[i] = texture;

                QString uniformName = QString("layer%1Texture").arg(i);

                chunkMaterial->setTextureUnitConfiguration(ShaderUnits::Texture1 + i, textures[i], world->getTextureManager()->getSampler(), uniformName.toLatin1());

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

                            if(index < 0 || index >= CHUNK_ARRAY_UC_SIZE || X < 0 || Y < 0 || X >= (MAP_WIDTH / CHUNKS) || Y >= (MAP_HEIGHT / CHUNKS))
                                continue;

                            if(textureIndex > 0 && world->getPaintMaximumState() && ToFloat(alphaMapsData[textureIndex - 1][index]) >= world->getPaintMaximumAlpha() * 255.0f)
                                continue;

                            float lossyMultiplier = brush->calcLossyMultiplier(dist);

                            switch(brush->BrushTypes().texturing)
                            {
                                case Brush::TexturingType::Solid:
                                default:
                                    {
                                        if(textureIndex > 0)
                                        {
                                            --textureIndex;

                                            alphaMapsData[textureIndex][index] = ToUChar(qMax(qMin(ToFloat(alphaMapsData[textureIndex][index]) + (((255.0f - ToFloat(alphaMapsData[textureIndex][index])) * flow * 3) * lossyMultiplier), 255.0f), 0.0f));

                                            changing.append(qMakePair<unsigned char, QVector2D>(alphaMapsData[textureIndex][index], QVector2D(X, Y)));

                                            ++textureIndex;
                                        }

                                        for(int i = textureIndex; i < ALPHAMAPS; ++i)
                                        {
                                            if(ToFloat(alphaMapsData[i][index]) > 0.0f)
                                            {
                                                alphaMapsData[i][index] = ToUChar(qMax(qMin(ToFloat(alphaMapsData[i][index]) - (((255.0f - ToFloat(alphaMapsData[i][index])) * flow * 3) * lossyMultiplier), 255.0f), 0.0f));

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

            chunkMaterial->setTextureUnitConfiguration(ShaderUnits::Alphamap1 + textureIndex, alphaMaps[textureIndex], terrainSampler, uniformName.toLatin1());

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

            chunkMaterial->setTextureUnitConfiguration(ShaderUnits::Alphamap1 + i, alphaMaps[i], terrainSampler, uniformName.toLatin1());
        }
    }

    return changed;
}

bool MapChunk::paintVertexShading(float x, float z, float flow, QColor& color)
{
    bool changed = false;

    const Brush* brush = world->getBrush();

    float xdiff = baseX - x + CHUNKSIZE / 2;
    float ydiff = baseY - z + CHUNKSIZE / 2;

    float dist = sqrt(pow(xdiff, 2) + pow(ydiff, 2));

    if(dist > (brush->OuterRadius() + CHUNK_DIAMETER))
        return changed;

    QList<QPair<QVector<unsigned char>, QVector2D>> changing;

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

                            float lossyMultiplier = brush->calcLossyMultiplier(dist);

                            switch(brush->BrushTypes().vertexShading)
                            {
                                case Brush::TexturingType::Solid:
                                default:
                                    {
                                        vertexShadingData[index]     = ToUChar(qMax(qMin(MathHelper::closerTo(vertexShadingData[index],     color.redF(),   ((255.0f - vertexShadingData[index])     * flow * 3) * lossyMultiplier), 255.0f), 0.0f));
                                        vertexShadingData[index + 1] = ToUChar(qMax(qMin(MathHelper::closerTo(vertexShadingData[index + 1], color.greenF(), ((255.0f - vertexShadingData[index + 1]) * flow * 3) * lossyMultiplier), 255.0f), 0.0f));
                                        vertexShadingData[index + 2] = ToUChar(qMax(qMin(MathHelper::closerTo(vertexShadingData[index + 2], color.blueF(),  ((255.0f - vertexShadingData[index + 2]) * flow * 3) * lossyMultiplier), 255.0f), 0.0f));

                                        if(!world->getPaintMaximumState() || (world->getPaintMaximumState() && ToFloat(vertexShadingData[index + 3]) < world->getPaintMaximumAlpha() * 255.0f))
                                            vertexShadingData[index + 3] = ToUChar(qMax(qMin(MathHelper::closerTo(vertexShadingData[index + 3], color.alphaF(), ((255.0f - vertexShadingData[index + 3]) * flow * 3) * lossyMultiplier), 255.0f), 0.0f));

                                        QVector<unsigned char> dataContainer;
                                        dataContainer.append(vertexShadingData[index]);     // R
                                        dataContainer.append(vertexShadingData[index + 1]); // G
                                        dataContainer.append(vertexShadingData[index + 2]); // B
                                        dataContainer.append(vertexShadingData[index + 3]); // A

                                        changing.append(qMakePair<QVector<unsigned char>, QVector2D>(dataContainer, QVector2D(X, Y)));
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
        vertexShadingMap->bind();

        for(int i = 0; i < changing.count(); ++i)
        {
            unsigned char data[4];

            for(int j = 0; j < 4; ++j)
                data[j] = changing.value(i).first.at(j);

            vertexShadingMap->setVertexShade(data, changing.value(i).second);
        }

        chunkMaterial->setTextureUnitConfiguration(ShaderUnits::VertexShading, vertexShadingMap, terrainSampler, "vertexShading");
    }

    return changed;
}

void MapChunk::updateNeighboursHeightmapData()
{
    if(leftNeighbour)
    {
        for(int y = 0; y < MAP_HEIGHT / CHUNKS; ++y)
            mapData[y * MAP_WIDTH / CHUNKS] = leftNeighbour->getHeight(MAP_WIDTH / CHUNKS - 1, y);
    }
}

void MapChunk::generation(bool accepted)
{
    mapGeneration = false;

    if(accepted)
    {
        delete[] mapDataCache;

        mapDataCache = NULL;
    }
    else
    {
        delete[] mapData; // delete generated mapData, user don't want to use them

        mapData = new float[CHUNK_ARRAY_UC_SIZE];

        memcpy(mapData, mapDataCache, sizeof(float) * CHUNK_ARRAY_UC_SIZE); // copy cache to mapData

        // delete cache
        delete[] mapDataCache;

        mapDataCache = NULL;

        // set heightmap
        world->getGLFunctions()->glActiveTexture(GL_TEXTURE0 + ShaderUnits::Heightmap);

        terrainData->bind();
        terrainData->setHeightmap(mapData);

        chunkMaterial->setTextureUnitConfiguration(ShaderUnits::Heightmap, terrainData, terrainSampler, QByteArrayLiteral("heightMap"));
    }
}

void MapChunk::heightmapSettings(bool accepted)
{
    mapScale      = false;
    mapGeneration = false;

    if(accepted) // set scale or import
    {
        delete[] mapDataCache;

        mapDataCache = NULL;
    }
    else
    {
        delete[] mapData; // delete scaled mapData or imported, user don't want to use them

        mapData = new float[CHUNK_ARRAY_UC_SIZE];

        memcpy(mapData, mapDataCache, sizeof(float) * CHUNK_ARRAY_UC_SIZE); // copy cache to mapData

        // delete cache
        delete[] mapDataCache;

        mapDataCache = NULL;

        // set heightmap
        world->getGLFunctions()->glActiveTexture(GL_TEXTURE0 + ShaderUnits::Heightmap);

        terrainData->bind();
        terrainData->setHeightmap(mapData);

        chunkMaterial->setTextureUnitConfiguration(ShaderUnits::Heightmap, terrainData, terrainSampler, QByteArrayLiteral("heightMap"));
    }
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

                world->getGLFunctions()->glActiveTexture(GL_TEXTURE0 + ShaderUnits::Texture1 + ToGLuint(index - 1));

                uniformName = QString("layer%1Texture").arg(index - 1);

                if(index - 1 == 0)
                    uniformName = "baseTexture";

                chunkMaterial->setTextureUnitConfiguration(ShaderUnits::Texture1 + index - 1, textures[index - 1], world->getTextureManager()->getSampler(), uniformName.toLatin1());
            }
            break;

        case false:
            {
                TexturePtr temp = textures[index + 1];

                textures[index + 1] = textures[index];
                textures[index]     = temp;

                world->getGLFunctions()->glActiveTexture(GL_TEXTURE0 + ShaderUnits::Texture1 + ToGLuint(index + 1));

                uniformName = QString("layer%1Texture").arg(index + 1);

                chunkMaterial->setTextureUnitConfiguration(ShaderUnits::Texture1 + index + 1, textures[index + 1], world->getTextureManager()->getSampler(), uniformName.toLatin1());
            }
            break;
    }

    world->getGLFunctions()->glActiveTexture(GL_TEXTURE0 + ShaderUnits::Texture1 + ToGLuint(index));

    uniformName = QString("layer%1Texture").arg(index);

    if(index == 0)
        uniformName = "baseTexture";

    chunkMaterial->setTextureUnitConfiguration(ShaderUnits::Texture1 + index, textures[index], world->getTextureManager()->getSampler(), uniformName.toLatin1());
}

void MapChunk::deleteAlphaMap(int index)
{
    textures[index] = TexturePtr(new Texture());

    QString uniformName = QString("layer%1Texture").arg(index);

    chunkMaterial->setTextureUnitConfiguration(ShaderUnits::Texture1 + index, textures[index], world->getTextureManager()->getSampler(), uniformName.toLatin1());

    memset(alphaMapsData[index], 0, MAP_WIDTH / CHUNKS * MAP_HEIGHT / CHUNKS); // clear alpha map

    alphaMaps[index]->setAlphamap(alphaMapsData[index]);

    uniformName = QString("layer%1Alpha").arg(index);

    chunkMaterial->setTextureUnitConfiguration(ShaderUnits::Alphamap1 + index, alphaMaps[index], terrainSampler, uniformName.toLatin1());
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

                    int index = (((MAP_HEIGHT / CHUNKS - 1) * (MAP_WIDTH / CHUNKS)) + data.at(i).first);

                    mapData[index] = data.at(i).second;
                }
            }
            break;

        case Vertical:
            {
                for(int i = 0; i < data.count(); ++i)
                {
                    terrainData->setHeight(data.at(i).second, QVector2D(MAP_HEIGHT / CHUNKS - 1, data.at(i).first));

                    int index = ((data.at(i).first * (MAP_WIDTH / CHUNKS)) + (MAP_HEIGHT / CHUNKS - 1));

                    mapData[index] = data.at(i).second;
                }
            }
            break;
    }

    chunkMaterial->setTextureUnitConfiguration(ShaderUnits::Heightmap, terrainData, terrainSampler, QByteArrayLiteral("heightMap"));
}

void MapChunk::setBottomNeighbour(MapChunk* chunk)
{
    bottomNeighbour = chunk;
}

void MapChunk::setLeftNeighbour(MapChunk* chunk)
{
    leftNeighbour = chunk;
}

void MapChunk::setHighlight(bool on)
{
    if(highlight == on)
        return;

    highlight = on;
}

void MapChunk::setSelected(bool on)
{
    if(selected == on)
        return;

    selected = on;
}

void MapChunk::setHeightmap(float* data)
{
    for(int i = 0; i < CHUNK_ARRAY_UC_SIZE; ++i)
        mapData[i] = data[i];

    world->getGLFunctions()->glActiveTexture(GL_TEXTURE0 + ShaderUnits::Heightmap);

    terrainData->bind();
    terrainData->setHeightmap(mapData);

    chunkMaterial->setTextureUnitConfiguration(ShaderUnits::Heightmap, terrainData, terrainSampler, QByteArrayLiteral("heightMap"));
}

void MapChunk::setGeneratedHeightmap(float* tileData)
{
    if(!mapGeneration)
    {
        mapGeneration = true;

        // check if mapDataCache is allocated somehow(?!)
        if(mapDataCache != NULL)
            delete[] mapDataCache;

        mapDataCache = new float[CHUNK_ARRAY_UC_SIZE];

        memcpy(mapDataCache, mapData, sizeof(float) * CHUNK_ARRAY_UC_SIZE); // copy mapData to cache
    }

    memcpy(mapData, tileData, sizeof(float) * CHUNK_ARRAY_UC_SIZE); // copy generated map to mapData

    world->getGLFunctions()->glActiveTexture(GL_TEXTURE0 + ShaderUnits::Heightmap);

    terrainData->bind();
    terrainData->setHeightmap(mapData);

    chunkMaterial->setTextureUnitConfiguration(ShaderUnits::Heightmap, terrainData, terrainSampler, QByteArrayLiteral("heightMap"));
}

void MapChunk::setHeightmapScale(float scale)
{
    if(!mapScale)
    {
        mapScale = true;

        // check if mapDataCache is allocated somehow(?!)
        if(mapDataCache != NULL)
            delete[] mapDataCache;

        mapDataCache = new float[CHUNK_ARRAY_UC_SIZE];

        memcpy(mapDataCache, mapData, sizeof(float) * CHUNK_ARRAY_UC_SIZE); // copy mapData to cache
    }
    else
        memcpy(mapData, mapDataCache, sizeof(float) * CHUNK_ARRAY_UC_SIZE); // clear scale from previous

    for(int i = 0; i < CHUNK_ARRAY_UC_SIZE; ++i)
        mapData[i] *= scale;

    world->getGLFunctions()->glActiveTexture(GL_TEXTURE0 + ShaderUnits::Heightmap);

    terrainData->bind();
    terrainData->setHeightmap(mapData);

    chunkMaterial->setTextureUnitConfiguration(ShaderUnits::Heightmap, terrainData, terrainSampler, QByteArrayLiteral("heightMap"));
}

void MapChunk::save(MCNK* chunk)
{
    chunk->areaID  = 0;
    chunk->doodads = 0;
    chunk->flags   = 0;
    chunk->indexX  = chunkX;
    chunk->indexY  = chunkY;
    chunk->layers  = 0;

    chunk->terrainOffset = new MCVT;

    // Heightmap
    for(int i = 0; i < CHUNK_ARRAY_UC_SIZE; ++i) // chunk_array_size
        chunk->terrainOffset->height[i] = mapData[i];

    // Alphamaps
    for(int i = 0; i < ALPHAMAPS; ++i)
    {
        for(int j = 0; j < CHUNK_ARRAY_UC_SIZE; ++j)
            chunk->terrainOffset->alphaMaps[i][j] = alphaMapsData[i][j];
    }

    // Vertex Shading
    for(int i = 0; i < CHUNK_ARRAY_SIZE; ++i)
        chunk->terrainOffset->vertexShading[i] = vertexShadingData[i];

    // Textures
    for(int i = 0; i < MAX_TEXTURES; ++i)
        chunk->terrainOffset->textures[i] = textures[i]->getPath();
}