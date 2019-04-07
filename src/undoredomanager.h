#ifndef UNDOREDOMANAGER_H
#define UNDOREDOMANAGER_H

#include <QUndoStack>
#include <QVector3D>
#include <QVector2D>
#include <QVector>

class MapObject;
class World;
class Model;

/// Structures
struct TerrainUndoData
{
    int index; // mapData index

    float value;

    QPoint position; // X_Y texcoords

    TerrainUndoData(int mapDataIndex, float val, QPoint textureCoords)
    {
        index    = mapDataIndex;
        value    = val;
        position = textureCoords;
    }

    TerrainUndoData()
    {
        index    = 0;
        value    = 0.0f;
        position = QPoint();
    }
};

struct TextureUndoData
{
    int index; // alphaMapsData[layer][index]

    QVector<float> values;

    QPoint position; // X_Y texcoords

    TextureUndoData(int alphaMapIndex, QVector<float> vals, QPoint textureCoords)
    {
        index    = alphaMapIndex;
        values   = vals;
        position = textureCoords;
    }

    TextureUndoData()
    {
        index    = 0;
        values   = QVector<float>();
        position = QPoint();
    }
};

struct VertexUndoData
{
    int index;

    unsigned char r, g, b, a;

    QPoint position; // X_Y texcoords

    VertexUndoData(int vertexIndex, unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha, QPoint textureCoords)
    {
        index    = vertexIndex;
        r        = red;
        g        = green;
        b        = blue;
        a        = alpha;
        position = textureCoords;
    }

    VertexUndoData()
    {
        index    = 0;
        r        = 0;
        g        = 0;
        b        = 0;
        a        = 0;
        position = QPoint();
    }
};

/// ----------

class MoveMapObjectCommand : public QUndoCommand
{
public:
    MoveMapObjectCommand(MapObject* mapObject, const QVector3D& oldPos, QUndoCommand* parent = 0);

    void undo();
    void redo();

    bool mergeWith(const QUndoCommand* other);

private:
    MapObject* object;

    QVector3D oldPosition;
    QVector3D newPosition;
};

class RotateMapObjectCommand : public QUndoCommand
{
public:
    RotateMapObjectCommand(MapObject* mapObject, const QVector3D& oldRot, QUndoCommand* parent = 0);

    void undo();
    void redo();

    bool mergeWith(const QUndoCommand* other);

private:
    MapObject* object;

    QVector3D oldRotation;
    QVector3D newRotation;
};

class ScaleMapObjectCommand : public QUndoCommand
{
public:
    ScaleMapObjectCommand(MapObject* mapObject, const QVector3D& oldSc, QUndoCommand* parent = 0);

    void undo();
    void redo();

    bool mergeWith(const QUndoCommand* other);

private:
    MapObject* object;

    QVector3D oldScale;
    QVector3D newScale;
};

class DeleteMapObjectCommand : public QUndoCommand
{
public:
    DeleteMapObjectCommand(MapObject* mapObject, World* mWorld, QUndoCommand* parent = 0);

    void undo();
    void redo();

private:
    QVector3D translation;
    QVector3D rotation;
    QVector3D scale;

    int objectIndex;

    MapObject* object;
    World*     world;
};

class AddMapObjectCommand : public QUndoCommand
{
public:
    AddMapObjectCommand(QVector3D translate, QVector3D rotate, QVector3D mScale, int objIndex, World* mWorld, bool selected = true, QUndoCommand* parent = 0);

    void undo();
    void redo();

private:
    QVector3D translation;
    QVector3D rotation;
    QVector3D scale;

    int objectIndex;

    bool select;

    MapObject* object;
    World*     world;
};

class SpawnDetailDoodadsCommand : public QUndoCommand
{
public:
    SpawnDetailDoodadsCommand(QVector<QVector<QPair<QString, QVector3D>>> resultSpawnData, World* mWorld, QUndoCommand* parent = 0);

    void undo();
    void redo();

private:
    QVector<QVector<QPair<QString, QVector3D>>> spawnData;
    QVector<MapObject*>                         objects;

    MapObject* object;
    World*     world;
};

class ModifyTerrainCommand : public QUndoCommand
{
public:
    ModifyTerrainCommand(QVector<QPair<QVector2D, TerrainUndoData>> dataBefore, QVector<QPair<QVector2D, TerrainUndoData>> dataAfter, World* mWorld, QUndoCommand* parent = 0);

    void undo();
    void redo();

private:
    QVector<QPair<QVector2D, TerrainUndoData>> dataBeforeList, dataAfterList;

    World* world;
};

class ModifyTexturesCommand : public QUndoCommand
{
public:
    ModifyTexturesCommand(QVector<QPair<QVector2D, TextureUndoData>> dataBefore, QVector<QPair<QVector2D, TextureUndoData>> dataAfter, World* mWorld, QUndoCommand* parent = 0);

    void undo();
    void redo();

private:
    QVector<QPair<QVector2D, TextureUndoData>> dataBeforeList, dataAfterList;

    World* world;
};

class ModifyVertexsCommand : public QUndoCommand
{
public:
    ModifyVertexsCommand(QVector<QPair<QVector2D, VertexUndoData>> dataBefore, QVector<QPair<QVector2D, VertexUndoData>> dataAfter, World* mWorld, bool vertexShading, QUndoCommand* parent = 0);

    void undo();
    void redo();

private:
    QVector<QPair<QVector2D, VertexUndoData>> dataBeforeList, dataAfterList;

    World* world;

    bool vertexLighting;
};

class UndoRedoManager : public QUndoStack
{
public:
    explicit UndoRedoManager(QObject* parent = 0);
    ~UndoRedoManager();
};

#endif // UNDOREDOMANAGER_H
