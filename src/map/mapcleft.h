#ifndef MAPCLEFT_H
#define MAPCLEFT_H

#include "mapchunk.h"
#include "maptile.h"
#include "mesh.h"

class MapCleft
{
public:
    MapCleft(World* mWorld, MapTile* tile, MapChunk::Border orient, MapChunk* prevChunk, MapChunk* nextChunk);
    ~MapCleft();

    bool isInVisibleRange(const float& distance, const QVector3D& camera) const;

    void draw(QOpenGLShaderProgram* shader);

private:
    World* world;

    MapChunk* previous;
    MapChunk* next;

    MapChunk::Border orientation;

    // Terrain
    Mesh mesh;

    Material* chunkMaterial;

    TexturePtr terrainData;
    SamplerPtr terrainSampler;

    float* cleftData;

    QVector<GLuint> displaySubroutines;

    //
    void initialize();

    const GLuint& getDisplaySubroutines() const { return displaySubroutines[world->displayMode()]; }

    friend class MapChunk;
    friend class MapTile;
};

#endif // MAPCLEFT_H
