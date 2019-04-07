#include "undoredomanager.h"

#include "mapobject.h"
#include "mapchunk.h"
#include "world.h"
#include "model.h"

UndoRedoManager::UndoRedoManager(QObject* parent)
: QUndoStack(parent)
{
    setUndoLimit(10);
}

UndoRedoManager::~UndoRedoManager()
{

}

MoveMapObjectCommand::MoveMapObjectCommand(MapObject* mapObject, const QVector3D& oldPos, QUndoCommand* parent)
: QUndoCommand(parent)
, object(mapObject)
, oldPosition(oldPos)
{
    newPosition = object->getTranslate();
}

void MoveMapObjectCommand::undo()
{
    object->setTranslate(oldPosition, true);
}

void MoveMapObjectCommand::redo()
{
    object->setTranslate(newPosition, true);
}

bool MoveMapObjectCommand::mergeWith(const QUndoCommand* other)
{
    const MoveMapObjectCommand* moveCommand = static_cast<const MoveMapObjectCommand*>(other);

    MapObject* item = moveCommand->object;

    if(item != object)
        return false;

    newPosition = item->getTranslate();

    return true;
}

RotateMapObjectCommand::RotateMapObjectCommand(MapObject* mapObject, const QVector3D& oldRot, QUndoCommand* parent)
: QUndoCommand(parent)
, object(mapObject)
, oldRotation(oldRot)
{
    newRotation = object->getRotation();
}

void RotateMapObjectCommand::undo()
{
    object->setRotation(oldRotation, true);
}

void RotateMapObjectCommand::redo()
{
    object->setRotation(newRotation, true);
}

bool RotateMapObjectCommand::mergeWith(const QUndoCommand* other)
{
    const RotateMapObjectCommand* rotateCommand = static_cast<const RotateMapObjectCommand*>(other);

    MapObject* item = rotateCommand->object;

    if(item != object)
        return false;

    newRotation = item->getRotation();

    return true;
}

ScaleMapObjectCommand::ScaleMapObjectCommand(MapObject* mapObject, const QVector3D& oldSc, QUndoCommand* parent)
: QUndoCommand(parent)
, object(mapObject)
, oldScale(oldSc)
{
    newScale = object->getScale();
}

void ScaleMapObjectCommand::undo()
{
    object->setScale(oldScale, true);
}

void ScaleMapObjectCommand::redo()
{
    object->setScale(newScale, true);
}

bool ScaleMapObjectCommand::mergeWith(const QUndoCommand* other)
{
    const ScaleMapObjectCommand* scaleCommand = static_cast<const ScaleMapObjectCommand*>(other);

    MapObject* item = scaleCommand->object;

    if(item != object)
        return false;

    newScale = item->getScale();

    return true;
}

DeleteMapObjectCommand::DeleteMapObjectCommand(MapObject* mapObject, World* mWorld, QUndoCommand* parent)
: QUndoCommand(parent)
, object(mapObject)
, world(mWorld)
{
    translation = object->getTranslate();
    rotation    = object->getRotation();
    scale       = object->getScale();
    objectIndex = world->getModelManager()->getIndex(object->getModel()->getModelInterface()->getFilePath() + object->getModel()->getModelInterface()->getFileName());
}

void DeleteMapObjectCommand::undo()
{
    /*if(object)
        delete object; // crash
*/
    object = new MapObject(world->getModelManager()->getModel(objectIndex));
    object->setTranslate(translation);
    object->setRotation(rotation);
    object->setScale(scale);
    object->updateBoundingBox();

    world->addObject(object, true);
}

void DeleteMapObjectCommand::redo()
{
    translation = object->getTranslate();
    rotation    = object->getRotation();
    scale       = object->getScale();
    objectIndex = world->getModelManager()->getIndex(object->getModel()->getModelInterface()->getFilePath() + object->getModel()->getModelInterface()->getFileName());

    world->removeObject(object);
}

AddMapObjectCommand::AddMapObjectCommand(QVector3D translate, QVector3D rotate, QVector3D mScale, int objIndex, World* mWorld, bool selected, QUndoCommand* parent)
: QUndoCommand(parent)
, translation(translate)
, rotation(rotate)
, scale(mScale)
, objectIndex(objIndex)
, object(NULL)
, select(selected)
, world(mWorld)
{
}

void AddMapObjectCommand::undo()
{
    translation = object->getTranslate();
    rotation    = object->getRotation();
    scale       = object->getScale();
    objectIndex = world->getModelManager()->getIndex(object->getModel()->getModelInterface()->getFilePath() + object->getModel()->getModelInterface()->getFileName());

    world->removeObject(object);
}

void AddMapObjectCommand::redo()
{
    /*if(object)
        delete object; // crash
*/

    object = new MapObject(world->getModelManager()->getModel(objectIndex));
    object->setTranslate(translation);
    object->setRotation(rotation);
    object->setScale(scale);
    object->updateBoundingBox();

    world->addObject(object, select);
}

SpawnDetailDoodadsCommand::SpawnDetailDoodadsCommand(QVector<QVector<QPair<QString, QVector3D>>> resultSpawnData, World* mWorld, QUndoCommand* parent)
: QUndoCommand(parent)
, spawnData(resultSpawnData)
, object(NULL)
, world(mWorld)
{
}

void SpawnDetailDoodadsCommand::undo()
{
    for(int i = 0; i < objects.count(); ++i)
        world->removeObject(objects[i]);
}

