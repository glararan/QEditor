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

#include "waterchunk.h"

#include "qeditor.h"

WaterChunk::WaterChunk(World* mWorld, int x, int y, Sampler* sampler, int tileX, int tileY)
: world(mWorld)
, waterSampler(sampler)
, waterSurface(new Texture(QOpenGLTexture::Target2D))
, data(false)
, displaySubroutines(world->DisplayModeCount)
, chunkX(x)
, chunkY(y)
, baseX(chunkX * CHUNKSIZE)
, baseY(chunkY * CHUNKSIZE)
, chunkBaseX((tileX * TILESIZE) + baseX)
, chunkBaseY((tileY * TILESIZE) + baseY)
{
    chunkMaterial = new Material();

    /// Set Water texture
    world->getGLFunctions()->glActiveTexture(GL_TEXTURE0 + ShaderUnits::Texture1);

    if(!world->getTextureManager()->hasTexture("waterTexture", "textures/water.png"))
        world->getTextureManager()->loadTexture("waterTexture", "textures/water.png");

    waterTexture = world->getTextureManager()->getTexture("waterTexture");

    chunkMaterial->setTextureUnitConfiguration(ShaderUnits::Texture1, waterTexture, world->getTextureManager()->getSampler(), QByteArrayLiteral("baseTexture"));

    /// Water
    waterData = new float[CHUNK_ARRAY_UC_SIZE]; // chunk_array_size

    memset(waterData, 0, sizeof(float) * CHUNK_ARRAY_UC_SIZE);

    world->getGLFunctions()->glActiveTexture(GL_TEXTURE0 + ShaderUnits::Heightmap);

    waterSurface->setSize(MAP_WIDTH / CHUNKS, MAP_HEIGHT / CHUNKS);
    waterSurface->setHeightmap(waterData);

    chunkMaterial->setTextureUnitConfiguration(ShaderUnits::Heightmap, waterSurface, waterSampler, QByteArrayLiteral("heightMap"));

    //
    BorderHeights bh;
    bh.top = bh.right = bh.bottom = bh.left = 0;

    heights = bh;

    initialize();
}

WaterChunk::WaterChunk(World* mWorld, int x, int y, Sampler* sampler, int tileX, int tileY, QFile& file)
: world(mWorld)
, waterSampler(sampler)
, waterSurface(new Texture(QOpenGLTexture::Target2D))
, data(false)
, displaySubroutines(world->DisplayModeCount)
, chunkX(x)
, chunkY(y)
, baseX(chunkX * CHUNKSIZE)
, baseY(chunkY * CHUNKSIZE)
, chunkBaseX((tileX * TILESIZE) + baseX)
, chunkBaseY((tileY * TILESIZE) + baseY)
{
    chunkMaterial = new Material();
    /// Set Water texture
    world->getGLFunctions()->glActiveTexture(GL_TEXTURE0 + ShaderUnits::Texture1);

    if(!world->getTextureManager()->hasTexture("waterTexture", "textures/water.png"))
        world->getTextureManager()->loadTexture("waterTexture", "textures/water.png");

    waterTexture = world->getTextureManager()->getTexture("waterTexture");

    chunkMaterial->setTextureUnitConfiguration(ShaderUnits::Texture1, waterTexture, world->getTextureManager()->getSampler(), QByteArrayLiteral("baseTexture"));

    /// Water - todo file load
    waterData = new float[CHUNK_ARRAY_UC_SIZE]; // chunk_array_size

    memset(waterData, 0, sizeof(float) * CHUNK_ARRAY_UC_SIZE);

    world->getGLFunctions()->glActiveTexture(GL_TEXTURE0 + ShaderUnits::Heightmap);

    waterSurface->setSize(MAP_WIDTH / CHUNKS, MAP_HEIGHT / CHUNKS);
    waterSurface->setHeightmap(waterData);

    chunkMaterial->setTextureUnitConfiguration(ShaderUnits::Heightmap, waterSurface, waterSampler, QByteArrayLiteral("heightMap"));

    //
    BorderHeights bh;
    bh.top = bh.right = bh.bottom = bh.left = 0;

    heights = bh;

    initialize();
}

WaterChunk::~WaterChunk()
{
    delete[] waterData;

    waterData = NULL;
}

