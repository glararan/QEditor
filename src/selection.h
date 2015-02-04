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

#ifndef SELECTION_H
#define SELECTION_H

#include <QString>
#include <QVector>

class MapChunk;
class MapObject;

class Selection
{
public:
    explicit Selection(MapChunk* chunk);
    explicit Selection(MapObject* object);
    Selection();
    ~Selection();

    union Data
    {
        MapChunk* mapChunk;
        MapObject* mapObject;
    };

    enum Types
    {
        None,
        MapObjectType,
        MapChunkType
    };

    const QString& getName() const { return name; }
    const Types getType() const    { return type; }
    const Data getData() const     { return data; }

private:
    QString name;

    Types type;

    Data data;
};

class SelectionManager
{
public:
    SelectionManager();
    ~SelectionManager();

    unsigned int add(MapChunk* chunk);
    unsigned int add(MapObject* object);

    void remove(const unsigned int ref);

    Selection* getSelection(unsigned int ref) const { return items.at(ref); }

private:
    QVector<Selection*> items;

    unsigned int nextSelection;
};

#endif // SELECTION_H
