#ifndef MAPTILE_H
#define MAPTILE_H

#include "mapchunk.h"
#include "world.h"
#include "mapheaders.h"

#include "texturearray.h"

class MapTile
{
public:
    MapTile(World* mWorld, const QString& mapFile, int x, int y);
    MapTile(World* mWorld, int x, int y, const QString& mapFile);
    ~MapTile();

    int coordX, coordY;

    MapChunk* getChunk(int x, int y);

    const MapHeader& getHeader() const { return tileHeader; }

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

    MapHeader tileHeader;

    friend class MapChunk;
};

#endif // MAPTILE_H
