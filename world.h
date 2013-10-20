#ifndef WORLD_H
#define WORLD_H

#include <QObject>
#include <QOpenGLFunctions_4_2_Core>
#include <QStringList>

#include "mapheaders.h"
#include "camera.h"
#include "brush.h"

class MapTile;

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
    explicit World(const QString& map_name);
    ~World();

    void initialize(QOpenGLContext* context);

    void update(float dt);
    void draw(QMatrix4x4 modelMatrix, float triangles, QVector2D mousePosition, bool drawBrush = false);

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

    // Sun position
    void setSunAngle(float sunAngle) { sunTheta = sunAngle; qDebug() << sunTheta; }
    float sunAngle() const           { return sunTheta; }

    void changeTerrain(float x, float z, float change, int brush_type);
    void flattenTerrain(float x, float z, float y, float change, int brush_type);
    void blurTerrain(float x, float z, float change, int brush_type);

    void paintTerrain();

    void save();

    QOpenGLFunctions_4_2_Core* getGLFunctions() { return GLfuncs; }
    Brush*                     getBrush()       { return brush; }

    const GLuint& getDisplaySubroutines() const { return eDisplaySubroutines[eDisplay]; }

    void setCamera(Camera* cam);

    MaterialPtr material;

    void test();

private:
    float time;

    Camera* camera;

    Brush* brush;

    QString mapName;

    bool tileLoaded(int x, int y) const;

    MapTile* loadTile(int x, int y);

    MapTiles mapTiles[TILES][TILES];

    // Angle of sun. 0 is directly overhead, 90 to the East, -90 to the West
    float sunTheta;

    eDisplayMode    eDisplay;
    QStringList     eDisplayNames;
    QVector<GLuint> eDisplaySubroutines;

    QOpenGLFunctions_4_2_Core* GLfuncs;
};

#endif // WORLD_H