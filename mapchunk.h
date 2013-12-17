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

class MapChunk
{
public:
    enum Border
    {
        HORIZONTAL,
        VERTICAL
    };

    MapChunk(World* mWorld, MapTile* tile, int x, int y);
    MapChunk(World* mWorld, MapTile* tile, QFile& file, int x, int y);
    ~MapChunk();

    bool isInVisibleRange(const float& distance, const QVector3D& camera) const;

    void draw();

    //Neighbours
    void setBottomNeighbour(MapChunk *bottomNeighbour);
    void setLeftNeighbour(MapChunk *leftNeighbour);

    /// Terrain
    bool changeTerrain(float x , float z, float change         , float radius, int brush, int brush_type);
    bool flattenTerrain(float x, float z, float y, float change, float radius, int brush, int brush_type);
    bool blurTerrain(float x   , float z, float change         , float radius, int brush, int brush_type);

    bool paintTerrain();

    /// Get
    const float getHeight(const float& x, const float& y) const;
    const float getHeight(const int& x, const int& y) const;
    const float getMapData(const int& index) const;

    const int chunkIndex() const;

    const QVector2D  getBases() const                     { return QVector2D(baseX, baseY); }
    const GLuint&    getDisplaySubroutines() const        { return displaySubroutines[world->displayMode()]; }

    QOpenGLShaderProgramPtr getShader() const { return chunkMaterial->shader(); }

    /// Set
    void setBorder(MapChunk::Border border, QVector<QPair<int, float> > &data);

    /// ...
    void save(MCNK* chunk);

    void test();

private:
    World* world;
    MapChunk *bottomNeighbour;
    MapChunk *leftNeighbour;

    /// Terrain
    QOpenGLVertexArrayObject vao;
    QOpenGLBuffer            patchBuffer;

    QVector<float> positionData;

    int patchCount;

    ChunkMaterial* chunkMaterial;

    TexturePtr terrainData;
    SamplerPtr terrainSampler;

    float* mapData;
    /// ----------------------------------

    /// Chunk data
    QVector<GLuint> displaySubroutines;

    int chunkX, chunkY;

    float baseX, baseY;
    float chunkBaseX, chunkBaseY;

    /// ----------------------------------

    int horizToHMapSize(float position);
    float HMapSizeToHoriz(int position);

    void broadcastBorderHeight(QVector<QPair<int, float>> data, MapChunkBorder border);

    void initialize();
};

#endif // MAPCHUNK_H
