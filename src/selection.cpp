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
