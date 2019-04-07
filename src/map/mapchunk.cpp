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
#include "mapobject.h"

#include <QImage>
#include <QOpenGLPixelTransferOptions>
#include <QMultiMap>
#include <QFileInfo>

MapChunk::MapChunk(World* mWorld, MapTile* tile, int x, int y) // Cache MapChunk
: world(mWorld)
, terrainSampler(tile->terrainSampler.data())
, terrainData(new Texture(QOpenGLTexture::Target2D))
, mapDataCache(NULL)
, textureArray(new Texture(QOpenGLTexture::Target2DArray))
, depthTextureArray(new Texture(QOpenGLTexture::Target2DArray))
, alphaArray(new Texture(QOpenGLTexture::Target2DArray))
, vertexShadingMap(NULL)
, vertexLightingMap(NULL)
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

    textureArray->setSize(1024, 1024);
    depthTextureArray->setSize(1024, 1024);

    textureArray->setLayers(4);
    depthTextureArray->setLayers(4);
    alphaArray->setLayers(3);

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

    /* !!! TODO, wait for Qt fix then QOpenGLTexture integration with glTexStorage3D !!! */
    /*QOpenGLPixelTransferOptions uploadOptions;
    uploadOptions.setAlignment(1);*/

    world->getGLFunctions()->glActiveTexture(GL_TEXTURE0 + ShaderUnits::Textures);

    textureArray->bind();

    world->getGLFunctions()->glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8, MAX_TEXTURE_SIZE, MAX_TEXTURE_SIZE, MAX_TEXTURES);

    unsigned char* texture_data = (unsigned char*)malloc(MAX_TEXTURE_SIZE * MAX_TEXTURE_SIZE * 4 * MAX_TEXTURES);

    for(int i = 0; i < MAX_TEXTURES; ++i)
    {
        if(textures[i]->isNull())
            continue;

        QImage texture = QImage(textures[i]->getPath()).convertToFormat(QImage::Format_RGBA8888);

        if(texture.width() > MAX_TEXTURE_SIZE || texture.height() > MAX_TEXTURE_SIZE)
            qCritical() << QObject::tr("Texture is larger than") << MAX_TEXTURE_SIZE << "px";

        if(texture.width() == 0 || texture.height() == 0)
        {
            qWarning() << QObject::tr("Texture is empty! ID: ") << i;

            continue;
        }

        for(int j = 0; j < MAX_TEXTURE_SIZE / texture.width() * MAX_TEXTURE_SIZE / texture.height(); ++j)
            memcpy(&texture_data[MAX_TEXTURE_SIZE * MAX_TEXTURE_SIZE * 4 * i + texture.width() * texture.height() * 4 * j], &texture.bits()[0], texture.width() * texture.height() * 4);
    }

    world->getGLFunctions()->glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, MAX_TEXTURE_SIZE, MAX_TEXTURE_SIZE, MAX_TEXTURES, GL_RGBA, GL_UNSIGNED_BYTE, texture_data);//textureArray->setData(0, i, QOpenGLTexture::RGBA, QOpenGLTexture::UInt8, textures[i]->getImage().bits(), &uploadOptions);

    chunkMaterial->setTextureUnitConfiguration(ShaderUnits::Textures, textureArray, world->getTextureManager()->getSampler(), "textures");

    free(texture_data); // dealloc

    /// Depth Textures
    depthTextures[0] = world->getTextureManager()->getDepthTexture("groundTexture");
    depthTextures[1] = world->getTextureManager()->getDepthTexture("grassTexture");
    depthTextures[2] = world->getTextureManager()->getDepthTexture("rockTexture");
    depthTextures[3] = world->getTextureManager()->getDepthTexture("snowTexture");

    world->getGLFunctions()->glActiveTexture(GL_TEXTURE0 + ShaderUnits::DepthTextures);

    depthTextureArray->bind();

    world->getGLFunctions()->glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8, MAX_TEXTURE_SIZE, MAX_TEXTURE_SIZE, MAX_TEXTURES);

    unsigned char* depth_data = (unsigned char*)malloc(MAX_TEXTURE_SIZE * MAX_TEXTURE_SIZE * 4 * MAX_TEXTURES);

    for(int i = 0; i < MAX_TEXTURES; ++i)
    {
        if(depthTextures[i]->isNull())
            continue;

        QImage depthTex = QImage(depthTextures[i]->getPath()).convertToFormat(QImage::Format_RGBA8888);

        if(depthTex.width() > MAX_TEXTURE_SIZE || depthTex.height() > MAX_TEXTURE_SIZE)
            qCritical() << QObject::tr("Depth texture is larger than") << MAX_TEXTURE_SIZE << "px";

        if(depthTex.width() == 0 || depthTex.height() == 0)
        {
            qWarning() << QObject::tr("Depth texture is empty! ID: ") << i;

            continue;
        }

        for(int j = 0; j < MAX_TEXTURE_SIZE / depthTex.width() * MAX_TEXTURE_SIZE / depthTex.height(); ++j)
            memcpy(&depth_data[MAX_TEXTURE_SIZE * MAX_TEXTURE_SIZE * 4 * i + depthTex.width() * depthTex.height() * 4 * j], &depthTex.bits()[0], depthTex.width() * depthTex.height() * 4);
    }

    world->getGLFunctions()->glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, MAX_TEXTURE_SIZE, MAX_TEXTURE_SIZE, MAX_TEXTURES, GL_RGBA, GL_UNSIGNED_BYTE, depth_data);

    chunkMaterial->setTextureUnitConfiguration(ShaderUnits::DepthTextures, depthTextureArray, world->getTextureManager()->getSampler(), "depthTextures");

    free(depth_data); // dealloc

    /// Alphamaps
    for(int i = 0; i < ALPHAMAPS; ++i)
    {
        world->getGLFunctions()->glActiveTexture(GL_TEXTURE0 + ShaderUnits::Alphamap1 + ToGLuint(i));

        alphaMapsData[i] = new unsigned char[CHUNK_ARRAY_UC_SIZE];

        memset(alphaMapsData[i], 0, CHUNK_ARRAY_UC_SIZE);

        TexturePtr alphamap(new Texture(QOpenGLTexture::Target2D));
        alphamap->setSize(TILE_WIDTH / CHUNKS, TILE_HEIGHT / CHUNKS);
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
    vertexShadingMap->setSize(TILE_WIDTH / CHUNKS, TILE_HEIGHT / CHUNKS);
    vertexShadingMap->setVertexShading(vertexShadingData);

    chunkMaterial->setTextureUnitConfiguration(ShaderUnits::VertexShading, vertexShadingMap, terrainSampler, "vertexShading");

    /// Vertex Lighting
    world->getGLFunctions()->glActiveTexture(GL_TEXTURE0 + ShaderUnits::VertexLighting);

    vertexLightingData = new unsigned char[CHUNK_ARRAY_SIZE];

    memset(vertexLightingData, 0, CHUNK_ARRAY_SIZE);

    vertexLightingMap = TexturePtr(new Texture(QOpenGLTexture::Target2D));
    vertexLightingMap->setSize(TILE_WIDTH / CHUNKS, TILE_HEIGHT / CHUNKS);
    vertexLightingMap->setVertexShading(vertexLightingData);

    chunkMaterial->setTextureUnitConfiguration(ShaderUnits::VertexLighting, vertexLightingMap, terrainSampler, "vertexLighting");

    /// Texture scale
    for(int i = 0; i < MAX_TEXTURES; ++i)
    {
        textureScaleFar[i]    = app().getSetting("textureScaleFarSlider", 0.4).toFloat();
        textureScaleNear[i]   = app().getSetting("textureScaleNearSlider", 0.4).toFloat();
        textureScaleOption[i] = (TextureScaleOption)app().getSetting("textureScaleOption", 0).toInt();
    }

    /// Automatic texture
    for(int i = 0; i < MAX_TEXTURES - 1; ++i)
    {
        automaticTexture[i]      = false;
        automaticTextureStart[i] = 0.0f;
        automaticTextureEnd[i]   = 0.0f;
    }

    /// Terrain
    mapData = new float[CHUNK_ARRAY_UC_SIZE]; // chunk_array_size

    memset(mapData, 0, sizeof(float) * CHUNK_ARRAY_UC_SIZE);

    world->getGLFunctions()->glActiveTexture(GL_TEXTURE0 + ShaderUnits::Heightmap);

    terrainData->setSize(TILE_WIDTH / CHUNKS, TILE_HEIGHT / CHUNKS);
    terrainData->setHeightmap(mapData);

    chunkMaterial->setTextureUnitConfiguration(ShaderUnits::Heightmap, terrainData, terrainSampler, QByteArrayLiteral("heightMap"));

    initialize();
}

