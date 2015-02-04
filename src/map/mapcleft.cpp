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
    world->getGLFunctions()->glActiveTexture(GL_TEXTURE0 + ShaderUnits::Textures);

    chunkMaterial->setTextureUnitConfiguration(ShaderUnits::Textures, previous->textureArray, world->getTextureManager()->getSampler(), "textures");

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
        cleftData = new float[14 * CHUNK_WIDTH];

        // top line
        for(int i = 0; i < CHUNK_WIDTH; ++i)
            cleftData[i] = previous->getHeight(i, CHUNK_HEIGHT - 7);

        for(int i = 0; i < CHUNK_WIDTH; ++i)
            cleftData[i + CHUNK_WIDTH] = previous->getHeight(i, CHUNK_HEIGHT - 6);

        for(int i = 0; i < CHUNK_WIDTH; ++i)
            cleftData[i + CHUNK_WIDTH * 2] = previous->getHeight(i, CHUNK_HEIGHT - 5);

        for(int i = 0; i < CHUNK_WIDTH; ++i)
            cleftData[i + CHUNK_WIDTH * 3] = previous->getHeight(i, CHUNK_HEIGHT - 4);

        for(int i = 0; i < CHUNK_WIDTH; ++i)
            cleftData[i + CHUNK_WIDTH * 7] = previous->getHeight(i, CHUNK_HEIGHT - 3);

        for(int i = 0; i < CHUNK_WIDTH; ++i)
            cleftData[i + CHUNK_WIDTH * 5] = previous->getHeight(i, CHUNK_HEIGHT - 2);

        for(int i = 0; i < CHUNK_WIDTH; ++i)
            cleftData[i + CHUNK_WIDTH * 6] = previous->getHeight(i, CHUNK_HEIGHT - 1);

        // bottom line
        for(int i = 0; i < CHUNK_WIDTH; ++i)
            cleftData[i + CHUNK_WIDTH * 7] = next->getHeight(i, 0);

        for(int i = 0; i < CHUNK_WIDTH; ++i)
            cleftData[i + CHUNK_WIDTH * 8] = next->getHeight(i, 1);

        for(int i = 0; i < CHUNK_WIDTH; ++i)
            cleftData[i + CHUNK_WIDTH * 9] = next->getHeight(i, 2);

        for(int i = 0; i < CHUNK_WIDTH; ++i)
            cleftData[i + CHUNK_WIDTH * 10] = next->getHeight(i, 3);

        for(int i = 0; i < CHUNK_WIDTH; ++i)
            cleftData[i + CHUNK_WIDTH * 11] = next->getHeight(i, 4);

        for(int i = 0; i < CHUNK_WIDTH; ++i)
            cleftData[i + CHUNK_WIDTH * 12] = next->getHeight(i, 5);

        for(int i = 0; i < CHUNK_WIDTH; ++i)
            cleftData[i + CHUNK_WIDTH * 13] = next->getHeight(i, 6);

        // top line
        /*for(int i = 0; i < CHUNK_WIDTH; ++i)
            cleftData[i] = previous->getHeight(i, CHUNK_HEIGHT - 1);

        // bottom line
        for(int i = 0; i < CHUNK_WIDTH; ++i)
            cleftData[i + CHUNK_WIDTH] = next->getHeight(i, 0);*/
    }
    else // verticall
    {
        cleftData = new float[14 * CHUNK_HEIGHT];

        // left line
        for(int i = 0; i < CHUNK_HEIGHT; ++i)
            cleftData[i * 14] = previous->getHeight(CHUNK_WIDTH - 7, i);

        for(int i = 0; i < CHUNK_HEIGHT; ++i)
            cleftData[i * 14 + 1] = previous->getHeight(CHUNK_WIDTH - 6, i);

        for(int i = 0; i < CHUNK_HEIGHT; ++i)
            cleftData[i * 14 + 2] = previous->getHeight(CHUNK_WIDTH - 5, i);

        for(int i = 0; i < CHUNK_HEIGHT; ++i)
            cleftData[i * 14 + 3] = previous->getHeight(CHUNK_WIDTH - 4, i);

        for(int i = 0; i < CHUNK_HEIGHT; ++i)
            cleftData[i * 14 + 4] = previous->getHeight(CHUNK_WIDTH - 3, i);

        for(int i = 0; i < CHUNK_HEIGHT; ++i)
            cleftData[i * 14 + 5] = previous->getHeight(CHUNK_WIDTH - 2, i);

        for(int i = 0; i < CHUNK_HEIGHT; ++i)
            cleftData[i * 14 + 6] = previous->getHeight(CHUNK_WIDTH - 1, i);

        // right line
        for(int i = 0; i < CHUNK_HEIGHT; ++i)
            cleftData[i * 14 + 7] = next->getHeight(0, i);

        for(int i = 0; i < CHUNK_HEIGHT; ++i)
            cleftData[i * 14 + 8] = next->getHeight(1, i);

        for(int i = 0; i < CHUNK_HEIGHT; ++i)
            cleftData[i * 14 + 9] = next->getHeight(2, i);

        for(int i = 0; i < CHUNK_HEIGHT; ++i)
            cleftData[i * 14 + 10] = next->getHeight(3, i);

        for(int i = 0; i < CHUNK_HEIGHT; ++i)
            cleftData[i * 14 + 11] = next->getHeight(4, i);

        for(int i = 0; i < CHUNK_HEIGHT; ++i)
            cleftData[i * 14 + 12] = next->getHeight(5, i);

        for(int i = 0; i < CHUNK_HEIGHT; ++i)
            cleftData[i * 14 + 13] = next->getHeight(6, i);

        // left line
        /*for(int i = 0; i < CHUNK_HEIGHT; ++i)
            cleftData[i * 2] = previous->getHeight(CHUNK_WIDTH - 1, i);

        // right line
        for(int i = 0; i < CHUNK_HEIGHT; ++i)
            cleftData[i * 2 + 1] = next->getHeight(0, i);*/
    }

    world->getGLFunctions()->glActiveTexture(GL_TEXTURE0 + ShaderUnits::Heightmap);

    const int width  = orientation == MapChunk::Horizontal ? CHUNK_WIDTH  : 14;
    const int height = orientation == MapChunk::Vertical   ? CHUNK_HEIGHT : 14;

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
    /*const int maxTessellationLevel     = 64;
    const int trianglesPerHeightSample = 10;

    const int width  = CHUNK_WIDTH;//orientation == MapChunk::Horizontal ? CHUNK_WIDTH  : 2;
    const int height = CHUNK_HEIGHT;//orientation == MapChunk::Vertical   ? CHUNK_HEIGHT : 2;

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
    }*/

    const int size = orientation == MapChunk::Horizontal ? CHUNK_WIDTH : CHUNK_HEIGHT;

    QVector<float> positionData(size * 4 * 3); // 3 floats per vertex, top and bottom * 2

    if(orientation == MapChunk::Horizontal)
    {
        for(int i = 0; i < size * 4; i += 4)
        {
            // prev data left
            positionData[i * 3]     = next->chunkBaseX + (CHUNKSIZE / CHUNK_WIDTH * (i / 4));
            positionData[i * 3 + 1] = previous->getHeight(i / 4, CHUNK_HEIGHT - 1);
            positionData[i * 3 + 2] = next->chunkBaseY;

            // prev data right
            positionData[(i + 1) * 3]     = next->chunkBaseX + (CHUNKSIZE / CHUNK_WIDTH * (i / 4 + 1));
            positionData[(i + 1) * 3 + 1] = previous->getHeight(i / 4 + 1, CHUNK_HEIGHT - 1);
            positionData[(i + 1) * 3 + 2] = next->chunkBaseY;

            // next data right
            positionData[(i + 2) * 3]     = next->chunkBaseX + (CHUNKSIZE / CHUNK_WIDTH * (i / 4 + 1));
            positionData[(i + 2) * 3 + 1] = next->getHeight(i / 4 + 1, 0);
            positionData[(i + 2) * 3 + 2] = next->chunkBaseY;

            // next data left
            positionData[(i + 3) * 3]     = next->chunkBaseX + (CHUNKSIZE / CHUNK_WIDTH * (i / 4));
            positionData[(i + 3) * 3 + 1] = next->getHeight(i / 4, 0);
            positionData[(i + 3) * 3 + 2] = next->chunkBaseY;

        }
    }
    else
    {
        for(int i = 0; i < size * 4; i += 4)
        {
            // prev data top
            positionData[i * 3]     = next->chunkBaseX;
            positionData[i * 3 + 1] = previous->getHeight(CHUNK_WIDTH - 1, i / 4);
            positionData[i * 3 + 2] = next->chunkBaseY + (CHUNKSIZE / CHUNK_HEIGHT * (i / 4));

            // prev data bot
            positionData[(i + 1) * 3]     = next->chunkBaseX;
            positionData[(i + 1) * 3 + 1] = previous->getHeight(CHUNK_WIDTH - 1, i / 4 + 1);
            positionData[(i + 1) * 3 + 2] = next->chunkBaseY + (CHUNKSIZE / CHUNK_HEIGHT * (i / 4 + 1));

            // next data bot
            positionData[(i + 2) * 3]     = next->chunkBaseX;
            positionData[(i + 2) * 3 + 1] = next->getHeight(0, i / 4 + 1);
            positionData[(i + 2) * 3 + 2] = next->chunkBaseY + (CHUNKSIZE / CHUNK_HEIGHT * (i / 4 + 1));

            // next data top
            positionData[(i + 3) * 3]     = next->chunkBaseX;
            positionData[(i + 3) * 3 + 1] = next->getHeight(0, i / 4);
            positionData[(i + 3) * 3 + 2] = next->chunkBaseY + (CHUNKSIZE / CHUNK_HEIGHT * (i / 4));
        }
    }

    /*QVector<float> positionData(12);

    positionData[0] = 0.0f;
    positionData[1] = 15.0f;
    positionData[2] = 0.0f;

    positionData[3] = 133.0f;
    positionData[4] = 15.0f;
    positionData[5] = 0.0f;

    positionData[6] = 133.0f;
    positionData[7] = 15.0f;
    positionData[8] = 133.0f;

    positionData[9] = 0.0f;
    positionData[10] = 15.0f;
    positionData[11] = 133.0f;*/

    mesh.createVertexArrayObject();
    mesh.createBuffer(Mesh::Vertices, positionData.data(), positionData.size() * sizeof(float));
    mesh.setNumFaces(size * 4);
    //mesh.setNumFaces(4);
}

