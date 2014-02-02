#ifndef MAPTILE_H
#define MAPTILE_H

#include "mapchunk.h"
#include "world.h"
#include "mapheaders.h"
#include "framebuffer.h"

#include <QOpenGLFramebufferObjectFormat>
#include <QOpenGLFramebufferObject>

class WaterTile;

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
    void drawWater(const float& distance, const QVector3D& camera);

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
};

#endif // MAPTILE_H