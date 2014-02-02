#ifndef WATERCHUNK_H
#define WATERCHUNK_H

#include "material.h"
#include "world.h"

#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QFile>

class WaterChunk
{
public:
    WaterChunk(World* mWorld, int x, int y, Sampler* sampler, int tileX, int tileY);
    WaterChunk(World* mWorld, int x, int y, Sampler* sampler, int tileX, int tileY, QFile& file);
    ~WaterChunk();

    void draw(GLuint reflectionTexture, GLuint depthTexture);
    void updateData();

    bool hasData() const { return data; }

    const int chunkIndex() const;

    struct BorderHeights
    {
        float top, right, bottom, left;

        bool topStatus, rightStatus, bottomStatus, leftStatus;
    };

    const QVector2D getBases() const       { return QVector2D(baseX, baseY); }
    const BorderHeights getHeights() const { return heights; }

    QOpenGLShaderProgramPtr getShader() const { return chunkMaterial->shader(); }

    float getHeight() const;
    float getHeight(int x, int y) const;

    void setHeight(float height);
    void setHeight(int x, int y, float height);
    void setHeights(BorderHeights borderHeights);

    void setData(bool datas) { data = datas; }

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

    BorderHeights heights;

    bool data;

    /// Chunk data
    int chunkX, chunkY;

    float baseX, baseY;
    float chunkBaseX, chunkBaseY;

    //
    void initialize();

    void setReflectionTexture(GLuint reflectionTexture, GLuint depthTexture);
};

#endif // WATERCHUNK_H