MapChunk::MapChunk(World* mWorld, MapTile* tile, QFile& file, int x, int y) // File based MapChunk
: world(mWorld)
, terrainSampler(tile->terrainSampler.data())
, terrainData(new Texture(QOpenGLTexture::Target2D))
, mapDataCache(NULL)
, textureArray(new Texture(QOpenGLTexture::Target2DArray))
, depthTextureArray(new Texture(QOpenGLTexture::Target2DArray))
, alphaArray(new Texture(QOpenGLTexture::Target2DArray))
, vertexShadingMap(NULL)
, vertexLightingMap(NULL)
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

    textureArray->setSize(1024, 1024);
    depthTextureArray->setSize(1024, 1024);

    textureArray->setLayers(4);
    depthTextureArray->setLayers(4);
    alphaArray->setLayers(3);

    // Load from file
    if(file.isOpen())
    {
        /// Load Textures
        for(int i = 0; i < MAX_TEXTURES; ++i)
        {
            QString texturePath = tile->getHeader().mcin->entries[chunkIndex()].mcnk->terrainOffset->textures[i];

            /*if(i == 0)
                texturePath = "textures/KLS_GRASS04_1024.png";
            else if(i == 1)
                texturePath = "textures/6ng_grass03_1024.png";
            else if(i == 2)
                texturePath = "textures/KLS_Rock05_1024.png";
            else
                texturePath = "textures/V4W_CLIFF01_1024.png";*/

            if(texturePath == QString())
            {
                textures[i]      = TexturePtr(new Texture());
                depthTextures[i] = TexturePtr(new Texture());

                continue;
            }

            QString textureName = QFileInfo(texturePath).baseName() + "Texture";

            if(!world->getTextureManager()->hasTexture(textureName, texturePath))
                world->getTextureManager()->loadTexture(textureName, texturePath, true);

            textures[i]      = world->getTextureManager()->getTexture(textureName, texturePath);
            depthTextures[i] = world->getTextureManager()->getDepthTexture(textureName, texturePath); /// Load Depth Textures
        }

        /// Load Alphamaps
        for(int i = 0; i < ALPHAMAPS; ++i)
            alphaMapsData[i] = tile->getHeader().mcin->entries[chunkIndex()].mcnk->terrainOffset->alphaMaps[i];

        /// Load Vertex Shading
        vertexShadingData = tile->getHeader().mcin->entries[chunkIndex()].mcnk->terrainOffset->vertexShading;

        /// Vertex Lighting
        world->getGLFunctions()->glActiveTexture(GL_TEXTURE0 + ShaderUnits::VertexLighting);

        vertexLightingData = new unsigned char[CHUNK_ARRAY_SIZE];

        memset(vertexLightingData, 127, CHUNK_ARRAY_SIZE);

        vertexLightingMap = TexturePtr(new Texture(QOpenGLTexture::Target2D));
        vertexLightingMap->setSize(TILE_WIDTH / CHUNKS, TILE_HEIGHT / CHUNKS);
        vertexLightingMap->setVertexShading(vertexLightingData);

        chunkMaterial->setTextureUnitConfiguration(ShaderUnits::VertexLighting, vertexLightingMap, terrainSampler, "vertexLighting");

        /// Load Texture scale
        for(int i = 0; i < MAX_TEXTURES; ++i)
        {
            textureScaleFar[i]    = tile->getHeader().mcin->entries[chunkIndex()].mcnk->terrainOffset->textureScaleFar[i];
            textureScaleNear[i]   = tile->getHeader().mcin->entries[chunkIndex()].mcnk->terrainOffset->textureScaleNear[i];
            textureScaleOption[i] = tile->getHeader().mcin->entries[chunkIndex()].mcnk->terrainOffset->textureScale[i];
        }

        /// Load Automatic texture
        for(int i = 0; i < MAX_TEXTURES - 1; ++i)
        {
            automaticTexture[i]      = tile->getHeader().mcin->entries[chunkIndex()].mcnk->terrainOffset->automaticTexture[i];
            automaticTextureStart[i] = tile->getHeader().mcin->entries[chunkIndex()].mcnk->terrainOffset->automaticTextureStart[i];
            automaticTextureEnd[i]   = tile->getHeader().mcin->entries[chunkIndex()].mcnk->terrainOffset->automaticTextureEnd[i];
        }

        /// Load Heightmap
        mapData = tile->getHeader().mcin->entries[chunkIndex()].mcnk->terrainOffset->height;
    }
    else
    {
        qWarning(QObject::tr("Loading default chunk for tile %1_%2 chunk %3_%4, because we couldn't open file!").arg(tile->coordX).arg(tile->coordY).arg(chunkX).arg(chunkY).toLatin1().data());

        /// Load Textures
        if(!world->getTextureManager()->hasTexture("groundTexture", "textures/ground.png"))
            world->getTextureManager()->loadTexture("groundTexture", "textures/ground.png", false);

        if(!world->getTextureManager()->hasTexture("grassTexture", "textures/grass.png"))
            world->getTextureManager()->loadTexture("grassTexture", "textures/grass.png", false);

        if(!world->getTextureManager()->hasTexture("rockTexture", "textures/rock.png"))
            world->getTextureManager()->loadTexture("rockTexture", "textures/rock.png", false);

        if(!world->getTextureManager()->hasTexture("snowTexture", "textures/snowrocks.png"))
            world->getTextureManager()->loadTexture("snowTexture", "textures/snowrocks.png", false);

        textures[0] = world->getTextureManager()->getTexture("groundTexture");
        textures[1] = world->getTextureManager()->getTexture("grassTexture");
        textures[2] = world->getTextureManager()->getTexture("rockTexture");
        textures[3] = world->getTextureManager()->getTexture("snowTexture");

        /// Load Depth Textures
        depthTextures[0] = world->getTextureManager()->getDepthTexture("groundTexture");
        depthTextures[1] = world->getTextureManager()->getDepthTexture("grassTexture");
        depthTextures[2] = world->getTextureManager()->getDepthTexture("rockTexture");
        depthTextures[3] = world->getTextureManager()->getDepthTexture("snowTexture");

        /// Load Alphamaps
        for(int i = 0; i < ALPHAMAPS; ++i)
        {
            alphaMapsData[i] = new unsigned char[CHUNK_ARRAY_UC_SIZE];

            memset(alphaMapsData[i], 0, CHUNK_ARRAY_UC_SIZE);
        }

        /// Load Vertex Shading
        vertexShadingData = new unsigned char[CHUNK_ARRAY_SIZE];

        memset(vertexShadingData, 0, CHUNK_ARRAY_SIZE);

        /// Load Texture scale
        for(int i = 0; i < MAX_TEXTURES; ++i)
        {
            textureScaleFar[i]    = app().getSetting("textureScaleFarSlider", 0.4).toFloat();
            textureScaleNear[i]   = app().getSetting("textureScaleNearSlider", 0.4).toFloat();
            textureScaleOption[i] = (TextureScaleOption)app().getSetting("textureScaleOption", 0).toInt();
        }

        /// Load Automatic texture
        for(int i = 0; i < MAX_TEXTURES - 1; ++i)
        {
            automaticTexture[i]      = false;
            automaticTextureStart[i] = 0.0f;
            automaticTextureEnd[i]   = 0.0f;
        }

        /// Load Heightmap
        mapData = new float[CHUNK_ARRAY_UC_SIZE]; // chunk_array_size

        memset(mapData, 0, sizeof(float) * CHUNK_ARRAY_UC_SIZE);
    }

    /// Set Textures    
    /* !!! TODO, wait for Qt fix then QOpenGLTexture integration with glTexStorage3D !!! */
    /*QOpenGLPixelTransferOptions uploadOptions;
    uploadOptions.setAlignment(1);*/

    world->getGLFunctions()->glActiveTexture(GL_TEXTURE0 + ShaderUnits::Textures);

    textureArray->bind();

    world->getGLFunctions()->glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8, MAX_TEXTURE_SIZE, MAX_TEXTURE_SIZE, MAX_TEXTURES);

    unsigned char* texture_data = (unsigned char*)malloc(MAX_TEXTURE_SIZE * MAX_TEXTURE_SIZE * 4 * MAX_TEXTURES);

    for(int i = 0; i < MAX_TEXTURES; ++i)
    {
        if(textures[i]->isNull())
            continue;

        QImage texture = textures[i]->getImage();//QImage(textures[i]->getPath()).convertToFormat(QImage::Format_RGBA8888);

        if(texture.width() > MAX_TEXTURE_SIZE || texture.height() > MAX_TEXTURE_SIZE)
            qCritical() << QObject::tr("Texture is larger than") << MAX_TEXTURE_SIZE << "px";

        if(texture.width() == 0 || texture.height() == 0)
        {
            qWarning() << QObject::tr("Texture is empty! ID: ") << i;

            continue;
        }

        for(int j = 0; j < MAX_TEXTURE_SIZE / texture.width() * MAX_TEXTURE_SIZE / texture.height(); ++j)
            memcpy(&texture_data[MAX_TEXTURE_SIZE * MAX_TEXTURE_SIZE * 4 * i + texture.width() * texture.height() * 4 * j], &texture.bits()[0], texture.width() * texture.height() * 4);
    }

    world->getGLFunctions()->glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, MAX_TEXTURE_SIZE, MAX_TEXTURE_SIZE, MAX_TEXTURES, GL_RGBA, GL_UNSIGNED_BYTE, texture_data);//textureArray->setData(0, i, QOpenGLTexture::RGBA, QOpenGLTexture::UInt8, textures[i]->getImage().bits(), &uploadOptions);

    chunkMaterial->setTextureUnitConfiguration(ShaderUnits::Textures, textureArray, world->getTextureManager()->getSampler(), "textures");

    free(texture_data); // dealloc

    /// Set Depth Textures
    world->getGLFunctions()->glActiveTexture(GL_TEXTURE0 + ShaderUnits::DepthTextures);

    depthTextureArray->bind();

    world->getGLFunctions()->glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8, MAX_TEXTURE_SIZE, MAX_TEXTURE_SIZE, MAX_TEXTURES);

    unsigned char* depth_data = (unsigned char*)malloc(MAX_TEXTURE_SIZE * MAX_TEXTURE_SIZE * 4 * MAX_TEXTURES);

    for(int i = 0; i < MAX_TEXTURES; ++i)
    {
        if(depthTextures[i]->isNull())
            continue;

        QImage depthTex = depthTextures[i]->getImage();//QImage(depthTextures[i]->getPath()).convertToFormat(QImage::Format_RGBA8888);

        if(depthTex.width() > MAX_TEXTURE_SIZE || depthTex.height() > MAX_TEXTURE_SIZE)
            qCritical() << QObject::tr("Depth texture is larger than") << MAX_TEXTURE_SIZE << "px";

        if(depthTex.width() == 0 || depthTex.height() == 0)
        {
            qWarning() << QObject::tr("Depth texture is empty! ID: ") << i;

            continue;
        }

        for(int j = 0; j < MAX_TEXTURE_SIZE / depthTex.width() * MAX_TEXTURE_SIZE / depthTex.height(); ++j)
            memcpy(&depth_data[MAX_TEXTURE_SIZE * MAX_TEXTURE_SIZE * 4 * i + depthTex.width() * depthTex.height() * 4 * j], &depthTex.bits()[0], depthTex.width() * depthTex.height() * 4);
    }

    world->getGLFunctions()->glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, MAX_TEXTURE_SIZE, MAX_TEXTURE_SIZE, MAX_TEXTURES, GL_RGBA, GL_UNSIGNED_BYTE, depth_data);

    chunkMaterial->setTextureUnitConfiguration(ShaderUnits::DepthTextures, depthTextureArray, world->getTextureManager()->getSampler(), "depthTextures");

    free(depth_data); // dealloc

    /// Set Alphamaps
    for(int i = 0; i < ALPHAMAPS; ++i)
    {
        world->getGLFunctions()->glActiveTexture(GL_TEXTURE0 + ShaderUnits::Alphamap1 + ToGLuint(i));

        TexturePtr alphamap(new Texture(QOpenGLTexture::Target2D));
        alphamap->setSize(TILE_WIDTH / CHUNKS, TILE_HEIGHT / CHUNKS);
        alphamap->setAlphamap(alphaMapsData[i]);

        alphaMaps[i] = alphamap;

        QString alphaMapLayer = QString("layer%1Alpha").arg(i + 1);

        chunkMaterial->setTextureUnitConfiguration(ShaderUnits::Alphamap1 + i, alphaMaps[i], terrainSampler, alphaMapLayer.toLatin1());
    }

    /// Set Vertex Shading
    world->getGLFunctions()->glActiveTexture(GL_TEXTURE0 + ShaderUnits::VertexShading);

    vertexShadingMap = TexturePtr(new Texture(QOpenGLTexture::Target2D));
    vertexShadingMap->setSize(TILE_WIDTH / CHUNKS, TILE_HEIGHT / CHUNKS);
    vertexShadingMap->setVertexShading(vertexShadingData);

    chunkMaterial->setTextureUnitConfiguration(ShaderUnits::VertexShading, vertexShadingMap, terrainSampler, "vertexShading");

    /// Set Terrain
    world->getGLFunctions()->glActiveTexture(GL_TEXTURE0 + ShaderUnits::Heightmap);

    terrainData->setSize(TILE_WIDTH / CHUNKS, TILE_HEIGHT / CHUNKS);
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

    const int xDivisions = trianglesPerHeightSample * TILE_WIDTH  / CHUNKS / maxTessellationLevel;
    const int zDivisions = trianglesPerHeightSample * TILE_HEIGHT / CHUNKS / maxTessellationLevel;

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

    mesh.createVertexArrayObject();
    mesh.createBuffer(Mesh::Vertices, positionData.data(), positionData.size() * sizeof(float));
    mesh.setNumFaces(patchCount);
}

