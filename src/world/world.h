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
#include "model/imodelmanager.h"
#include "model/imodel.h"
#include "model/ipipeline.h"

class MapTile;
class MapChunk;
class WaterChunk;
class TextureManager;

struct ObjectBrush
{
    ObjectBrush()
    {
        impend     = 0.0f;
        scale      = 1.0f;
        rotation_x = 0.0f;
        rotation_y = 0.0f;
        rotation_z = 0.0f;
    }

    float impend;
    float scale;
    float rotation_x;
    float rotation_y;
    float rotation_z;
};

class MapTiles
{
private:
    MapTile* tile;

    MapTiles() : tile(NULL)
    {
    }

    friend class World;
};

struct MapObject;

class World
{
public:
    explicit World(const ProjectFileData& projectFile);

    void deleteMe();
    void initialize(QOpenGLContext* context, QSize fboSize);

    void update(float dt);
    void draw(QMatrix4x4 viewportMatrix, QVector2D viewportSize, QMatrix4x4 modelMatrix, float triangles, QVector2D mousePosition, bool drawBrush = false, bool drawNewModel = false);

    bool hasTile(int pX, int pY) const;

    void loadNewProjectMapTilesIntoMemory(bool** mapCoords, QSize size);

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
    void paintVertexShading(float x, float z, float flow, QColor& color);

    void highlightMapChunkAt(const QVector3D& position);
    void unHighlight();

    void updateNewModel(bool shiftDown, bool leftButtonPressed);

    void save();

    QOpenGLFunctions_4_2_Core* getGLFunctions()    { return GLfuncs; }
    Brush*                     getBrush()          { return brush; }
    Camera*                    getCamera()         { return camera; }
    ObjectBrush*               getObjectBrush()    { return objectBrush; }
    TextureManager*            getTextureManager() { return textureManager; }
    IModelManager*             getModelManager()   { return modelManager; }

    const ProjectFileData getProjectData() const { return projectData; }
    const int getAlphamapSize() const            { return alphaMapSize; }
    const float getTerrainMaximumHeight() const  { return terrainMaximumHeight; }
    const bool getTerrainMaximumState() const    { return terrainMaximumState; }

    MapTile* getTileAt(float x, float z) const;
    MapChunk*   getMapChunkAt(const QVector3D& position) const;
    WaterChunk* getWaterChunkAt(const QVector3D& position) const;

    void setFboSize(QSize size);
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

    QVector3D getWorldCoordinates();

    void test();

private:
    ~World();

    float time;

    Camera*         camera;
    Brush*          brush;
    ObjectBrush*    objectBrush;
    TextureManager* textureManager;
    IModelManager*  modelManager;
    MapChunk*       highlightChunk;

    bool tileLoaded(int x, int y) const;

    MapTile* loadTile(int x, int y, bool fileExists = true);

    MapTiles mapTiles[TILES][TILES];

    IModel* possibleModel;

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

    // worldCoordinates
    void loadWorldCoordinates(QMatrix4x4 viewportMatrix, QVector2D viewportSize, QMatrix4x4 model, QMatrix4x4 view, QMatrix4x4 projection, QVector2D mousePosition);
    QVector3D worldCoordinates;
};

#endif // WORLD_H