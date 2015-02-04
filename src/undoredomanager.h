#ifndef UNDOREDOMANAGER_H
#define UNDOREDOMANAGER_H

#include <QUndoStack>
#include <QVector3D>
#include <QVector2D>
#include <QVector>

class MapObject;
class World;
class Model;

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
    ModifyTerrainCommand(QVector<QPair<QVector2D, float>> dataBefore, QVector<QPair<QVector2D, float>> dataAfter, World* mWorld, QUndoCommand* parent = 0);

    void undo();
    void redo();

private:
    QVector<QPair<QVector2D, float>> dataBeforeList, dataAfterList;

    World* world;
};

class UndoRedoManager : public QUndoStack
{
public:
    explicit UndoRedoManager(QObject* parent = 0);
    ~UndoRedoManager();
};

#endif // UNDOREDOMANAGER_H
