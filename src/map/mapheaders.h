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

#ifndef MAPHEADERS_H
#define MAPHEADERS_H

#include "globalheader.h" // quint32, guaranteed 32-bit on all platforms

class QDataStream;

/// Static const data
static const int TILES  = 64;
static const int CHUNKS = 4;

static const int TILES_TOTAL  = TILES * TILES;
static const int CHUNKS_TOTAL = CHUNKS * CHUNKS;

static const float TILESIZE  = 533.33333f;
static const float CHUNKSIZE = TILESIZE / CHUNKS;
static const float UNITSIZE  = CHUNKSIZE / 8.0f;

static const double CHUNK_DIAMETER = sqrt(pow(CHUNKSIZE, 2) + pow(CHUNKSIZE, 2));

static const float MAP_DRAW_DISTANCE = 1000.0f;

static const int TILE_WIDTH  = 1024;
static const int TILE_HEIGHT = 1024;

static const int CHUNK_WIDTH  = TILE_WIDTH / CHUNKS;
static const int CHUNK_HEIGHT = TILE_HEIGHT / CHUNKS;

static const int CHUNK_ARRAY_SIZE    = TILE_WIDTH / CHUNKS * TILE_HEIGHT / CHUNKS * sizeof(float);
static const int CHUNK_ARRAY_UC_SIZE = CHUNK_ARRAY_SIZE / sizeof(float); // Unsigned char array size

static const int MAX_TEXTURES = 4;
static const int ALPHAMAPS    = MAX_TEXTURES - 1;

static const int MAX_TEXTURE_SIZE = 1024;

enum ShaderUnits
{
    Heightmap     = 0,
    Texture1      = 1,
    Texture2      = 2,
    Texture3      = 3,
    Texture4      = 4,
    Alphamap1     = 5,
    //Alphamap1     = 4,
    //Texture4      = 5,
    Alphamap2     = 6,
    Alphamap3     = 7,
    VertexShading = 8
};

/// Structures
enum TextureScaleOption
{
    Multiply    = 0, // Multiply far and near
    Divide      = 1, // Divide far and near
    MultiplyFar = 2, // Multiply far and divide near
    DivideFar   = 3  // Divide far and multiply near
};

struct MCVT
{
    float height[CHUNK_ARRAY_UC_SIZE]; // chunk_array_size

    quint8 alphaMaps[ALPHAMAPS][CHUNK_ARRAY_UC_SIZE]; // unsigned char
    quint8 vertexShading[CHUNK_ARRAY_SIZE];

    QString textures[MAX_TEXTURES];

    qreal textureScaleFar[MAX_TEXTURES];
    qreal textureScaleNear[MAX_TEXTURES];

    TextureScaleOption textureScale[MAX_TEXTURES];

    bool automaticTexture[MAX_TEXTURES - 1];

    float automaticTextureStart[MAX_TEXTURES - 1];
    float automaticTextureEnd[MAX_TEXTURES - 1];
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

    MCVT* terrainOffset;
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

struct MapGenerationData
{
    int seed;
    int perlinNoiseMultiple;
    int erodeSmoothenLevel;

    float perlinNoiseLevel;
    float perlinNoiseHeight;
    float perturbFrequency;
    float perturbD;
    float erode;
};

struct BasicSettingsData
{
    bool setTerrainHeight;
    bool setWaterHeight;
    bool setWaterDraw;

    float terrainHeight;
    float waterHeight;
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
inline QDataStream& operator<<(QDataStream& dataStream, const MapHeader& mapHeader) // Save
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

        for(int j = 0; j < CHUNK_ARRAY_UC_SIZE; ++j) // chunk_array_size
            dataStream << mapHeader.mcin->entries[i].mcnk->terrainOffset->height[j];

        // Alphamaps
        for(int j = 0; j < ALPHAMAPS; ++j)
        {
            for(int k = 0; k < CHUNK_ARRAY_UC_SIZE; ++k)
                dataStream << mapHeader.mcin->entries[i].mcnk->terrainOffset->alphaMaps[j][k];
        }

        // Vertex Shading
        for(int j = 0; j < CHUNK_ARRAY_SIZE; ++j)
            dataStream << mapHeader.mcin->entries[i].mcnk->terrainOffset->vertexShading[j];

        // Textures
        for(int j = 0; j < MAX_TEXTURES; ++j)
            dataStream << mapHeader.mcin->entries[i].mcnk->terrainOffset->textures[j];

        // Texture scale
        for(int j = 0; j < MAX_TEXTURES; ++j)
        {
            dataStream << mapHeader.mcin->entries[i].mcnk->terrainOffset->textureScale[j]
                       << mapHeader.mcin->entries[i].mcnk->terrainOffset->textureScaleFar[j]
                       << mapHeader.mcin->entries[i].mcnk->terrainOffset->textureScaleNear[j];
        }

        // Automatic texture
        for(int j = 0; j < MAX_TEXTURES - 1; ++j)
        {
            dataStream << mapHeader.mcin->entries[i].mcnk->terrainOffset->automaticTexture[j]
                       << mapHeader.mcin->entries[i].mcnk->terrainOffset->automaticTextureStart[j]
                       << mapHeader.mcin->entries[i].mcnk->terrainOffset->automaticTextureEnd[j];
        }
    }

    return dataStream;
}

inline QDataStream& operator>>(QDataStream& dataStream, MapHeader& mapHeader) // Load
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

        mapHeader.mcin->entries[i].mcnk->terrainOffset = new MCVT;

        for(int j = 0; j < CHUNK_ARRAY_UC_SIZE; ++j) // chunk_array_size
            dataStream >> mapHeader.mcin->entries[i].mcnk->terrainOffset->height[j];

        // Alphamaps
        for(int j = 0; j < ALPHAMAPS; ++j)
        {
            for(int k = 0; k < CHUNK_ARRAY_UC_SIZE; ++k)
                dataStream >> mapHeader.mcin->entries[i].mcnk->terrainOffset->alphaMaps[j][k];
        }

        // Vertex Shading
        for(int j = 0; j < CHUNK_ARRAY_SIZE; ++j)
            dataStream >> mapHeader.mcin->entries[i].mcnk->terrainOffset->vertexShading[j];

        // Textures
        for(int j = 0; j < MAX_TEXTURES; ++j)
            dataStream >> mapHeader.mcin->entries[i].mcnk->terrainOffset->textures[j];

        // Texture scale
        for(int j = 0; j < MAX_TEXTURES; ++j)
        {
            int option = 0;

            dataStream >> option
                       >> mapHeader.mcin->entries[i].mcnk->terrainOffset->textureScaleFar[j]
                       >> mapHeader.mcin->entries[i].mcnk->terrainOffset->textureScaleNear[j];

            mapHeader.mcin->entries[i].mcnk->terrainOffset->textureScale[j] = (TextureScaleOption)option;
        }

        // Automatic texture
        for(int j = 0; j < MAX_TEXTURES - 1; ++j)
        {
            dataStream >> mapHeader.mcin->entries[i].mcnk->terrainOffset->automaticTexture[j]
                       >> mapHeader.mcin->entries[i].mcnk->terrainOffset->automaticTextureStart[j]
                       >> mapHeader.mcin->entries[i].mcnk->terrainOffset->automaticTextureEnd[j];
        }
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
