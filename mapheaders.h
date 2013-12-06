#ifndef MAPHEADERS_H
#define MAPHEADERS_H

#include "globalheader.h" // quint32, guaranteed 32-bit on all platforms

class QDataStream;

static const int TILES  = 64;
static const int CHUNKS = 4;

static const float TILESIZE  = 533.33333f;
static const float CHUNKSIZE = TILESIZE / CHUNKS;
static const float UNITSIZE  = CHUNKSIZE / ((float)CHUNKS / 2);

static const double CHUNK_DIAMETER = sqrt(pow(CHUNKSIZE, 2) + pow(CHUNKSIZE, 2));

static const float MAP_DRAW_DISTANCE = 1000.0f;

static const int MAP_WIDTH  = 1024;
static const int MAP_HEIGHT = 1024;

static const int CHUNK_ARRAY_SIZE = MAP_WIDTH / CHUNKS * MAP_HEIGHT / CHUNKS * sizeof(float);

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
/// Map header data streams
inline QDataStream& operator<<(QDataStream& dataStream, const MapHeader& mapHeader)
{
    dataStream << mapHeader.version;

    for(int i = 0; i < CHUNKS * CHUNKS; ++i)
    {
        dataStream << mapHeader.mcin->entries[i].size
                   << mapHeader.mcin->entries[i].mcnk->flags
                   << mapHeader.mcin->entries[i].mcnk->indexX
                   << mapHeader.mcin->entries[i].mcnk->indexY
                   << mapHeader.mcin->entries[i].mcnk->layers
                   << mapHeader.mcin->entries[i].mcnk->doodads
                   << mapHeader.mcin->entries[i].mcnk->areaID;

        for(int j = 0; j < CHUNK_ARRAY_SIZE; ++j)
            dataStream << mapHeader.mcin->entries[i].mcnk->heightOffset->height[j];
    }

    return dataStream;
}

inline QDataStream& operator>>(QDataStream& dataStream, MapHeader& mapHeader)
{
    dataStream >> mapHeader.version;

    mapHeader.mcin = new MCIN;

    for(int i = 0; i < CHUNKS * CHUNKS; ++i)
    {
        mapHeader.mcin->entries[i].mcnk = new MCNK;

        dataStream >> mapHeader.mcin->entries[i].size
                   >> mapHeader.mcin->entries[i].mcnk->flags
                   >> mapHeader.mcin->entries[i].mcnk->indexX
                   >> mapHeader.mcin->entries[i].mcnk->indexY
                   >> mapHeader.mcin->entries[i].mcnk->layers
                   >> mapHeader.mcin->entries[i].mcnk->doodads
                   >> mapHeader.mcin->entries[i].mcnk->areaID;

        mapHeader.mcin->entries[i].mcnk->heightOffset = new MCVT;

        for(int j = 0; j < CHUNK_ARRAY_SIZE; ++j)
            dataStream >> mapHeader.mcin->entries[i].mcnk->heightOffset->height[j];
    }

    return dataStream;
}

/// Project Data
inline QDataStream& operator<<(QDataStream& dataStream, const ProjectFileData& projectData)
{
    dataStream << projectData.version
               << projectData.projectFile
               << projectData.projectRootDir
               << projectData.projectName
               << projectData.mapName
               << projectData.mapsCount;

    for(int i = 0; i < TILES * TILES; ++i)
    {
        dataStream << projectData.maps[i].exists
                   << projectData.maps[i].x
                   << projectData.maps[i].y;
    }

    return dataStream;
}

inline QDataStream& operator>>(QDataStream& dataStream, ProjectFileData& projectData)
{
    dataStream >> projectData.version
               >> projectData.projectFile
               >> projectData.projectRootDir
               >> projectData.projectName
               >> projectData.mapName
               >> projectData.mapsCount;

    for(int i = 0; i < TILES * TILES; ++i)
    {
        dataStream >> projectData.maps[i].exists
                   >> projectData.maps[i].x
                   >> projectData.maps[i].y;
    }

    return dataStream;
}

#endif // MAPHEADERS_H
