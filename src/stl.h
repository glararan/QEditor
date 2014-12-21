#ifndef STLEXPORT_H
#define STLEXPORT_H

#include "mapheaders.h"

#include <QString>
#include <QVector3D>
#include <QTextStream>

// Cube normals
#define NORMALS_TOP    QVector3D( 0,  1,  0)
#define NORMALS_BOTTOM QVector3D( 0, -1,  0)
#define NORMALS_LEFT   QVector3D(-1,  0,  0)
#define NORMALS_RIGHT  QVector3D( 1,  0,  0)
#define NORMALS_NORTH  QVector3D( 0,  0,  1)
#define NORMALS_SOUTH  QVector3D( 0,  0, -1)

class MapTile;
class Face;
class Vertex;

class STL
{
public:
    enum Facets
    {
        South  = 0,
        Left   = 1,
        Top    = 2,
        Right  = 3,
        Bottom = 4,
        North  = 5
    };

    enum Resolution
    {
        Low,
        High
    };

    STL(MapTile* tile = NULL, Resolution resolution = Low);
    ~STL();

    void exportIt(QString file, float scale = 1.0);
    void createData();

    void setMapTile(MapTile* tile);
    void setResolution(Resolution resolution);

    const MapTile*   getMapTile() const    { return mapTile; }
    const Resolution getResolution() const { return stlResolution; }

private:
    MapTile* mapTile;

    Resolution stlResolution;

    QVector3D height[TILE_WIDTH][TILE_HEIGHT];
    QVector3D dataSouth[TILE_WIDTH][2];
    QVector3D dataNorth[TILE_WIDTH][2];
    QVector3D dataLeft[TILE_WIDTH][2];
    QVector3D dataRight[TILE_WIDTH][2];
    QVector3D dataBottom[2][2];

    QVector<Face*> faces;

    Face* getFace(Facets facet);
};

class Face
{
public:
    Face(STL::Facets face);
    ~Face();

    void writeToFile(QTextStream* stream, float scale);

    void createVertices();

    void setInputData(QVector<QVector3D>& datas);
    void setResolution(STL::Resolution resolution);

    const STL::Resolution getResolution() const { return facetResolution; }
    const STL::Facets     getFacet() const      { return facet; }

private:
    QVector3D data[TILE_WIDTH][TILE_HEIGHT];

    QVector<Vertex> vertexs;

    STL::Resolution facetResolution;
    STL::Facets facet;
};

class Vertex
{
public:
    Vertex();
    Vertex(QVector3D vertex_1, QVector3D vertex_2, QVector3D vertex_3);
    ~Vertex();

    void scale(const float value);

    const QVector3D getVertex(const int i) const;

private:
    QVector3D vertex1, vertex2, vertex3;
};

#endif // STLEXPORT_H