void MapChunk::test()
{
    /*terrainData->bind();

    for(int j = 0; j < 1; ++j)
    {
        for(int i = 0; i < TILE_HEIGHT / CHUNKS; ++i)
            terrainData->updatePixel(1.0f, QVector2D(j, i));
    }

    chunkMaterial->setTextureUnitConfiguration(0, terrainData, terrainSampler, QByteArrayLiteral("heightMap"));*/

    /// Fix NaN
    /*terrainData->bind();

    for(int x = 0; x < TILE_WIDTH / CHUNKS; ++x)
    {
        for(int y = 0; y < TILE_HEIGHT / CHUNKS; ++y)
        {
            if(MathHelper::isNaN(getHeight(x, y)))
            {
                mapData[(TILE_WIDTH / CHUNKS) * y + x] = 0.0f;

                terrainData->setHeight(0.0f, QVector2D(QPoint(x, y)));
            }
        }
    }

    chunkMaterial->setTextureUnitConfiguration(ShaderUnits::Heightmap, terrainData, terrainSampler, QByteArrayLiteral("heightMap"));*/

    /// Fix height
    /*terrainData->bind();

    for(int x = 0; x < TILE_WIDTH / CHUNKS; ++x)
    {
        for(int y = 0; y < TILE_HEIGHT / CHUNKS; ++y)
        {
            if(x < 30 && y < 30)
            {
                if(x % 2 == 0 || y % 2 == 0)
                    continue;

                mapData[(TILE_WIDTH / CHUNKS) * y + x] = x;

                terrainData->setHeight(x, QVector2D(QPoint(x, y)));
            }
        }
    }

    chunkMaterial->setTextureUnitConfiguration(ShaderUnits::Heightmap, terrainData, terrainSampler, QByteArrayLiteral("heightMap"));*/

    terrainData->bind();

    for(int x = 0; x < TILE_WIDTH / CHUNKS; ++x)
    {
        for(int y = 0; y < TILE_HEIGHT / CHUNKS; ++y)
        {
            mapData[(TILE_WIDTH / CHUNKS) * y + x] += 2.0f;

            terrainData->setHeight(mapData[(TILE_WIDTH / CHUNKS) * y + x], QVector2D(QPoint(x, y)));
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

    shader->setUniformValue("textureScaleOption", QVector4D(textureScaleOption[0], textureScaleOption[1], textureScaleOption[2], textureScaleOption[3]));
    shader->setUniformValue("textureScaleFar",    QVector4D(textureScaleFar[0],    textureScaleFar[1],    textureScaleFar[2],    textureScaleFar[3]));
    shader->setUniformValue("textureScaleNear",   QVector4D(textureScaleNear[0],   textureScaleNear[1],   textureScaleNear[2],   textureScaleNear[3]));

    shader->setUniformValue("automaticTexture",      QVector3D(automaticTexture[0], automaticTexture[1], automaticTexture[2]));
    shader->setUniformValue("automaticTextureStart", QVector3D(automaticTextureStart[0], automaticTextureStart[1], automaticTextureStart[2]));
    shader->setUniformValue("automaticTextureEnd",   QVector3D(automaticTextureEnd[0], automaticTextureEnd[1], automaticTextureEnd[2]));

    // Set the fragment shader display mode subroutine
    world->getGLFunctions()->glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &getDisplaySubroutines());

    if(world->displayMode() != world->Hidden)
    {
        // Render the quad as a patch
        {
            mesh.bind();
            mesh.createAttributeArray(Mesh::Vertices, shader, "vertexPosition", GL_FLOAT, 0, 2);

            shader->setPatchVertexCount(1);

            world->getGLFunctions()->glDrawArrays(GL_PATCHES, 0, mesh.getNumFaces());
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
    int X = MathHelper::toInt(x) % TILE_WIDTH;
    int Y = MathHelper::toInt(y) % TILE_HEIGHT;

    int index = (Y * TILE_WIDTH / CHUNKS) + X;

    return mapData[index];
}

const float MapChunk::getHeight(const int& x, const int& y) const
{
    int X = x % TILE_WIDTH;
    int Y = y % TILE_HEIGHT;

    int index = (Y * TILE_WIDTH / CHUNKS) + X;

    return mapData[index];
}

const float MapChunk::getHeightFromWorld(float x, float z)
{
    int X = horizToHMapSize(x) % TILE_WIDTH;
    int Y = horizToHMapSize(z) % TILE_HEIGHT;

    X -= chunkX * (TILE_WIDTH / CHUNKS);
    Y -= chunkY * (TILE_HEIGHT / CHUNKS);

    int index = (Y * TILE_WIDTH / CHUNKS) + X;

    return mapData[index];
}

const float MapChunk::getMapData(const int& index) const
{
    if(index >= (TILE_WIDTH / CHUNKS) * (TILE_HEIGHT / CHUNKS))
    {
        qCritical() << QObject::tr("Trying to getMapData with index out of range:") << index;

        return 0.0f;
    }

    return mapData[index];
}

const float MapChunk::getAlphaFromWorld(float x, float z, int layer)
{
    int X = horizToHMapSize(x) % TILE_WIDTH;
    int Y = horizToHMapSize(z) % TILE_HEIGHT;

    X -= chunkX * (TILE_WIDTH / CHUNKS);
    Y -= chunkY * (TILE_HEIGHT / CHUNKS);

    int index = (Y * TILE_WIDTH / CHUNKS) + X;

    if(layer == 0)
    {
        float alpha = 0.0f;

        for(int i = 0; i < ALPHAMAPS; ++i)
            alpha += MathHelper::toFloat(alphaMapsData[i][index]) / 255.0f;

        return 1.0f - alpha;
    }
    else // layer - 1 cause we already started at 1 but index starts at 0
        return MathHelper::toFloat(alphaMapsData[layer - 1][index]) / 255.0f;;
}

const float MapChunk::getAlphaMapsData(const int& index, const int& layer) const
{
    if(index >= (TILE_WIDTH / CHUNKS) * (TILE_HEIGHT / CHUNKS))
    {
        qCritical() << QObject::tr("Trying to getAlphaMapsData with index out of range:") << index;

        return 0.0f;
    }

    if(layer == 0)
    {
        float alpha = 0.0f;

        for(int i = 0; i < ALPHAMAPS; ++i)
            alpha += MathHelper::toFloat(alphaMapsData[i][index]) / 255.0f;

        return 1.0f - alpha;
    }
    else // layer - 1 cause we already started at 1 but index starts at 0
        return MathHelper::toFloat(alphaMapsData[layer - 1][index] / 255.0f);
}

const QColor MapChunk::getVertexShadingData(const int& index) const
{
    if(index + 3 >= (TILE_WIDTH / CHUNKS) * (TILE_HEIGHT / CHUNKS) * 4) // 4 channels
    {
        qCritical() << QObject::tr("Trying to getVertexShadingData with index out of range:") << index;

        return QColor();
    }

    return QColor(MathHelper::toInt(vertexShadingData[index]), MathHelper::toInt(vertexShadingData[index + 1]), MathHelper::toInt(vertexShadingData[index + 2]), MathHelper::toInt(vertexShadingData[index + 3]));
}

const QColor MapChunk::getVertexLightingData(const int& index) const
{
    if(index + 3 >= (TILE_WIDTH / CHUNKS) * (TILE_HEIGHT / CHUNKS) * 4) // 4 channels
    {
        qCritical() << QObject::tr("Trying to getVertexLightingData with index out of range:") << index;

        return QColor();
    }

    return QColor(MathHelper::toInt(vertexLightingData[index]), MathHelper::toInt(vertexLightingData[index + 1]), MathHelper::toInt(vertexLightingData[index + 2]), MathHelper::toInt(vertexLightingData[index + 3]));
}

const int MapChunk::chunkIndex() const
{
    return (chunkY * CHUNKS) + chunkX;
}

const int MapChunk::getTextureScaleOption(int layer) const
{
    return (int)textureScaleOption[layer];
}

const float MapChunk::getTextureScaleFar(int layer) const
{
    return textureScaleFar[layer];
}

const float MapChunk::getTextureScaleNear(int layer) const
{
    return textureScaleNear[layer];
}

const bool MapChunk::getAutomaticTexture(int layer) const
{
    // dynamic warning
    bool warn = true;

    for(int i = 1; i < MAX_TEXTURES; ++i)
    {
        if(i == layer)
        {
            warn = false;

            break;
        }
    }

    if(warn)
    {
        qWarning() << QObject::tr("Layer is out of range! Returning false for safe working, report this to developer!");

        return false;
    }

    --layer; // index is moved from 1-3 to 0-2

    return automaticTexture[layer];
}

const float MapChunk::getAutomaticTextureStart(int layer) const
{
    // dynamic warning
    bool warn = true;

    for(int i = 1; i < MAX_TEXTURES; ++i)
    {
        if(i == layer)
        {
            warn = false;

            break;
        }
    }

    if(warn)
    {
        qWarning() << QObject::tr("Layer is out of range! Returning 0.0f for safe working, report this to developer!");

        return 0.0f;
    }

    --layer; // index is moved from 1-3 to 0-2

    return automaticTextureStart[layer];
}

const float MapChunk::getAutomaticTextureEnd(int layer) const
{
    // dynamic warning
    bool warn = true;

    for(int i = 1; i < MAX_TEXTURES; ++i)
    {
        if(i == layer)
        {
            warn = false;

            break;
        }
    }

    if(warn)
    {
        qWarning() << QObject::tr("Layer is out of range! Returning 0.0f for safe working, report this to developer!");

        return 0.0f;
    }

    --layer; // index is moved from 1-3 to 0-2

    return automaticTextureEnd[layer];
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
    QVector<TerrainUndoData>       undo;

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
                            int X = _x % TILE_WIDTH;
                            int Y = _y % TILE_HEIGHT;

                            X -= chunkX * (TILE_WIDTH / CHUNKS);
                            Y -= chunkY * (TILE_HEIGHT / CHUNKS);

                            int index = (Y * (TILE_WIDTH / CHUNKS)) + X;

                            if(index < 0 || index >= CHUNK_ARRAY_UC_SIZE || X < 0 || Y < 0 || X >= (TILE_WIDTH / CHUNKS) || Y >= (TILE_HEIGHT / CHUNKS))
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
                                //mapData[index] = world->getTileAt(x, z)->getChunk(chunkX - 1, chunkY)->getHeight(TILE_WIDTH / CHUNKS - 1, Y) + 0.001f;

                            changing.append(qMakePair<float, QVector2D>(mapData[index], QVector2D(X, Y)));
                            undo.append(TerrainUndoData(index, mapData[index] - changeFormula, QPoint(X, Y)));

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

            QVector2D position = QVector2D(chunkBaseX + changing.value(i).second.x() / MathHelper::toFloat(CHUNK_WIDTH) * CHUNKSIZE, chunkBaseY + changing.value(i).second.y() / MathHelper::toFloat(CHUNK_HEIGHT) * CHUNKSIZE);

            world->addModifiedTerrain(QPair<QVector2D, TerrainUndoData>(position, undo.at(i)));

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
    QVector<TerrainUndoData>       undo;

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
                            int X = _x % TILE_WIDTH;
                            int Y = _y % TILE_HEIGHT;

                            X -= chunkX * (TILE_WIDTH / CHUNKS);
                            Y -= chunkY * (TILE_HEIGHT / CHUNKS);

                            int index = (Y * (TILE_WIDTH / CHUNKS)) + X;

                            if(index < 0 || index >= CHUNK_ARRAY_UC_SIZE || X < 0 || Y < 0 || X >= (TILE_WIDTH / CHUNKS) || Y >= (TILE_HEIGHT / CHUNKS))
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

                            float preVal = mapData[index];

                            mapData[index] = changeFormula;

                            changing.append(qMakePair<float, QVector2D>(mapData[index], QVector2D(X, Y)));
                            undo.append(TerrainUndoData(index, preVal, QPoint(X, Y)));

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

            QVector2D position = QVector2D(chunkBaseX + changing.value(i).second.x() / MathHelper::toFloat(CHUNK_WIDTH) * CHUNKSIZE, chunkBaseY + changing.value(i).second.y() / MathHelper::toFloat(CHUNK_HEIGHT) * CHUNKSIZE);

            world->addModifiedTerrain(QPair<QVector2D, TerrainUndoData>(position, undo.at(i)));

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
    QVector<TerrainUndoData>       undo;

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
                            int X = _x % TILE_WIDTH;
                            int Y = _y % TILE_HEIGHT;

                            X -= chunkX * (TILE_WIDTH / CHUNKS);
                            Y -= chunkY * (TILE_HEIGHT / CHUNKS);

                            int index = (Y * (TILE_WIDTH / CHUNKS)) + X;

                            if(index < 0 || index >= CHUNK_ARRAY_UC_SIZE || X < 0 || Y < 0 || X >= (TILE_WIDTH / CHUNKS) || Y >= (TILE_HEIGHT / CHUNKS))
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

                                    int X2 = MathHelper::toInt(tx) % TILE_WIDTH;
                                    int Y2 = MathHelper::toInt(tz) % TILE_HEIGHT;

                                    X2 -= chunkX * (TILE_WIDTH / CHUNKS);
                                    Y2 -= chunkY * (TILE_HEIGHT / CHUNKS);

                                    int index2 = (Y2 * (TILE_WIDTH / CHUNKS)) + X2;

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

                            float preVal = mapData[index];

                            mapData[index] = changeFormula;

                            changing.append(qMakePair<float, QVector2D>(mapData[index], QVector2D(X, Y)));
                            undo.append(TerrainUndoData(index, preVal, QPoint(X, Y)));

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

            QVector2D position = QVector2D(chunkBaseX + changing.value(i).second.x() / MathHelper::toFloat(CHUNK_WIDTH) * CHUNKSIZE, chunkBaseY + changing.value(i).second.y() / MathHelper::toFloat(CHUNK_HEIGHT) * CHUNKSIZE);

            world->addModifiedTerrain(QPair<QVector2D, TerrainUndoData>(position, undo.at(i)));

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

bool MapChunk::uniformTerrain(float x, float z, float height)
{
    bool changed = false;

    const Brush* brush = world->getBrush();

    float xdiff = baseX - x + CHUNKSIZE / 2;
    float ydiff = baseY - z + CHUNKSIZE / 2;

    float dist = sqrt(pow(xdiff, 2) + pow(ydiff, 2));

    if(dist > (brush->OuterRadius() + CHUNK_DIAMETER) || MathHelper::isNaN(height))
        return changed;

    QList<QPair<float, QVector2D>> changing;
    QVector<TerrainUndoData>       undo;

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
                            int X = _x % TILE_WIDTH;
                            int Y = _y % TILE_HEIGHT;

                            X -= chunkX * (TILE_WIDTH / CHUNKS);
                            Y -= chunkY * (TILE_HEIGHT / CHUNKS);

                            int index = (Y * (TILE_WIDTH / CHUNKS)) + X;

                            if(index < 0 || index >= CHUNK_ARRAY_UC_SIZE || X < 0 || Y < 0 || X >= (TILE_WIDTH / CHUNKS) || Y >= (TILE_HEIGHT / CHUNKS))
                                continue;

                            if(mapData[index] == height)
                                continue;

                            float changeFormula = mapData[index];

                            mapData[index] = height;

                            changing.append(qMakePair<float, QVector2D>(mapData[index], QVector2D(X, Y)));
                            undo.append(TerrainUndoData(index, changeFormula, QPoint(X, Y)));

                            changed = true;
                        }
                    }
                    break;
            }
        }
    }

    if(changed) // without neighbours
    {
        terrainData->bind();

        for(int i = 0; i < changing.count(); ++i)
        {
            QVector2D position = QVector2D(chunkBaseX + changing.value(i).second.x() / MathHelper::toFloat(CHUNK_WIDTH) * CHUNKSIZE, chunkBaseY + changing.value(i).second.y() / MathHelper::toFloat(CHUNK_HEIGHT) * CHUNKSIZE);

            world->addModifiedTerrain(QPair<QVector2D, TerrainUndoData>(position, undo.at(i)));

            terrainData->setHeight(changing.value(i).first, changing.value(i).second);
        }

        chunkMaterial->setTextureUnitConfiguration(ShaderUnits::Heightmap, terrainData, terrainSampler, QByteArrayLiteral("heightMap"));
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

                /// Set texture
                unsigned char* texture_data = (unsigned char*)malloc(MAX_TEXTURE_SIZE * MAX_TEXTURE_SIZE * 4);

                memset(texture_data, 0, MAX_TEXTURE_SIZE * MAX_TEXTURE_SIZE * 4);

                if(!textures[i]->isNull())
                {
                    QImage _texture = QImage(textures[i]->getPath()).convertToFormat(QImage::Format_RGBA8888);

                    if(_texture.width() > MAX_TEXTURE_SIZE || _texture.height() > MAX_TEXTURE_SIZE)
                        qCritical() << QObject::tr("Texture is larger than") << MAX_TEXTURE_SIZE << "px";

                    if(_texture.width() != 0 && _texture.height() != 0)
                    {
                        for(int i = 0; i < MAX_TEXTURE_SIZE / _texture.width() * MAX_TEXTURE_SIZE / _texture.height(); ++i)
                            memcpy(&texture_data[_texture.width() * _texture.height() * 4 * i], &_texture.bits()[0], _texture.width() * _texture.height() * 4);
                    }
                    else
                        qWarning() << QObject::tr("Texture is empty!");
                }

                textureArray->bind();

                world->getGLFunctions()->glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, MAX_TEXTURE_SIZE, MAX_TEXTURE_SIZE, 1, GL_RGBA, GL_UNSIGNED_BYTE, texture_data);

                free(texture_data); // dealloc

                chunkMaterial->setTextureUnitConfiguration(ShaderUnits::Textures, textureArray, world->getTextureManager()->getSampler(), "textures");

                /// Depth texture
                TexturePtr depthTexture = TexturePtr(new Texture());

                unsigned char* depth_data = (unsigned char*)malloc(MAX_TEXTURE_SIZE * MAX_TEXTURE_SIZE * 4);

                if(world->getTextureManager()->hasDepthTexture(texture))
                    depthTexture = world->getTextureManager()->getDepthTexture(QFileInfo(texture->getPath()).baseName() + "Texture");

                if(!depthTexture->isNull())
                {
                    QImage _depthTexture = QImage(depthTexture->getPath()).convertToFormat(QImage::Format_RGBA8888);

                    if(_depthTexture.width() > MAX_TEXTURE_SIZE || _depthTexture.height() > MAX_TEXTURE_SIZE)
                        qCritical() << QObject::tr("Depth Texture is larger than") << MAX_TEXTURE_SIZE << "px";

                    if(_depthTexture.width() != 0 && _depthTexture.height() != 0)
                    {
                        for(int i = 0; i < MAX_TEXTURE_SIZE / _depthTexture.width() * MAX_TEXTURE_SIZE / _depthTexture.height(); ++i)
                            memcpy(&depth_data[_depthTexture.width() * _depthTexture.height() * 4 * i], &_depthTexture.bits()[0], _depthTexture.width() * _depthTexture.height() * 4);
                    }
                    else
                        qWarning() << QObject::tr("Depth Texture is empty!");
                }

                depthTextureArray->bind();

                world->getGLFunctions()->glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, MAX_TEXTURE_SIZE, MAX_TEXTURE_SIZE, 1, GL_RGBA, GL_UNSIGNED_BYTE, depth_data);

                free(depth_data); // dealloc

                chunkMaterial->setTextureUnitConfiguration(ShaderUnits::Textures, textureArray, world->getTextureManager()->getSampler(), "depthTextures");

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

    QVector<TextureUndoData> undo;

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
                            int X = _x % TILE_WIDTH;
                            int Y = _y % TILE_HEIGHT;

                            X -= chunkX * (TILE_WIDTH / CHUNKS);
                            Y -= chunkY * (TILE_HEIGHT / CHUNKS);

                            int index = (Y * (TILE_WIDTH / CHUNKS)) + X;

                            if(index < 0 || index >= CHUNK_ARRAY_UC_SIZE || X < 0 || Y < 0 || X >= (TILE_WIDTH / CHUNKS) || Y >= (TILE_HEIGHT / CHUNKS))
                                continue;

                            if(textureIndex > 0 && world->getPaintMaximumState() && ToFloat(alphaMapsData[textureIndex - 1][index]) >= world->getPaintMaximumAlpha() * 255.0f)
                                continue;

                            float lossyMultiplier = brush->calcLossyMultiplier(dist);

                            switch(brush->BrushTypes().texturing)
                            {
                                case Brush::TexturingType::Solid:
                                default:
                                    {
                                        QVector<float> values;

                                        for(int i = 0; i < ALPHAMAPS; ++i)
                                            values.append(MathHelper::toFloat(alphaMapsData[i][index]) / 255.0f);

                                        undo.append(TextureUndoData(index, values, QPoint(X, Y)));

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

        for(int i = 0; i < undo.count(); ++i)
        {
            QVector2D position = QVector2D(chunkBaseX + undo.at(i).position.x() / MathHelper::toFloat(CHUNK_WIDTH) * CHUNKSIZE, chunkBaseY + undo.at(i).position.y() / MathHelper::toFloat(CHUNK_HEIGHT) * CHUNKSIZE);

            world->addModifiedTextures(QPair<QVector2D, TextureUndoData>(position, undo.at(i)));
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
    QVector<VertexUndoData>                         undo;

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
                            int X = _x % TILE_WIDTH;
                            int Y = _y % TILE_HEIGHT;

                            X -= chunkX * (TILE_WIDTH / CHUNKS);
                            Y -= chunkY * (TILE_HEIGHT / CHUNKS);

                            int index = (Y * (TILE_WIDTH / CHUNKS)) + X;

                            if(index < 0 || index >= CHUNK_ARRAY_SIZE || X < 0 || Y < 0 || X >= (TILE_WIDTH / CHUNKS) || Y >= (TILE_HEIGHT / CHUNKS))
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
                                        undo.append(VertexUndoData(index, dataContainer.at(0), dataContainer.at(1), dataContainer.at(2), dataContainer.at(3), QPoint(X, Y)));
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

            QVector2D position = QVector2D(chunkBaseX + changing.at(i).second.x() / MathHelper::toFloat(CHUNK_WIDTH) * CHUNKSIZE, chunkBaseY + changing.at(i).second.y() / MathHelper::toFloat(CHUNK_HEIGHT) * CHUNKSIZE);

            world->addModifiedVertex(QPair<QVector2D, VertexUndoData>(position, undo.at(i)));
        }

        chunkMaterial->setTextureUnitConfiguration(ShaderUnits::VertexShading, vertexShadingMap, terrainSampler, "vertexShading");
    }

    return changed;
}