void MapCleft::draw(QOpenGLShaderProgram* shader)
{
    /*chunkMaterial->bind(shader);

    if(orientation == MapChunk::Horizontal)
    {
        shader->setUniformValue("baseX", next->chunkBaseX);
        shader->setUniformValue("baseY", next->chunkBaseY - (CHUNKSIZE / CHUNK_HEIGHT * 7));//- 0.005f);
    }
    else
    {
        shader->setUniformValue("baseX", next->chunkBaseX - (CHUNKSIZE / CHUNK_WIDTH * 7));//- 0.001f);//(CHUNKSIZE / CHUNK_WIDTH * 4));
        shader->setUniformValue("baseY", next->chunkBaseY);
    }

    shader->setUniformValue("chunkX", next->chunkX);
    shader->setUniformValue("chunkY", next->chunkY);

    // Get subroutine indices
    for(int i = 0; i < world->DisplayModeCount; ++i)
        displaySubroutines[i] = world->getGLFunctions()->glGetSubroutineIndex(shader->programId(), GL_FRAGMENT_SHADER, world->displayName(i).toLatin1());

    // Set the fragment shader display mode subroutine
    world->getGLFunctions()->glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &getDisplaySubroutines());*/

    //if(world->displayMode() != world->Hidden)
    //{
        // Render the quad as a patch
        {
            mesh.bind();
            //mesh.createAttributeArray(Mesh::Vertices, shader, "vertexPosition", GL_FLOAT, 0, 2);
            mesh.createAttributeArray(Mesh::Vertices, shader, "vertexPosition", GL_FLOAT, 0, 3);

            shader->setPatchVertexCount(4);

            world->getGLFunctions()->glDrawArrays(GL_PATCHES, 0, mesh.getNumFaces());
        }
    //}
}

bool MapCleft::isInVisibleRange(const float &distance, const QVector3D &camera) const
{
    static const float chunkRadius = sqrtf(CHUNKSIZE * CHUNKSIZE / 2.0f);

    return (camera.length() - chunkRadius) < distance;
}
