#ifndef STLEXPORT_H
#define STLEXPORT_H

#include "mapheaders.h"

#include <QString>
#include <QVector3D>

class MapTile;

class STLExport
{
public:
    STLExport();
    ~STLExport();

    void exportIt(QString file);
    void generateData();

    void setMapTile(MapTile* tile);

private:
    MapTile* mapTile;

    QVector3D data[TILE_WIDTH][TILE_HEIGHT];
};

#endif // STLEXPORT_H
