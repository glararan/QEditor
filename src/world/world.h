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

#ifndef WORLD_H
#define WORLD_H

#include <QObject>
#include <QOpenGLFunctions_4_2_Core>
#include <QStringList>
#include <QHash>
#include <QVector2D>

#include "mapheaders.h"
#include "globalheader.h"
#include "camera.h"
#include "brush.h"
#include "texturemanager.h"
#include "framebuffer.h"
#include "skybox.h"
#include "selection.h"
#include "undoredomanager.h"

#include "model/modelmanager.h"
#include "model/model.h"
#include "model/pipeline.h"

class MapTile;
class MapChunk;
class MapObject;
class MapView;
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

class World
{
public:
    World(const ProjectFileData& projectFile, UndoRedoManager* undoManager);

    void deleteMe();
    void initialize(QOpenGLContext* context, QSize fboSize, bool** mapCoords = NULL);

    void update(float dt);
    void draw(MapView* mapView, QVector3D& terrain_pos, QMatrix4x4 modelMatrix, float triangles, QVector2D mousePosition, bool drawBrush = false, bool drawNewModel = false);
    void drawExplorerView(Camera* cam, QMatrix4x4 modelMatrix, float triangles);

    bool hasTile(int pX, int pY) const;

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
    void uniformTerrain(float x, float z, float height);

    void paintTerrain(float x, float z, float flow);
    void paintVertexShading(float x, float z, float flow, QColor& color);
    void paintVertexLighting(float x, float z, float flow, QColor& lightColor);

    bool trySelectMapObject(const QVector3D& position);
    void spawnMapObjectFromFavouriteList(const QVector3D& position);
    void addObject(MapObject* object, bool selected = false);
    void removeObject(float x, float z);
    void removeObject(MapObject* object);

    void highlightMapChunkAt(const QVector3D& position);
    void unHighlight();

    void updateNewModel(bool shiftDown, bool leftButtonPressed);

    void mapGenerationAccepted();
    void mapGenerationRejected();

    void heightmapWidgetAccepted();
    void heightmapWidgetRejected();

    void importHeightmap(QString path, float scale);
    void exportHeightmap(QString path, float scale);
    void exportSTL(QString path, float surface, bool scaleHeight, bool low, bool tile);

    void save();

    QOpenGLFunctions_4_2_Core* getGLFunctions()       { return GLfuncs; }
    Brush*                     getBrush()             { return brush; }
    Camera*                    getCamera()            { return camera; }
    ObjectBrush*               getObjectBrush()       { return objectBrush; }
    Skybox*                    getSkybox()            { return skybox; }
    TextureManager*            getTextureManager()    { return textureManager; }
    ModelManager*              getModelManager()      { return modelManager; }
    SelectionManager*          getSelectionManager()  { return selectionManager; }
    Selection*                 getCurrentSelection()  { return currentSelection; }
    UndoRedoManager*           getUndoRedoManager()   { return undoRedoManager; }
    QOpenGLShaderProgram*      getModelShader()       { return modelShader; }
    QOpenGLShaderProgram*      getTerrainShader()     { return terrainShader; }
    QOpenGLShaderProgram*      getCleftShader()       { return cleftShader; }
    QOpenGLShaderProgram*      getWaterShader()       { return waterShader; }
    QOpenGLShaderProgram*      getSkyboxShader()      { return skyboxShader; }
    QOpenGLShaderProgram*      getBoundingBoxShader() { return boundingBoxShader; }

    const ProjectFileData getProjectData() const          { return projectData; }
    const int             getAlphamapSize() const         { return alphaMapSize; }
    const float           getTerrainMaximumHeight() const { return terrainMaximumHeight; }
    const bool            getTerrainMaximumState() const  { return terrainMaximumState; }
    const float           getPaintMaximumAlpha() const    { return paintMaximumAlpha; }
    const bool            getPaintMaximumState() const    { return paintMaximumState; }

