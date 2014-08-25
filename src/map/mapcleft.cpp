#include "mapcleft.h"

MapCleft::MapCleft(World* mWorld, MapTile* tile, MapChunk::Border orient, MapChunk* prevChunk, MapChunk* nextChunk)
: world(mWorld)
, terrainSampler(tile->terrainSampler.data())
, terrainData(new Texture(QOpenGLTexture::Target2D))
, displaySubroutines(world->DisplayModeCount)
, orientation(orient)
, previous(prevChunk)
, next(nextChunk)
{
    chunkMaterial = new Material();

    ///////////
    for(int i = 0; i < MAX_TEXTURES; ++i)
    {
        world->getGLFunctions()->glActiveTexture(GL_TEXTURE0 + ShaderUnits::Texture1 + ToGLuint(i));

        QString uniformName = QString("layer%1Texture").arg(i);

        if(i == 0)
            uniformName = "baseTexture";

        chunkMaterial->setTextureUnitConfiguration(ShaderUnits::Texture1 + i, previous->textures[i], world->getTextureManager()->getSampler(), uniformName.toLatin1());
    }

    for(int i = 0; i < ALPHAMAPS; ++i)
    {
        world->getGLFunctions()->glActiveTexture(GL_TEXTURE0 + ShaderUnits::Alphamap1 + ToGLuint(i));

        QString alphaMapLayer = QString("layer%1Alpha").arg(i + 1);

        chunkMaterial->setTextureUnitConfiguration(ShaderUnits::Alphamap1 + i, previous->alphaMaps[i], terrainSampler, alphaMapLayer.toLatin1());
    }

    world->getGLFunctions()->glActiveTexture(GL_TEXTURE0 + ShaderUnits::VertexShading);

    chunkMaterial->setTextureUnitConfiguration(ShaderUnits::VertexShading, previous->vertexShadingMap, terrainSampler, "vertexShading");

    //-////////

    // fill data
    if(orientation == MapChunk::Horizontal)
    {
        cleftData = new float[2 * CHUNK_WIDTH];

        /*// top line
        for(int i = 0; i < CHUNK_WIDTH; ++i)
            cleftData[i] = previous->getHeight(i, CHUNK_HEIGHT - 2);

        for(int i = 0; i < CHUNK_WIDTH; ++i)
            cleftData[i + CHUNK_WIDTH] = previous->getHeight(i, CHUNK_HEIGHT - 1);

        // bottom line
        for(int i = 0; i < CHUNK_WIDTH; ++i)
            cleftData[i + CHUNK_WIDTH * 2] = next->getHeight(i, 0);

        for(int i = 0; i < CHUNK_WIDTH; ++i)
            cleftData[i + CHUNK_WIDTH * 3] = next->getHeight(i, 1);*/

        // top line
        for(int i = 0; i < CHUNK_WIDTH; ++i)
            cleftData[i] = previous->getHeight(i, CHUNK_HEIGHT - 1);

        // bottom line
        for(int i = 0; i < CHUNK_WIDTH; ++i)
            cleftData[i + CHUNK_WIDTH] = next->getHeight(i, 0);
    }
    else // verticall
    {
        cleftData = new float[2 * CHUNK_HEIGHT];

        /*// left line
        for(int i = 0; i < CHUNK_HEIGHT; ++i)
            cleftData[i * 4] = previous->getHeight(CHUNK_WIDTH - 2, i);

        for(int i = 0; i < CHUNK_HEIGHT; ++i)
            cleftData[i * 4 + 1] = previous->getHeight(CHUNK_WIDTH - 1, i);

        // right line
        for(int i = 0; i < CHUNK_HEIGHT; ++i)
            cleftData[i * 4 + 2] = next->getHeight(0, i);

        for(int i = 0; i < CHUNK_HEIGHT; ++i)
            cleftData[i * 4 + 3] = next->getHeight(1, i);*/

        // left line
        for(int i = 0; i < CHUNK_HEIGHT; ++i)
            cleftData[i * 2] = previous->getHeight(CHUNK_WIDTH - 1, i);

        // right line
        for(int i = 0; i < CHUNK_HEIGHT; ++i)
            cleftData[i * 2 + 1] = next->getHeight(0, i);
    }

    world->getGLFunctions()->glActiveTexture(GL_TEXTURE0 + ShaderUnits::Heightmap);

    const int width  = orientation == MapChunk::Horizontal ? CHUNK_WIDTH  : 2;
    const int height = orientation == MapChunk::Vertical   ? CHUNK_HEIGHT : 2;

    terrainData->setSize(width, height);
    terrainData->setHeightmap(cleftData);

    chunkMaterial->setTextureUnitConfiguration(ShaderUnits::Heightmap, terrainData, terrainSampler, QByteArrayLiteral("heightMap"));

    initialize();
}

MapCleft::~MapCleft()
{
    delete[] cleftData;

    cleftData = NULL;
}

void MapCleft::initialize()
{
    /// Create a Vertex Buffers
    const int maxTessellationLevel     = 64;
    const int trianglesPerHeightSample = 10;

    const int width  = orientation == MapChunk::Horizontal ? CHUNK_WIDTH  : 2;
    const int height = orientation == MapChunk::Vertical   ? CHUNK_HEIGHT : 2;

    int xDivisions = trianglesPerHeightSample * width  / maxTessellationLevel;
    int zDivisions = trianglesPerHeightSample * height / maxTessellationLevel;

    if(xDivisions == 0)
        xDivisions = 10;

    if(zDivisions == 0)
        zDivisions = 10;

    int patchCount = xDivisions * zDivisions;

    QVector<float> positionData(2 * patchCount); // 2 floats per vertex

    qDebug() << QObject::tr("Total number of patches for mapcleft =") << patchCount;

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
    mesh.createBuffer(IMesh::Vertices, positionData.data(), positionData.size() * sizeof(float));
    mesh.setNumFaces(patchCount);
}

void MapCleft::draw(QOpenGLShaderProgram* shader)
{
    chunkMaterial->bind(shader);

    if(orientation == MapChunk::Horizontal)
    {
        shader->setUniformValue("baseX", next->chunkBaseX);
        shader->setUniformValue("baseY", next->chunkBaseY - 0.005f);
    }
    else
    {
        shader->setUniformValue("baseX", next->chunkBaseX - 0.001f);//(CHUNKSIZE / CHUNK_WIDTH * 4));
        shader->setUniformValue("baseY", next->chunkBaseY);
    }

    shader->setUniformValue("chunkX", next->chunkX);
    shader->setUniformValue("chunkY", next->chunkY);

    // Get subroutine indices
    for(int i = 0; i < world->DisplayModeCount; ++i)
        displaySubroutines[i] = world->getGLFunctions()->glGetSubroutineIndex(shader->programId(), GL_FRAGMENT_SHADER, world->displayName(i).toLatin1());

    // Set the fragment shader display mode subroutine
    world->getGLFunctions()->glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &getDisplaySubroutines());

    //if(world->displayMode() != world->Hidden)
    //{
        // Render the quad as a patch
        {
            mesh.bind();
            mesh.createAttributeArray(IMesh::Vertices, shader, "vertexPosition", GL_FLOAT, 0, 2);

            shader->setPatchVertexCount(1);

            world->getGLFunctions()->glDrawArrays(GL_PATCHES, 0, mesh.getNumFaces());
        }
    //}
}

bool MapCleft::isInVisibleRange(const float &distance, const QVector3D &camera) const
{
    static const float chunkRadius = sqrtf(CHUNKSIZE * CHUNKSIZE / 2.0f);

    return (camera.length() - chunkRadius) < distance;
}
