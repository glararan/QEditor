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