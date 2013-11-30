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

#include "material.h"
#include "world.h"

enum MapChunkBorder
{
    Top    = 0,
    Right  = 1,
    Bottom = 2,
    Left   = 3
};

class Broadcast
{
public:
    Broadcast() : broadcastHeight(false)
    {
    }

    Broadcast(QVector<QPair<int, float>> top, QVector<QPair<int, float>> right, QVector<QPair<int, float>> bot, QVector<QPair<int, float>> left)
    : broadcastHeight(true)
    , topData(top)
    , rightData(right)
    , bottomData(bot)
    , leftData(left)
    {
    }

    ~Broadcast()
    {
    }

    const bool isBroadcasting() const { return broadcastHeight; }

    const QVector<QPair<int, float>>& getTopData() const    { return topData; }
    const QVector<QPair<int, float>>& getRightData() const  { return rightData; }
    const QVector<QPair<int, float>>& getBottomData() const { return bottomData; }
    const QVector<QPair<int, float>>& getLeftData() const   { return leftData; }

    void stop()
    {
        broadcastHeight = false;
    }

private:
    QVector<QPair<int, float>> topData;
    QVector<QPair<int, float>> rightData;
    QVector<QPair<int, float>> bottomData;
    QVector<QPair<int, float>> leftData;

    bool broadcastHeight;
};

class MapChunk
{
public:
    MapChunk(World* mWorld, MapTile* tile, int x, int y);
    MapChunk(World* mWorld, MapTile* tile, QFile& file, int x, int y);
    ~MapChunk();

    bool isInVisibleRange(const float& distance, const QVector3D& camera) const;

    void draw();

    bool changeTerrain(float x , float z, float change         , float radius, int brush, int brush_type);
    bool flattenTerrain(float x, float z, float y, float change, float radius, int brush, int brush_type);
    bool blurTerrain(float x   , float z, float change         , float radius, int brush, int brush_type);

    bool paintTerrain();

    const float getHeight(const float& x, const float& y) const;
    const float getHeight(const int& x, const int& y) const;
    const float getMapData(const int& index) const;

    const int chunkIndex() const;

    const QVector2D  getBases() const                     { return QVector2D(baseX, baseY); }
    const GLuint&    getDisplaySubroutines() const        { return displaySubroutines[world->displayMode()]; }
    const Broadcast* getBroadcast() const                 { return broadcast; }

    QOpenGLShaderProgramPtr getShader() const { return chunkMaterial->shader(); }

    void save(MCNK* chunk);

    void setBorderHeight(const QVector<QPair<int, float>> data, MapChunkBorder border);

    void test();

private:
    World* world;

    QOpenGLVertexArrayObject vao;
    QOpenGLBuffer            patchBuffer;

    QVector<float> positionData;

    int patchCount;

    ChunkMaterialPtr chunkMaterial;

    TexturePtr terrainData;
    SamplerPtr terrainSampler;

    QVector<GLuint> displaySubroutines;

    int chunkX, chunkY;

    float baseX, baseY;
    float chunkBaseX, chunkBaseY;

    Broadcast* broadcast;

    int horizToHMapSize(float position);
    float HMapSizeToHoriz(int position);

    void broadcastBorderHeight(QVector<QPair<int, float>> data, MapChunkBorder border);

    float* mapData;

    void initialize();
};

#endif // MAPCHUNK_H
