#ifndef MAPTILE_H
#define MAPTILE_H

#include "mapchunk.h"
#include "world.h"
#include "mapheaders.h"

#include "texturearray.h"

class MapTile
{
public:
    MapTile(World* mWorld, int x, int y, const QString& mapFile);
    ~MapTile();

    int coordX, coordY;

    MapChunk* getChunk(int x, int y);

    void draw(const float& distance, const QVector3D& camera);

    bool isTile(int pX, int pY);

    void saveTile();

    void test();

private:
    QString fileName;

    MapChunk* mapChunks[CHUNKS][CHUNKS];

    World* world;

    QOpenGLVertexArrayObject vao;
    QOpenGLBuffer            patchBuffer;

    QVector<float> positionData;

    int patchCount;

    SamplerPtr      terrainSampler;
    TextureArrayPtr terrainMapData;

    friend class MapChunk;
};

#endif // MAPTILE_H
