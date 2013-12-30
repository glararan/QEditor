#ifndef WORLD_H
#define WORLD_H

#include <QObject>
#include <QOpenGLFunctions_4_2_Core>
#include <QStringList>

#include "mapheaders.h"
#include "globalheader.h"
#include "camera.h"
#include "brush.h"
#include "texturemanager.h"

class MapTile;
class MapChunk;
class TextureManager;

class MapTiles
{
private:
    MapTile* tile;

    MapTiles() : tile(NULL)
    {
    }

    friend class World;
};

class World
{
public:
    explicit World(const ProjectFileData& projectFile);

    void deleteMe();
    void initialize(QOpenGLContext* context);

    void update(float dt);
    void draw(QMatrix4x4 modelMatrix, float triangles, QVector2D mousePosition, bool drawBrush = false);

    bool hasTile(int pX, int pY) const;

    void loadNewProjectMapTilesIntoMemory(bool** mapCoords);

    enum eDisplayMode
    {
        SimpleWireFrame = 0,
        WorldHeight,
        WorldNormals,
        Grass,
        GrassAndRocks,
        GrassRocksAndSnow,
        LightingFactors,
        TexturedAndLit,
        WorldTexturedWireframed,
        Hidden,
        DisplayModeCount
    };

    void setDisplayMode(int displayMode);
    eDisplayMode displayMode() const { return eDisplay; }

    QString displayName(int index) const { return eDisplayNames.at(index); }

    // Sun position
    void setSunAngle(float sunAngle) { sunTheta = sunAngle; qDebug() << sunTheta; }
    float sunAngle() const           { return sunTheta; }

    void changeTerrain(float x, float z, float change);
    void flattenTerrain(float x, float z, float y, float change);
    void blurTerrain(float x, float z, float change);

    void paintTerrain(float x, float z, float flow);

    void highlightMapChunkAt(const QVector3D& position);
    void unHighlight();

    void save();

    QOpenGLFunctions_4_2_Core* getGLFunctions()    { return GLfuncs; }
    Brush*                     getBrush()          { return brush; }
    Camera*                    getCamera()         { return camera; }
    TextureManager*            getTextureManager() { return textureManager; }

    const ProjectFileData getProjectData() const { return projectData; }
    const int getAlphamapSize() const            { return alphaMapSize; }
    const float getTerrainMaximumHeight() const  { return terrainMaximumHeight; }
    const bool getTerrainMaximumState() const    { return terrainMaximumState; }

    MapChunk* getMapChunkAt(const QVector3D& position) const;

    void setCamera(Camera* cam);
    void setProjectData(ProjectFileData& data);

    void setTerrainMaximumHeight(float value);
    void setTerrainMaximumState(bool state);

    void setChunkShaderUniform(const char* name, const QVector2D& value);
    void setChunkShaderUniform(const char* name, const QVector4D& value);
    void setChunkShaderUniform(const char* name, const QMatrix4x4& value);
    void setChunkShaderUniform(const char* name, float value);
    void setChunkShaderUniform(const char* name, int value);

    MaterialPtr material;

    void test();

private:
    ~World();

    float time;

    Camera*         camera;
    Brush*          brush;
    TextureManager* textureManager;
    MapChunk*       highlightChunk;

    bool tileLoaded(int x, int y) const;

    MapTile* loadTile(int x, int y, bool fileExists = true);

    MapTiles mapTiles[TILES][TILES];

    float terrainMaximumHeight;
    bool  terrainMaximumState;

    // Angle of sun. 0 is directly overhead, 90 to the East, -90 to the West
    float sunTheta;

    int alphaMapSize;

    eDisplayMode    eDisplay;
    QStringList     eDisplayNames;

    QOpenGLFunctions_4_2_Core* GLfuncs;

    ProjectFileData projectData;

    //
    void createNeighbours();
};

#endif // WORLD_H