bool MapChunk::paintVertexLighting(float x, float z, float flow, QColor& lightColor)
{
    bool changed = false;

    const Brush* brush = world->getBrush();

    float xdiff = baseX - x + CHUNKSIZE / 2;
    float ydiff = baseY - z + CHUNKSIZE / 2;

    float dist = sqrt(pow(xdiff, 2) + pow(ydiff, 2));

    if(dist > (brush->OuterRadius() + CHUNK_DIAMETER))
        return changed;

    QList<QPair<QVector<unsigned char>, QVector2D>> changing;
    QVector<VertexUndoData>                         undo;

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
                            int X = _x % TILE_WIDTH;
                            int Y = _y % TILE_HEIGHT;

                            X -= chunkX * (TILE_WIDTH / CHUNKS);
                            Y -= chunkY * (TILE_HEIGHT / CHUNKS);

                            int index = (Y * (TILE_WIDTH / CHUNKS)) + X;

                            if(index < 0 || index >= CHUNK_ARRAY_SIZE || X < 0 || Y < 0 || X >= (TILE_WIDTH / CHUNKS) || Y >= (TILE_HEIGHT / CHUNKS))
                                continue;

                            index *= sizeof(float);

                            float lossyMultiplier = brush->calcLossyMultiplier(dist);

                            switch(brush->BrushTypes().vertexShading)
                            {
                                case Brush::TexturingType::Solid:
                                default:
                                    {
                                        vertexLightingData[index]     = ToUChar(qMax(qMin(MathHelper::closerTo(vertexLightingData[index],     lightColor.redF(),   ((255.0f - vertexLightingData[index])     * flow * 3) * lossyMultiplier), 255.0f), 0.0f));
                                        vertexLightingData[index + 1] = ToUChar(qMax(qMin(MathHelper::closerTo(vertexLightingData[index + 1], lightColor.greenF(), ((255.0f - vertexLightingData[index + 1]) * flow * 3) * lossyMultiplier), 255.0f), 0.0f));
                                        vertexLightingData[index + 2] = ToUChar(qMax(qMin(MathHelper::closerTo(vertexLightingData[index + 2], lightColor.blueF(),  ((255.0f - vertexLightingData[index + 2]) * flow * 3) * lossyMultiplier), 255.0f), 0.0f));

                                        if(!world->getPaintMaximumState() || (world->getPaintMaximumState() && ToFloat(vertexLightingData[index + 3]) < world->getPaintMaximumAlpha() * 255.0f))
                                            vertexLightingData[index + 3] = ToUChar(qMax(qMin(MathHelper::closerTo(vertexLightingData[index + 3], lightColor.alphaF(), ((255.0f - vertexLightingData[index + 3]) * flow * 3) * lossyMultiplier), 255.0f), 0.0f));

                                        QVector<unsigned char> dataContainer;
                                        dataContainer.append(vertexLightingData[index]);     // R
                                        dataContainer.append(vertexLightingData[index + 1]); // G
                                        dataContainer.append(vertexLightingData[index + 2]); // B
                                        dataContainer.append(vertexLightingData[index + 3]); // A

                                        changing.append(qMakePair<QVector<unsigned char>, QVector2D>(dataContainer, QVector2D(X, Y)));
                                        undo.append(VertexUndoData(index, dataContainer.at(0), dataContainer.at(1), dataContainer.at(2), dataContainer.at(3), QPoint(X, Y)));
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
        vertexLightingMap->bind();

        for(int i = 0; i < changing.count(); ++i)
        {
            unsigned char data[4];

            for(int j = 0; j < 4; ++j)
                data[j] = changing.value(i).first.at(j);

            vertexLightingMap->setVertexShade(data, changing.value(i).second);

            QVector2D position = QVector2D(chunkBaseX + changing.at(i).second.x() / MathHelper::toFloat(CHUNK_WIDTH) * CHUNKSIZE, chunkBaseY + changing.at(i).second.y() / MathHelper::toFloat(CHUNK_HEIGHT) * CHUNKSIZE);

            world->addModifiedVertex(QPair<QVector2D, VertexUndoData>(position, undo.at(i)));
        }

        chunkMaterial->setTextureUnitConfiguration(ShaderUnits::VertexLighting, vertexLightingMap, terrainSampler, "vertexLighting");
    }

    return changed;
}

QVector<QPair<QString, QVector3D>> MapChunk::spawnDetailDoodads(QVector<QString>& texturesList, QMap<int, QVector<QPair<QString, float>>>& data)
{
    // remove stuff which is not for this chunk
    QVector<QString> texturesList2;
    QVector<int>     texturesListAlphaIndex;
    QVector<QVector<QPair<float, float>>> texturesListAlpha;

    QVector<QVector<QPair<QString, float>>> data2;
    QVector<DetailDoodadsDataContainer> data3;

    for(int i = 0; i < texturesList.count(); ++i)
    {
        bool exists = false;

        int layer = -1;

        for(int j = 0; j < MAX_TEXTURES; ++j)
        {
            if(texturesList[i] == textures[j]->getPath())
            {
                exists = true;
                layer  = j;

                break;
            }
        }

        if(exists)
        {
            texturesList2.append(texturesList.at(i));
            texturesListAlphaIndex.append(layer);
            data2.append(data[i]);
        }

        if(texturesList2.count() >= MAX_TEXTURES)
            break;
    }

    texturesListAlpha.resize(texturesList2.count());

    for(int i = 0; i < texturesList2.count(); ++i)
    {
        for(int j = 0; j < data2[i].count(); ++j)
        {
            DetailDoodadsDataContainer container;
            container.objectPath  = data2[i][j].first;
            container.texturePath = texturesList2[i];
            container.radius      = data2[i][j].second;

            data3.append(container);
        }
    }

    // alphamaps available points
    int counter = 0;

    for(int i = 0; i < texturesList2.count(); ++i)
    {
        int layer = texturesListAlphaIndex[i];

        for(int y = 0; y < CHUNK_HEIGHT; ++y)
        {
            for(int x = 0; x < CHUNK_WIDTH; ++x)
            {
                float _x = chunkBaseX + CHUNKSIZE / MathHelper::toFloat(CHUNK_WIDTH) * x;
                float _z = chunkBaseY + CHUNKSIZE / MathHelper::toFloat(CHUNK_HEIGHT) * y;

                if(getAlphaFromWorld(_x, _z, layer) >= 0.5f)
                {
                    texturesListAlpha[i].append(QPair<float, float>(_x, _z));

                    ++counter;
                }

                if(counter >= CHUNK_WIDTH * CHUNK_HEIGHT)
                    break;
            }

            if(counter >= CHUNK_WIDTH * CHUNK_HEIGHT)
                break;
        }

        if(counter >= CHUNK_WIDTH * CHUNK_HEIGHT)
            break;
    }

    // spawn vector
    int objectsPerChunk = MAX_OBJECTS * 4 / (CHUNKS * CHUNKS);
    int objectIndex     = 0;

    QVector<QPair<QString, QVector3D>> resultData;

    MapTile* tile = world->getTileAt(chunkBaseX, chunkBaseY);

    for(int i = 0; i < objectsPerChunk; ++i)
    {
        if(objectIndex >= data3.count())
            objectIndex = 0;

        // get correct texture
        bool can = false;

        int layer = -1;

        for(int j = 0; j < MAX_TEXTURES; ++j)
        {
            if(data3[objectIndex].texturePath == textures[j]->getPath())
            {
                can   = true;
                layer = j;

                break;
            }
        }

        if(!can)
        {
            --i;

            continue;
        }

        if(texturesListAlpha[layer].count() < 1) // just go on and count objectPerChunk
        {
            ++objectIndex;

            continue;
        }

        int index = MathHelper::random(0, texturesListAlpha[layer].count());

        QPair<float, float> value = texturesListAlpha[layer][index];

        float x = value.first;
        float y = value.second;

        for(int j = 0; j < resultData.count(); ++j)
        {
            QVector3D resultDataVector = resultData[j].second;

            if(MathHelper::isInRadius(x, y, resultDataVector.x(), resultDataVector.z(), data3[objectIndex].radius))
            {
                can = false;

                break;
            }
        }

        if(!can)
        {
            texturesListAlpha[layer].removeAt(index);

            --i;

            continue;
        }

        for(int j = 0; j < tile->getMapObjects().count(); ++j)
        {
            QVector3D mapObjectData = tile->getMapObjects().at(j)->getTranslate();

            if(MathHelper::isInRadius(x, y, mapObjectData.x(), mapObjectData.z(), data3[objectIndex].radius))
            {
                can = false;

                break;
            }
        }

        if(!can)
        {
            texturesListAlpha[layer].removeAt(index);

            --i;

            continue;
        }

        texturesListAlpha[layer].removeAt(index);

        float z = getHeightFromWorld(x, y);

        resultData.append(QPair<QString, QVector3D>(data3[objectIndex].objectPath, QVector3D(x, z, y)));

        ++objectIndex;
    }

    return resultData; // => spawn
}

void MapChunk::updateNeighboursHeightmapData()
{
    if(leftNeighbour)
    {
        for(int y = 0; y < TILE_HEIGHT / CHUNKS; ++y)
            mapData[y * TILE_WIDTH / CHUNKS] = leftNeighbour->getHeight(TILE_WIDTH / CHUNKS - 1, y);
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
    return MathHelper::toInt(position / TILESIZE * MathHelper::toFloat(TILE_WIDTH));
}

float MapChunk::HMapSizeToHoriz(int position)
{
    return MathHelper::toFloat(position) / MathHelper::toFloat(TILE_WIDTH) * TILESIZE;
}

void MapChunk::moveAlphaMap(int index, bool up)
{
    /// TODO: use calloc instead of malloc + memset? !!! Dont set memory for depth texture! Never...
    textureArray->bind();

    switch(up)
    {
        case true:
            {
                TexturePtr temp      = textures[index - 1];
                TexturePtr tempDepth = depthTextures[index - 1];

                qDebug() << textures[index - 1]->getPath() << "going bot" << chunkIndex();
                qDebug() << textures[index]->getPath() << "going top";

                textures[index - 1] = textures[index];
                textures[index]     = temp;

                depthTextures[index - 1] = depthTextures[index];
                depthTextures[index]     = tempDepth;

                // texture
                unsigned char* texture_data = (unsigned char*)malloc(MAX_TEXTURE_SIZE * MAX_TEXTURE_SIZE * 4);

                memset(texture_data, 0, MAX_TEXTURE_SIZE * MAX_TEXTURE_SIZE * 4);

                if(!textures[index - 1]->isNull())
                {
                    QImage texture = QImage(textures[index - 1]->getPath()).convertToFormat(QImage::Format_RGBA8888);

                    if(texture.width() > MAX_TEXTURE_SIZE || texture.height() > MAX_TEXTURE_SIZE)
                        qCritical() << QObject::tr("Texture is larger than") << MAX_TEXTURE_SIZE << "px";

                    if(texture.width() != 0 && texture.height() != 0)
                    {
                        for(int i = 0; i < MAX_TEXTURE_SIZE / texture.width() * MAX_TEXTURE_SIZE / texture.height(); ++i)
                            memcpy(&texture_data[texture.width() * texture.height() * 4 * i], &texture.bits()[0], texture.width() * texture.height() * 4);
                    }
                    else
                        qWarning() << QObject::tr("Texture is empty!");
                }

                textureArray->bind();

                world->getGLFunctions()->glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, index - 1, MAX_TEXTURE_SIZE, MAX_TEXTURE_SIZE, 1, GL_RGBA, GL_UNSIGNED_BYTE, texture_data);

                free(texture_data); // dealloc

                // depth
                unsigned char* depth_data = (unsigned char*)malloc(MAX_TEXTURE_SIZE * MAX_TEXTURE_SIZE * 4);

                if(!depthTextures[index - 1]->isNull())
                {
                    QImage depthTexture = QImage(depthTextures[index - 1]->getPath()).convertToFormat(QImage::Format_RGBA8888);

                    if(depthTexture.width() > MAX_TEXTURE_SIZE || depthTexture.height() > MAX_TEXTURE_SIZE)
                        qCritical() << QObject::tr("Depth Texture is larger than") << MAX_TEXTURE_SIZE << "px";

                    if(depthTexture.width() != 0 && depthTexture.height() != 0)
                    {
                        for(int i = 0; i < MAX_TEXTURE_SIZE / depthTexture.width() * MAX_TEXTURE_SIZE / depthTexture.height(); ++i)
                            memcpy(&depth_data[depthTexture.width() * depthTexture.height() * 4 * i], &depthTexture.bits()[0], depthTexture.width() * depthTexture.height() * 4);
                    }
                    else
                        qWarning() << QObject::tr("Depth Texture is empty!");
                }

                depthTextureArray->bind();

                world->getGLFunctions()->glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, index - 1, MAX_TEXTURE_SIZE, MAX_TEXTURE_SIZE, 1, GL_RGBA, GL_UNSIGNED_BYTE, depth_data);

                free(depth_data); // dealloc
            }
            break;

        case false:
            {
                TexturePtr temp      = textures[index + 1];
                TexturePtr tempDepth = depthTextures[index + 1];

                textures[index + 1] = textures[index];
                textures[index]     = temp;

                depthTextures[index + 1] = depthTextures[index];
                depthTextures[index]     = tempDepth;

                // texture
                unsigned char* texture_data = (unsigned char*)malloc(MAX_TEXTURE_SIZE * MAX_TEXTURE_SIZE * 4);

                memset(texture_data, 0, MAX_TEXTURE_SIZE * MAX_TEXTURE_SIZE * 4);

                if(!textures[index + 1]->isNull())
                {
                    QImage texture = QImage(textures[index + 1]->getPath()).convertToFormat(QImage::Format_RGBA8888);

                    if(texture.width() > MAX_TEXTURE_SIZE || texture.height() > MAX_TEXTURE_SIZE)
                        qCritical() << QObject::tr("Texture is larger than") << MAX_TEXTURE_SIZE << "px";

                    if(texture.width() != 0 && texture.height() != 0)
                    {
                        for(int i = 0; i < MAX_TEXTURE_SIZE / texture.width() * MAX_TEXTURE_SIZE / texture.height(); ++i)
                            memcpy(&texture_data[texture.width() * texture.height() * 4 * i], &texture.bits()[0], texture.width() * texture.height() * 4);
                    }
                    else
                        qWarning() << QObject::tr("Texture is empty!");
                }

                textureArray->bind();

                world->getGLFunctions()->glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, index + 1, MAX_TEXTURE_SIZE, MAX_TEXTURE_SIZE, 1, GL_RGBA, GL_UNSIGNED_BYTE, texture_data);

                free(texture_data); // dealloc

                // depth
                unsigned char* depth_data = (unsigned char*)malloc(MAX_TEXTURE_SIZE * MAX_TEXTURE_SIZE * 4);

                if(!depthTextures[index + 1]->isNull())
                {
                    QImage depthTexture = QImage(depthTextures[index + 1]->getPath()).convertToFormat(QImage::Format_RGBA8888);

                    if(depthTexture.width() > MAX_TEXTURE_SIZE || depthTexture.height() > MAX_TEXTURE_SIZE)
                        qCritical() << QObject::tr("Depth Texture is larger than") << MAX_TEXTURE_SIZE << "px";

                    if(depthTexture.width() != 0 && depthTexture.height() != 0)
                    {
                        for(int i = 0; i < MAX_TEXTURE_SIZE / depthTexture.width() * MAX_TEXTURE_SIZE / depthTexture.height(); ++i)
                            memcpy(&depth_data[depthTexture.width() * depthTexture.height() * 4 * i], &depthTexture.bits()[0], depthTexture.width() * depthTexture.height() * 4);
                    }
                    else
                        qWarning() << QObject::tr("Depth Texture is empty!");
                }

                depthTextureArray->bind();

                world->getGLFunctions()->glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, index + 1, MAX_TEXTURE_SIZE, MAX_TEXTURE_SIZE, 1, GL_RGBA, GL_UNSIGNED_BYTE, depth_data);

                free(depth_data); // dealloc
            }
            break;
    }

    // texture
    unsigned char* texture_temp_data = (unsigned char*)malloc(MAX_TEXTURE_SIZE * MAX_TEXTURE_SIZE * 4);

    memset(texture_temp_data, 0, MAX_TEXTURE_SIZE * MAX_TEXTURE_SIZE * 4);

    if(!textures[index]->isNull())
    {
        QImage texture = QImage(textures[index]->getPath()).convertToFormat(QImage::Format_RGBA8888);

        if(texture.width() > MAX_TEXTURE_SIZE || texture.height() > MAX_TEXTURE_SIZE)
            qCritical() << QObject::tr("Texture is larger than") << MAX_TEXTURE_SIZE << "px";

        if(texture.width() != 0 && texture.height() != 0)
        {
            for(int i = 0; i < MAX_TEXTURE_SIZE / texture.width() * MAX_TEXTURE_SIZE / texture.height(); ++i)
                memcpy(&texture_temp_data[texture.width() * texture.height() * 4 * i], &texture.bits()[0], texture.width() * texture.height() * 4);
        }
        else
            qWarning() << QObject::tr("Texture is empty!");
    }

    textureArray->bind();

    world->getGLFunctions()->glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, index, MAX_TEXTURE_SIZE, MAX_TEXTURE_SIZE, 1, GL_RGBA, GL_UNSIGNED_BYTE, texture_temp_data);

    free(texture_temp_data); // dealloc

    // depth
    unsigned char* depth_temp_data = (unsigned char*)malloc(MAX_TEXTURE_SIZE * MAX_TEXTURE_SIZE * 4);

    if(!depthTextures[index]->isNull())
    {
        QImage depthTexture = QImage(depthTextures[index]->getPath()).convertToFormat(QImage::Format_RGBA8888);

        if(depthTexture.width() > MAX_TEXTURE_SIZE || depthTexture.height() > MAX_TEXTURE_SIZE)
            qCritical() << QObject::tr("Depth Texture is larger than") << MAX_TEXTURE_SIZE << "px";

        if(depthTexture.width() != 0 && depthTexture.height() != 0)
        {
            for(int i = 0; i < MAX_TEXTURE_SIZE / depthTexture.width() * MAX_TEXTURE_SIZE / depthTexture.height(); ++i)
                memcpy(&depth_temp_data[depthTexture.width() * depthTexture.height() * 4 * i], &depthTexture.bits()[0], depthTexture.width() * depthTexture.height() * 4);
        }
        else
            qWarning() << QObject::tr("Depth Texture is empty!");
    }

    depthTextureArray->bind();

    world->getGLFunctions()->glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, index, MAX_TEXTURE_SIZE, MAX_TEXTURE_SIZE, 1, GL_RGBA, GL_UNSIGNED_BYTE, depth_temp_data);

    free(depth_temp_data); // dealloc

    chunkMaterial->setTextureUnitConfiguration(ShaderUnits::Textures, textureArray, world->getTextureManager()->getSampler(), "textures");
    chunkMaterial->setTextureUnitConfiguration(ShaderUnits::Textures, textureArray, world->getTextureManager()->getSampler(), "depthTextures");
}

