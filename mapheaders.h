#ifndef MAPHEADERS_H
#define MAPHEADERS_H

#include "mathhelper.h"

static const int TILES  = 64;
static const int CHUNKS = 4;

static const float TILESIZE  = 533.33333f;
static const float CHUNKSIZE = TILESIZE / CHUNKS;
static const float UNITSIZE  = CHUNKSIZE / (CHUNKS / 2);

static const double CHUNK_DIAMETER = sqrt(pow(CHUNKSIZE, 2) + pow(CHUNKSIZE, 2));

static const float MAP_DRAW_DISTANCE = 1000.0f;

static const int MAP_WIDTH  = 1024;
static const int MAP_HEIGHT = 1024;

static const int CHUNK_ARRAY_SIZE = MAP_WIDTH / CHUNKS * MAP_HEIGHT / CHUNKS;

#endif // MAPHEADERS_H
