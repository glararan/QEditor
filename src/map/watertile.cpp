#include "watertile.h"

WaterTile::WaterTile(MapTile* parentTile)
: tile(parentTile)
, world(parentTile->world)
{
    for(int x = 0; x < CHUNKS; ++x)
    {
        for(int y = 0; y < CHUNKS; ++y)
            chunks[x][y] = new WaterChunk(world, x, y, tile->terrainSampler.data(), tile->coordX, tile->coordY);
    }
}

WaterTile::WaterTile(MapTile* parentTile, QFile& file)
: tile(parentTile)
, world(parentTile->world)
{
    for(int x = 0; x < CHUNKS; ++x)
    {
        for(int y = 0; y < CHUNKS; ++y)
            chunks[x][y] = new WaterChunk(world, x, y, tile->terrainSampler.data(), tile->coordX, tile->coordY, file);
    }
}

WaterTile::~WaterTile()
{
    for(int x = 0; x < CHUNKS; ++x)
    {
        for(int y = 0; y < CHUNKS; ++y)
        {
            if(chunks[x][y])
            {
                delete chunks[x][y];

                chunks[x][y] = NULL;
            }
        }
    }
}

WaterChunk* WaterTile::getChunk(int x, int y)
{
    if(x < CHUNKS && y < CHUNKS)
        return chunks[x][y];
    else
        return NULL;
}

bool WaterTile::hasData()
{
    for(int x = 0; x < CHUNKS; ++x)
    {
        for(int y = 0; y < CHUNKS; ++y)
        {
            if(chunks[x][y]->hasData())
                return true;
        }
    }

    return false;
}