void MapChunk::deleteAlphaMap(int index)
{
    textures[index] = TexturePtr(new Texture());

    unsigned char* texture_data = (unsigned char*)malloc(MAX_TEXTURE_SIZE * MAX_TEXTURE_SIZE * 4);

    memset(texture_data, 0, MAX_TEXTURE_SIZE * MAX_TEXTURE_SIZE * 4);

    textureArray->bind();

    world->getGLFunctions()->glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, index + 1, MAX_TEXTURE_SIZE, MAX_TEXTURE_SIZE, 1, GL_RGBA, GL_UNSIGNED_BYTE, texture_data);

    free(texture_data); // dealloc

    chunkMaterial->setTextureUnitConfiguration(ShaderUnits::Textures, textureArray, world->getTextureManager()->getSampler(), "textures");

    memset(alphaMapsData[index], 0, TILE_WIDTH / CHUNKS * TILE_HEIGHT / CHUNKS); // clear alpha map

    alphaMaps[index]->setAlphamap(alphaMapsData[index]);

    QString uniformName = QString("layer%1Alpha").arg(index);

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
                    terrainData->setHeight(data.at(i).second, QVector2D(data.at(i).first, TILE_HEIGHT / CHUNKS - 1));

                    int index = (((TILE_HEIGHT / CHUNKS - 1) * (TILE_WIDTH / CHUNKS)) + data.at(i).first);

                    mapData[index] = data.at(i).second;
                }
            }
            break;

        case Vertical:
            {
                for(int i = 0; i < data.count(); ++i)
                {
                    terrainData->setHeight(data.at(i).second, QVector2D(TILE_HEIGHT / CHUNKS - 1, data.at(i).first));

                    int index = ((data.at(i).first * (TILE_WIDTH / CHUNKS)) + (TILE_HEIGHT / CHUNKS - 1));

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

void MapChunk::setHeight(int index, float height, QPoint textureCoords)
{
    if(index >= (TILE_WIDTH / CHUNKS) * (TILE_HEIGHT / CHUNKS))
        return;

    mapData[index] = height;

    terrainData->bind();
    terrainData->setHeight(height, QVector2D(textureCoords.x(), textureCoords.y()));

    chunkMaterial->setTextureUnitConfiguration(ShaderUnits::Heightmap, terrainData, terrainSampler, QByteArrayLiteral("heightMap"));
}

void MapChunk::setAlphas(int index, QVector<float> alphas, QPoint textureCoords)
{
    if(index >= (TILE_WIDTH / CHUNKS) * (TILE_HEIGHT / CHUNKS))
        return;

    for(int i = 0; i < alphas.count(); ++i)
    {
        alphaMapsData[i][index] = MathHelper::toUChar(alphas.at(i) * 255.0f);

        alphaMaps[i]->bind();
        alphaMaps[i]->setAlpha(alphaMapsData[i][index], QVector2D(textureCoords.x(), textureCoords.y()));

        QString uniformName = QString("layer%1Alpha").arg(i + 1);

        chunkMaterial->setTextureUnitConfiguration(ShaderUnits::Alphamap1 + i, alphaMaps[i], terrainSampler, uniformName.toLatin1());
    }
}

void MapChunk::setVertexShading(int index, QColor color, QPoint textureCoords)
{
    if(index + 3 >= (TILE_WIDTH / CHUNKS) * (TILE_HEIGHT / CHUNKS) * 4) // 4 channels
        return;

    unsigned char data[4];

    vertexShadingData[index]     = data[0] = MathHelper::toUChar(color.red());
    vertexShadingData[index + 1] = data[1] = MathHelper::toUChar(color.green());
    vertexShadingData[index + 2] = data[2] = MathHelper::toUChar(color.blue());
    vertexShadingData[index + 3] = data[3] = MathHelper::toUChar(color.alpha());

    vertexShadingMap->bind();
    vertexShadingMap->setVertexShade(data, QVector2D(textureCoords.x(), textureCoords.y()));

    chunkMaterial->setTextureUnitConfiguration(ShaderUnits::VertexShading, vertexShadingMap, terrainSampler, "vertexShading");
}

void MapChunk::setVertexLighting(int index, QColor color, QPoint textureCoords)
{
    if(index + 3 >= (TILE_WIDTH / CHUNKS) * (TILE_HEIGHT / CHUNKS) * 4) // 4 channels
        return;

    unsigned char data[4];

    vertexLightingData[index]     = data[0] = MathHelper::toUChar(color.red());
    vertexLightingData[index + 1] = data[1] = MathHelper::toUChar(color.green());
    vertexLightingData[index + 2] = data[2] = MathHelper::toUChar(color.blue());
    vertexLightingData[index + 3] = data[3] = MathHelper::toUChar(color.alpha());

    vertexLightingMap->bind();
    vertexLightingMap->setVertexShade(data, QVector2D(textureCoords.x(), textureCoords.y()));

    chunkMaterial->setTextureUnitConfiguration(ShaderUnits::VertexShading, vertexShadingMap, terrainSampler, "vertexLighting");
}

void MapChunk::setHeightFromWorld(QVector2D position, float height)
{
    int X = horizToHMapSize(position.x()) % TILE_WIDTH;
    int Y = horizToHMapSize(position.y()) % TILE_HEIGHT;

    X -= chunkX * (TILE_WIDTH / CHUNKS);
    Y -= chunkY * (TILE_HEIGHT / CHUNKS);

    int index = (Y * TILE_WIDTH / CHUNKS) + X;

    mapData[index] = height;

    terrainData->bind();
    terrainData->setHeight(height, QVector2D(X, Y));

    chunkMaterial->setTextureUnitConfiguration(ShaderUnits::Heightmap, terrainData, terrainSampler, QByteArrayLiteral("heightMap"));
}

void MapChunk::setAlphaFromWorld(QVector2D position, QVector<float> alphas)
{
    int X = horizToHMapSize(position.x()) % TILE_WIDTH;
    int Y = horizToHMapSize(position.y()) % TILE_HEIGHT;

    X -= chunkX * (TILE_WIDTH / CHUNKS);
    Y -= chunkY * (TILE_HEIGHT / CHUNKS);

    int index = (Y * TILE_WIDTH / CHUNKS) + X;

    for(int i = 0; i < alphas.count(); ++i)
    {
        alphaMapsData[i][index] = MathHelper::toUChar(alphas.at(i) * 255.0f);

        alphaMaps[i]->bind();
        alphaMaps[i]->setAlpha(alphaMapsData[i][index], QVector2D(X, Y));

        QString uniformName = QString("layer%1Alpha").arg(i + 1);

        chunkMaterial->setTextureUnitConfiguration(ShaderUnits::Alphamap1 + i, alphaMaps[i], terrainSampler, uniformName.toLatin1());
    }
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

void MapChunk::setTextureScaleOption(int option, int layer)
{
    textureScaleOption[layer] = (TextureScaleOption)option;
}

void MapChunk::setTextureScaleFar(double value, int layer)
{
    textureScaleFar[layer] = MathHelper::toFloat(value);
}

void MapChunk::setTextureScaleNear(double value, int layer)
{
    textureScaleNear[layer] = MathHelper::toFloat(value);
}

void MapChunk::setAutomaticTexture(bool enabled, int layer)
{
    // dynamic warning
    bool warn = true;

    for(int i = 1; i < MAX_TEXTURES; ++i)
    {
        if(i == layer)
        {
            warn = false;

            break;
        }
    }

    if(warn)
    {
        qWarning() << QObject::tr("Layer is out of range! Breaking function for safe working, report this to developer!");

        return;
    }

    --layer; // index is moved from 1-3 to 0-2

    automaticTexture[layer] = enabled;
}

void MapChunk::setAutomaticTextureStart(double value, int layer)
{
    // dynamic warning
    bool warn = true;

    for(int i = 1; i < MAX_TEXTURES; ++i)
    {
        if(i == layer)
        {
            warn = false;

            break;
        }
    }

    if(warn)
    {
        qWarning() << QObject::tr("Layer is out of range! Breaking function for safe working, report this to developer!");

        return;
    }

    --layer; // index is moved from 1-3 to 0-2

    automaticTextureStart[layer] = MathHelper::toFloat(value);
}

void MapChunk::setAutomaticTextureEnd(double value, int layer)
{
    // dynamic warning
    bool warn = true;

    for(int i = 1; i < MAX_TEXTURES; ++i)
    {
        if(i == layer)
        {
            warn = false;

            break;
        }
    }

    if(warn)
    {
        qWarning() << QObject::tr("Layer is out of range! Breaking function for safe working, report this to developer!");

        return;
    }

    --layer; // index is moved from 1-3 to 0-2

    automaticTextureEnd[layer] = MathHelper::toFloat(value);
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

    // Texture scale
    for(int i = 0; i < MAX_TEXTURES; ++i)
    {
        chunk->terrainOffset->textureScale[i]     = textureScaleOption[i];
        chunk->terrainOffset->textureScaleFar[i]  = textureScaleFar[i];
        chunk->terrainOffset->textureScaleNear[i] = textureScaleNear[i];
    }

    // Automatic texture
    for(int i = 0; i < MAX_TEXTURES - 1; ++i)
    {
        chunk->terrainOffset->automaticTexture[i]      = automaticTexture[i];
        chunk->terrainOffset->automaticTextureStart[i] = automaticTextureStart[i];
        chunk->terrainOffset->automaticTextureEnd[i]   = automaticTextureEnd[i];
    }
}
