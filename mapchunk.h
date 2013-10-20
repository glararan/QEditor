#ifndef MAPCHUNK_H
#define MAPCHUNK_H

#include <QOpenGLContext>
#include <QOpenGLFunctions_4_0_Core>
#include <QOpenGLBuffer>
#include <QOpenGLDebugLogger>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>

#include <QSharedPointer>
#include <QVector3D>
#include <QFile>

#include "world.h"

class MapChunk
{
public:
    MapChunk(World* mWorld, MapTile* tile, QFile* file, int x, int y);
    ~MapChunk();

    bool isInVisibleRange(const float& distance, const QVector3D& camera) const;

    void draw();

    bool changeTerrain(float x , float z, float change         , float radius, int brush, int brush_type);
    bool flattenTerrain(float x, float z, float y, float change, float radius, int brush, int brush_type);
    bool blurTerrain(float x   , float z, float change         , float radius, int brush, int brush_type);

    bool paintTerrain();

    const float getHeight(const float& x, const float& y) const;
    const float getMapData(const int& index) const;

    const int chunkIndex() const;

    const QVector2D getBases() const;

    void test();

private:
    World* world;

    TextureArrayPtr terrainData;
    SamplerPtr      terrainSampler;

    int chunkX, chunkY;
    int baseX, baseY;

    float mapData[CHUNK_ARRAY_SIZE];

    int horizToHMapSize(float position);
    float HMapSizeToHoriz(int position);
};

#endif // MAPCHUNK_H
