#ifndef WATERCHUNK_H
#define WATERCHUNK_H

#include "material.h"
#include "world.h"

#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>

class WaterChunk
{
public:
    WaterChunk(World* mWorld, int x, int y, Sampler* sampler, int tileX, int tileY, bool cache = false);
    ~WaterChunk();

    void draw(GLuint reflectionTexture);

    bool hasData();

    const int chunkIndex() const;

    const QVector2D getBases() const { return QVector2D(baseX, baseY); }

    QOpenGLShaderProgramPtr getShader() const { return chunkMaterial->shader(); }

    float getHeight() const;
    float getHeight(int x, int y) const;

    void setHeight(float height);
    void setHeight(int x, int y, float height);

    void save();

private:
    World* world;

    /// Water
    QOpenGLVertexArrayObject vao;
    QOpenGLBuffer            patchBuffer;

    QVector<float> positionData;

    int patchCount;

    ChunkMaterial* chunkMaterial;

    TexturePtr waterSurface;
    SamplerPtr waterSampler;

    float* waterData;

    /// Chunk data
    int chunkX, chunkY;

    float baseX, baseY;
    float chunkBaseX, chunkBaseY;

    //
    void setReflectionTexture(GLuint reflectionTexture);
};

#endif // WATERCHUNK_H