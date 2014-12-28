#include "selection.h"

#include "mapchunk.h"
#include "mapobject.h"

Selection::Selection()
: type(Types::None)
, name("None")
{
}

Selection::Selection(MapChunk* chunk)
: type(Types::MapChunkType)
, name(QString("MapChunk: [%1, %2]").arg(chunk->chunkIndex() / CHUNKS).arg(chunk->chunkIndex() % CHUNKS))
{
    data.mapChunk = chunk;
}

Selection::Selection(MapObject* object)
: type(Types::MapObjectType)
, name(QString("MapObject: ..."))
{
    data.mapObject = object;
}

Selection::~Selection()
{
}

SelectionManager::SelectionManager()
{
    items.append(new Selection());

    nextSelection = 1;
}

SelectionManager::~SelectionManager()
{
    for(int i = 0; i < items.count(); ++i)
    {
        if(items.at(i))
        {
            delete items[i];

            items[i] = NULL;
        }
    }
}

unsigned int SelectionManager::add(MapChunk* chunk)
{
    items.append(new Selection(chunk));

    return nextSelection++; // same effect with ++nextSelection?
}

unsigned int SelectionManager::add(MapObject* object)
{
    items.append(new Selection(object));

    return nextSelection++; // same effect with ++nextSelection?
}

void SelectionManager::remove(const unsigned int ref)
{
    if(items.at(ref))
    {
        // todo: if world has selection then reset

        delete items[ref];

        items[ref] = NULL;
    }
}
