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

#ifndef MAPTILE_H
#define MAPTILE_H

#include "mapchunk.h"
#include "world.h"
#include "mapheaders.h"
#include "model/modelmanager.h"
#include "model/model.h"

#include <QOpenGLFramebufferObjectFormat>
#include <QOpenGLFramebufferObject>

class WaterTile;
class MapCleft;
class MapObject;

class MapTile
{
public:
    MapTile(World* mWorld, const QString& mapFile, int x, int y);
    MapTile(World* mWorld, int x, int y, const QString& mapFile);
    ~MapTile();

    int coordX, coordY;

    MapChunk* getChunk(int x, int y);

    const MapHeader& getHeader() const { return tileHeader; }
    WaterTile* getWater() const        { return waterTile; }

    void draw(const float& distance, const QVector3D& camera);
    void drawObjects(const float& distance, const QVector3D& camera, QMatrix4x4 viewMatrix, QMatrix4x4 projectionMatrix);
    void drawWater(const float& distance, const QVector3D& camera, QOpenGLFramebufferObject* refraction);

    void update(qreal time);

    void insertModel(MapObject* object);
    void deleteModel(float x, float z);
    void updateModelHeight();

    void generateMap(MapGenerationData& data);

    void importHeightmap(QString path, float scale);
    void exportHeightmap(QString path, float scale);

    bool isTile(int pX, int pY);

    void saveTile();

    void test();

private:
    QString fileName;

    MapChunk* mapChunks[CHUNKS][CHUNKS];

    MapCleft* mapCleftHorizontal[CHUNKS][(CHUNKS) - 1];
    MapCleft* mapCleftVertical[CHUNKS - 1][CHUNKS];

    World* world;

    WaterTile* waterTile;

    SamplerPtr terrainSampler;

    MapHeader tileHeader;

    friend class WaterTile;
    friend class MapChunk;
    friend class MapCleft;

    //
    MapChunk* getChunkAt(float x, float z);

    float getHeight(float x, float z);

    QVector<MapObject*> objects;
};

#endif // MAPTILE_H
