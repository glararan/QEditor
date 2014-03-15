/*This file is part of QEditor.

QEditor is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

QEditor is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with QEditor.  If not, see <http://www.gnu.org/licenses/>.*/

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

    void draw(QOpenGLShaderProgram* shader, GLuint reflectionTexture, GLuint depthTexture);
    void updateData();

    bool hasData() const { return data; }

    const int chunkIndex() const;

    struct BorderHeights
    {
        float top, right, bottom, left;
    };

    const QVector2D getBases() const       { return QVector2D(baseX, baseY); }
    const BorderHeights getHeights() const { return heights; }

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
    IMesh Mesh;

    Material* chunkMaterial;

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