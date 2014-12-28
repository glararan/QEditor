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
