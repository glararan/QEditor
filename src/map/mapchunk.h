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
#include "mesh.h"

class MapChunk
{
public:
    MapChunk(World* mWorld, MapTile* tile, int x, int y);
    MapChunk(World* mWorld, MapTile* tile, QFile& file, int x, int y);
    ~MapChunk();

    bool isInVisibleRange(const float& distance, const QVector3D& camera) const;

    void draw(QOpenGLShaderProgram* shader);

    /// Terrain
    bool changeTerrain(float x , float z, float change);
    bool flattenTerrain(float x, float z, float y, float change);
    bool blurTerrain(float x   , float z, float change);
    bool uniformTerrain(float x, float z, float height);

    bool paintTerrain(float x       , float z, float flow, TexturePtr texture);
    bool paintVertexShading(float x , float z, float flow, QColor& color);
    bool paintVertexLighting(float x, float z, float flow, QColor& lightColor);

    void updateNeighboursHeightmapData();

    void generation(bool accepted);
    void heightmapSettings(bool accepted);

    /// Get
    const float getHeight(const float& x, const float& y) const;
    const float getHeight(const int& x, const int& y) const;

    const float getHeightFromWorld(float x, float z);
    const float getMapData(const int& index) const;

    const int chunkIndex() const;

    const int   getTextureScaleOption(int layer) const;
    const float getTextureScaleFar(int layer) const;
    const float getTextureScaleNear(int layer) const;

    const bool  getAutomaticTexture(int layer) const;
    const float getAutomaticTextureStart(int layer) const;
    const float getAutomaticTextureEnd(int layer) const;

    const QVector2D  getBases() const              { return QVector2D(baseX, baseY); }
    const GLuint&    getDisplaySubroutines() const { return displaySubroutines[world->displayMode()]; }
    const bool       getHighlight() const          { return highlight; }

    TexturePtr getTexture(int index) const
    {
        if(index >= MAX_TEXTURES)
            return TexturePtr(new Texture());

        return textures[index];
    }

    //
    void moveAlphaMap(int index, bool up);
    void deleteAlphaMap(int index);

    /// Set
    enum Border
    {
        Horizontal = 0,
        Vertical   = 1
    };

    void setBorder(Border border, const QVector<QPair<int, float>>& data);

    void setBottomNeighbour(MapChunk* chunk);
    void setLeftNeighbour(MapChunk* chunk);

    void setHighlight(bool on);
    void setSelected(bool on);

    void setHeightmap(float* data);
    void setGeneratedHeightmap(float* tileData);

    void setHeightmapScale(float scale);

    void setTextureScaleOption(int option, int layer);
    void setTextureScaleNear(double value, int layer);
    void setTextureScaleFar(double value, int layer);

    void setAutomaticTexture(bool enabled, int layer);
    void setAutomaticTextureStart(double value, int layer);
    void setAutomaticTextureEnd(double value, int layer);

    /// ...
    void save(MCNK* chunk);

    void test();

private:
    World* world;

    /// Terrain
    Mesh mesh;

    Material* chunkMaterial;

    TexturePtr terrainData;
    SamplerPtr terrainSampler;

    float* mapData;
    float* mapDataCache;

    /// Textures, Alphamaps, Vertex Shading and Texture scale
    TexturePtr textures[MAX_TEXTURES];
    TexturePtr depthTextures[MAX_TEXTURES];
    TexturePtr alphaMaps[ALPHAMAPS];

    TexturePtr textureArray;
    TexturePtr depthTextureArray;
    TexturePtr alphaArray;

    TexturePtr vertexShadingMap;
    TexturePtr vertexLightingMap;

    unsigned char* alphaMapsData[ALPHAMAPS];
    unsigned char* vertexShadingData;
    unsigned char* vertexLightingData;

    float textureScaleNear[MAX_TEXTURES];
    float textureScaleFar[MAX_TEXTURES];

    TextureScaleOption textureScaleOption[MAX_TEXTURES];

    bool automaticTexture[MAX_TEXTURES - 1];

    float automaticTextureStart[MAX_TEXTURES - 1];
    float automaticTextureEnd[MAX_TEXTURES - 1];

    /// Chunk data
    QVector<GLuint> displaySubroutines;

    int chunkX, chunkY;

    float baseX, baseY;
    float chunkBaseX, chunkBaseY;

    bool highlight;
    bool selected;
    bool mapGeneration;
    bool mapScale;

    // Neighbour
    MapChunk* bottomNeighbour;
    MapChunk* leftNeighbour;
    /// ----------------------------------

    int horizToHMapSize(float position);
    float HMapSizeToHoriz(int position);

    void initialize();

    friend class MapTile;
    friend class MapCleft;
};

#endif // MAPCHUNK_H