void SpawnDetailDoodadsCommand::redo()
{
    /*if(object)
        delete object; // crash
*/

    for(int i = 0; i < spawnData.count(); ++i)
    {
        QVector<QPair<QString, QVector3D>> resultData = spawnData[i];

        for(int j = 0; j < resultData.count(); ++j)
        {
            QPair<QString, QVector3D> pair = resultData[j];

            // todo rotation based on heightmap bounding boxes
            MapObject* object = new MapObject(world->getModelManager()->getModel(world->getModelManager()->getIndex(pair.first)));
            object->setTranslate(pair.second, false);
            object->setRotation(QVector3D(0.0f, 0.0f, 0.0f));
            object->setScale(QVector3D(1.0f, 1.0f, 1.0f));
            object->updateBoundingBox();

            objects.append(object);

            world->addObject(object);
        }
    }
}

ModifyTerrainCommand::ModifyTerrainCommand(QVector<QPair<QVector2D, TerrainUndoData>> dataBefore, QVector<QPair<QVector2D, TerrainUndoData>> dataAfter, World* mWorld, QUndoCommand* parent)
: QUndoCommand(parent)
, dataBeforeList(dataBefore)
, dataAfterList(dataAfter)
, world(mWorld)
{
}

void ModifyTerrainCommand::undo()
{
    for(int i = 0; i < dataBeforeList.count(); ++i)
    {
        QPair<QVector2D, TerrainUndoData> pair = dataBeforeList.at(i);

        MapChunk* chunk = world->getMapChunkAt(QVector3D(pair.first.x(), 0.0f, pair.first.y()));

        if(chunk)
            chunk->setHeight(pair.second.index, pair.second.value, pair.second.position);
    }
}

void ModifyTerrainCommand::redo()
{
    for(int i = 0; i < dataAfterList.count(); ++i)
    {
        QPair<QVector2D, TerrainUndoData> pair = dataAfterList.at(i);

        MapChunk* chunk = world->getMapChunkAt(QVector3D(pair.first.x(), 0.0f, pair.first.y()));

        if(chunk)
            chunk->setHeight(pair.second.index, pair.second.value, pair.second.position);
    }
}

ModifyTexturesCommand::ModifyTexturesCommand(QVector<QPair<QVector2D, TextureUndoData>> dataBefore, QVector<QPair<QVector2D, TextureUndoData>> dataAfter, World* mWorld, QUndoCommand* parent)
: QUndoCommand(parent)
, dataBeforeList(dataBefore)
, dataAfterList(dataAfter)
, world(mWorld)
{
}

void ModifyTexturesCommand::undo()
{
    for(int i = 0; i < dataBeforeList.count(); ++i)
    {
        QPair<QVector2D, TextureUndoData> pair = dataBeforeList.at(i);

        MapChunk* chunk = world->getMapChunkAt(QVector3D(pair.first.x(), 0.0f, pair.first.y()));

        if(chunk)
            chunk->setAlphas(pair.second.index, pair.second.values, pair.second.position);
    }
}

void ModifyTexturesCommand::redo()
{
    for(int i = 0; i < dataAfterList.count(); ++i)
    {
        QPair<QVector2D, TextureUndoData> pair = dataAfterList.at(i);

        MapChunk* chunk = world->getMapChunkAt(QVector3D(pair.first.x(), 0.0f, pair.first.y()));

        if(chunk)
            chunk->setAlphas(pair.second.index, pair.second.values, pair.second.position);
    }
}

ModifyVertexsCommand::ModifyVertexsCommand(QVector<QPair<QVector2D, VertexUndoData>> dataBefore, QVector<QPair<QVector2D, VertexUndoData>> dataAfter, World* mWorld, bool vertexShading, QUndoCommand* parent)
: QUndoCommand(parent)
, dataBeforeList(dataBefore)
, dataAfterList(dataAfter)
, world(mWorld)
, vertexLighting(!vertexShading)
{
}

void ModifyVertexsCommand::undo()
{
    for(int i = 0; i < dataBeforeList.count(); ++i)
    {
        QPair<QVector2D, VertexUndoData> pair = dataBeforeList.at(i);

        MapChunk* chunk = world->getMapChunkAt(QVector3D(pair.first.x(), 0.0f, pair.first.y()));

        if(chunk && vertexLighting)
            chunk->setVertexLighting(pair.second.index, QColor(MathHelper::toInt(pair.second.r), MathHelper::toInt(pair.second.g), MathHelper::toInt(pair.second.b), MathHelper::toInt(pair.second.a)), pair.second.position);
        else if(chunk && !vertexLighting)
            chunk->setVertexShading(pair.second.index, QColor(MathHelper::toInt(pair.second.r), MathHelper::toInt(pair.second.g), MathHelper::toInt(pair.second.b), MathHelper::toInt(pair.second.a)), pair.second.position);
    }
}

void ModifyVertexsCommand::redo()
{
    for(int i = 0; i < dataAfterList.count(); ++i)
    {
        QPair<QVector2D, VertexUndoData> pair = dataAfterList.at(i);

        MapChunk* chunk = world->getMapChunkAt(QVector3D(pair.first.x(), 0.0f, pair.first.y()));

        if(chunk && vertexLighting)
            chunk->setVertexLighting(pair.second.index, QColor(MathHelper::toInt(pair.second.r), MathHelper::toInt(pair.second.g), MathHelper::toInt(pair.second.b), MathHelper::toInt(pair.second.a)), pair.second.position);
        else if(chunk && !vertexLighting)
            chunk->setVertexShading(pair.second.index, QColor(MathHelper::toInt(pair.second.r), MathHelper::toInt(pair.second.g), MathHelper::toInt(pair.second.b), MathHelper::toInt(pair.second.a)), pair.second.position);
    }
}
