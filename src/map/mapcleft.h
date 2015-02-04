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