    void addModifiedTerrain(QPair<QVector2D, TerrainUndoData> pair) { if(!modifiedTerrain.contains(pair.first)) modifiedTerrain.insert(pair.first, pair.second); }
    void clearModifiedTerrain()                                     { modifiedTerrain.clear(); }
    QHash<QVector2D, TerrainUndoData> getModifiedTerrain()          { return modifiedTerrain; }

    void addModifiedTextures(QPair<QVector2D, TextureUndoData> pair) { if(!modifiedTextures.contains(pair.first)) modifiedTextures.insert(pair.first, pair.second); }
    void clearModifiedTextures()                                     { modifiedTextures.clear(); }
    QHash<QVector2D, TextureUndoData> getModifiedTextures()          { return modifiedTextures; }

    void addModifiedVertex(QPair<QVector2D, VertexUndoData> pair) { if(!modifiedVertex.contains(pair.first)) modifiedVertex.insert(pair.first, pair.second); }
    void clearModifiedVertex()                                    { modifiedVertex.clear(); }
    QHash<QVector2D, VertexUndoData> getModifiedVertexs()         { return modifiedVertex; }

    MapTile*    getTileAt(float x, float z) const;
    MapChunk*   getMapChunkAt(const QVector3D& position) const;
    WaterChunk* getWaterChunkAt(const QVector3D& position) const;

    void setFboSize(QSize size);
    void setCamera(Camera* cam);
    void setProjectData(ProjectFileData& data);
    void setMapGenerationData(MapGenerationData& data);
    void setBasicSettings(BasicSettingsData& data);

    void setTerrainMaximumHeight(float value);
    void setTerrainMaximumState(bool state);

    void setPaintMaximumAlpha(float value);
    void setPaintMaximumState(bool state);

    void setHeightmapScale(float scale);

    void setVertexShadingSwitch(bool state);
    void setSkyboxSwitch(bool state);

    QVector3D getWorldCoordinates() const;

    void test();

private:
    ~World();

    float time;

    Camera*           camera;
    Brush*            brush;
    ObjectBrush*      objectBrush;
    Skybox*           skybox;
    TextureManager*   textureManager;
    ModelManager*     modelManager;
    MapChunk*         highlightChunk;
    SelectionManager* selectionManager;
    Selection*        currentSelection;
    UndoRedoManager*  undoRedoManager;

    QOpenGLFramebufferObject* fbo;
    QOpenGLFramebufferObject* reflection_fbo;
    QOpenGLFramebufferObject* refraction_fbo;

    // reflection
    QMatrix4x4 reflectionView;

    //
    QSize viewportSize;

    bool tileLoaded(int x, int y) const;

    MapTile* loadTile(int x, int y, bool fileExists = true);

    MapTiles mapTiles[TILES][TILES];

    Model* possibleModel;

    float terrainMaximumHeight;
    bool  terrainMaximumState;

    QHash<QVector2D, TerrainUndoData> modifiedTerrain;
    QHash<QVector2D, TextureUndoData> modifiedTextures;
    QHash<QVector2D, VertexUndoData>  modifiedVertex;

    float paintMaximumAlpha;
    bool  paintMaximumState;

    bool shadingOff, skyboxOff;

    // Angle of sun. 0 is directly overhead, 90 to the East, -90 to the West
    float sunTheta;

    int alphaMapSize;

    eDisplayMode eDisplay;
    QStringList  eDisplayNames;

    QOpenGLFunctions_4_2_Core* GLfuncs;

    QOpenGLShaderProgram* modelShader;
    QOpenGLShaderProgram* terrainShader;
    QOpenGLShaderProgram* cleftShader;
    QOpenGLShaderProgram* waterShader;
    QOpenGLShaderProgram* skyboxShader;
    QOpenGLShaderProgram* boundingBoxShader;

    ProjectFileData projectData;

    //
    void createNeighbours();
    void updateNeighboursHeightmapData();

    void drawSkybox(QMatrix4x4& modelMatrix);
    void drawReflection();
    void drawRefraction();

    // worldCoordinates
    QVector3D worldCoordinates;
};

#endif // WORLD_H
