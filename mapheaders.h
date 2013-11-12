#ifndef MAPHEADERS_H
#define MAPHEADERS_H

#include "globalheader.h" // quint32, guaranteed 32-bit on all platforms

class QDataStream;

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

struct MCVT
{
    float height[CHUNK_ARRAY_SIZE];
};

struct MH2O
{

};

// invidual MCNK chunks, row by row, from top-left
struct MCNK
{
    quint32 flags;
    quint32 indexX;
    quint32 indexY;
    quint32 layers; // max 4, 8, 16?
    quint32 doodads;
    quint32 areaID;

    MCVT* heightOffset;
    //MH2O* liquidOffset;
};

// Pointers to MCNK chunks and their sizes
struct MCIN
{
    struct MCINentry
    {
        MCNK* mcnk; // offset

        quint32 size; // the size of the MCNK chunk
    } entries[CHUNKS * CHUNKS];
};

// Contains offsets relative to MapHeader data in the file for specific chunks
struct MapHeader
{
    quint32 version;

    MCIN* mcin;
};

struct ProjectFileData
{
    quint32 version;

    QString projectFile;
    QString projectRootDir;
    QString projectName;
    QString mapName;

    quint32 mapsCount;

    struct Maps
    {
        quint8 exists; // no == 0, yes == 1

        quint32 x;
        quint32 y;
    } maps[TILES * TILES];
};

// Data streams operators
/// Map Header
QDataStream& operator<<(QDataStream& dataStream, const MapHeader& mapHeader);
QDataStream& operator>>(QDataStream& dataStream, MapHeader& mapHeader);

/// Project Data
QDataStream& operator<<(QDataStream& dataStream, const ProjectFileData& projectData);
QDataStream& operator>>(QDataStream& dataStream, ProjectFileData& projectData);

#endif // MAPHEADERS_H
