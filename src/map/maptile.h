#ifndef MAPTILE_H
#define MAPTILE_H

#include "mapchunk.h"
#include "world.h"
#include "mapheaders.h"
#include "framebuffer.h"
#include "model/imodelmanager.h"
#include "model/imodel.h"

#include <QOpenGLFramebufferObjectFormat>
#include <QOpenGLFramebufferObject>

class WaterTile;

struct MapObject
{
    MapObject()
    {
        model         = 0;
        height_offset = 0.0f;
    }

    ~MapObject()
    {
        delete model;
    }

    IModel* model;

    float height_offset;

    QVector3D translate;
    QVector3D rotation;
    QVector3D scale;
};

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
    void drawWater(const float& distance, const QVector3D& camera);

    void update(qreal time);

    void insertModel(MapObject* object);

    bool isTile(int pX, int pY);

    void setFboSize(QSize size);

    void saveTile();

    void test();

private:
    QString fileName;

    MapChunk* mapChunks[CHUNKS][CHUNKS];

    World* world;

    WaterTile* waterTile;

    Framebuffer* fbo;

    SamplerPtr terrainSampler;

    MapHeader tileHeader;

    friend class WaterTile;
    friend class MapChunk;

    //
    MapChunk* getChunkAt(float x, float z);

    float getHeight(float x, float z);

    QVector<MapObject*> objects;
};

#endif // MAPTILE_H