void WaterChunk::initialize()
{
    /// Create a Vertex Buffers
    const int maxTessellationLevel     = 64;
    const int trianglesPerHeightSample = 10;

    const int xDivisions = trianglesPerHeightSample * MAP_WIDTH  / CHUNKS / maxTessellationLevel;
    const int zDivisions = trianglesPerHeightSample * MAP_HEIGHT / CHUNKS / maxTessellationLevel;

    int patchCount = xDivisions * zDivisions;

    QVector<float> positionData(2 * patchCount); // 2 floats per vertex

    qDebug() << QObject::tr("Total number of patches for waterchunk =") << patchCount;

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

void WaterChunk::draw(QOpenGLShaderProgram* shader, GLuint reflectionTexture, GLuint depthTexture)
{
    if(data)
    {
        setReflectionTexture(reflectionTexture, depthTexture);

        shader->setUniformValue("baseX", chunkBaseX);
        shader->setUniformValue("baseY", chunkBaseY);

        shader->setUniformValue("chunkX", chunkX);
        shader->setUniformValue("chunkY", chunkY);

        shader->setUniformValue("eyePos", world->getCamera()->position());

        chunkMaterial->bind(shader);

        // Set the fragment shader display mode subroutine
        world->getGLFunctions()->glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &getDisplaySubroutines());

        // Render the quad as a patch
        {
            Mesh.bind();
            Mesh.createAttributeArray(IMesh::Vertices, shader, "vertexPosition", GL_FLOAT, 0, 2);

            shader->setPatchVertexCount(1);

            world->getGLFunctions()->glDrawArrays(GL_PATCHES, 0, Mesh.getNumFaces());
        }
    }
}

void WaterChunk::updateData()
{
    if(data)
    {
        float top   = heights.top;
        float right = heights.right;
        float bot   = heights.bottom;
        float left  = heights.left;

        float topleft  = (top + left) / 2;
        float topright = (top + right) / 2;
        float botleft  = (bot + left) / 2;
        float botright = (bot + right) / 2;

        // row column
        int rc = MAP_WIDTH / CHUNKS;

        // calc row min to row max, cell min to cell max first and last index
        for(int y = 0; y < rc; ++y)
        {
            waterData[y * rc]          = topleft - (topleft - botleft) / (rc - 1) * y;
            waterData[y * rc + rc - 1] = topright - (topright - botright) / (rc - 1) * y;
        }

        for(int x = 0; x < rc; ++x)
        {
            waterData[x]                 = topleft - (topleft - topright) / (rc - 1) * x;
            waterData[x + (rc - 1) * rc] = botleft - (botleft - botright) / (rc - 1) * x;
        }

        // calc data
        for(int x = 0; x < rc; ++x)
        {
            if(x <= 0 || x >= rc - 1)
                continue;

            for(int y = 0; y < rc; ++y)
            {
                if(y <= 0 || y >= rc - 1)
                    continue;

                float currLeft  = waterData[y * rc];
                float currRight = waterData[y * rc + rc - 1];

                waterData[x + rc * y] = currLeft - ((currLeft - currRight) / (rc - 1)) * x;
            }
        }

        waterSurface->bind();
        waterSurface->setHeightmap(waterData);

        chunkMaterial->setTextureUnitConfiguration(ShaderUnits::Heightmap, waterSurface, waterSampler, QByteArrayLiteral("heightMap"));
    }
}

const int WaterChunk::chunkIndex() const
{
    return (chunkY * CHUNKS) + chunkX;
}

float WaterChunk::getHeight() const
{
    float average = 0.0f;

    for(int i = 0; i < CHUNK_ARRAY_UC_SIZE; ++i)
        average += waterData[i];

    average /= CHUNK_ARRAY_UC_SIZE;

    return average;
}

float WaterChunk::getHeight(int x, int y) const
{
    int X = x % MAP_WIDTH;
    int Y = y % MAP_HEIGHT;

    int index = (Y * MAP_WIDTH / CHUNKS) + X;

    return waterData[index * sizeof(float)];
}

void WaterChunk::setHeight(float height)
{
    for(int i = 0; i < CHUNK_ARRAY_UC_SIZE; ++i)
        waterData[i] = height;

    waterSurface->setHeightmap(waterData);

    chunkMaterial->setTextureUnitConfiguration(ShaderUnits::Heightmap, waterSurface, waterSampler, QByteArrayLiteral("heightMap"));
}

void WaterChunk::setHeight(int x, int y, float height)
{
    waterData[y * MAP_WIDTH / CHUNKS + x] = height;

    waterSurface->setHeight(height, QVector2D(x, y), true);

    chunkMaterial->setTextureUnitConfiguration(ShaderUnits::Heightmap, waterSurface, waterSampler, QByteArrayLiteral("heightMap"));
}

void WaterChunk::setHeights(BorderHeights borderHeights)
{
    heights = borderHeights;
}

void WaterChunk::save()
{

}

void WaterChunk::setReflectionTexture(GLuint reflectionTexture, GLuint depthTexture)
{
    /*world->getGLFunctions()->glBindTexture(GL_TEXTURE_2D, reflectionTexture);

    chunkMaterial->setFramebufferUnitConfiguration(ShaderUnits::Texture2, reflectionTexture, QByteArrayLiteral("reflectionTexture"));

    world->getGLFunctions()->glBindTexture(GL_TEXTURE_2D, depthTexture);

    chunkMaterial->setFramebufferUnitConfiguration(ShaderUnits::Texture3, depthTexture, QByteArrayLiteral("depthTexture"));*/
}