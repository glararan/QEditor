#include "stl.h"

#include "maptile.h"

#include <QFile>

STL::STL(MapTile* tile, Resolution resolution)
: mapTile(tile)
, stlResolution(resolution)
{
    faces.append(new Face(South));
    faces.append(new Face(Left));
    faces.append(new Face(Top));
    faces.append(new Face(Right));
    faces.append(new Face(Bottom));
    faces.append(new Face(North));

    for(int i = 0; i < faces.count(); ++i)
        faces[i]->setResolution(stlResolution);
}

STL::~STL()
{
    for(int i = 0; i < faces.count(); ++i)
        delete faces[i];
}

void STL::exportIt(QString file, float scale)
{
    if(mapTile)
    {
        QFile f(file);

        if(f.open(QIODevice::WriteOnly | QIODevice::Truncate))
        {
            QTextStream out(&f);

            out << "solid QEditor_map" << endl;

            /// South
            getFace(South)->writeToFile(&out, scale);

            /// Left
            getFace(Left)->writeToFile(&out, scale);

            /// Top
            getFace(Top)->writeToFile(&out, scale);

            /// Right
            getFace(Right)->writeToFile(&out, scale);

            /// Bottom
            getFace(Bottom)->writeToFile(&out, scale);

            /// North
            getFace(North)->writeToFile(&out, scale);

            out << "endsolid QEditor_map";

            out.flush();

            qDebug() << QObject::tr("Success STL data write to") << f.fileName();

            f.close();
        }
    }
}

void STL::createData()
{
    if(mapTile)
    {
        /// Resolution settings
        const int STLwidth  = stlResolution == High ? TILE_WIDTH  : TILE_WIDTH  / 4;
        const int STLheight = stlResolution == High ? TILE_HEIGHT : TILE_HEIGHT / 4;

        QVector<QVector3D> datas;

        /// Top
        // generate 2d heightmap array
        for(int chunkX = 0; chunkX < CHUNKS; ++chunkX)
        {
            for(int chunkY = 0; chunkY < CHUNKS; ++chunkY)
            {
                for(int x = 0; x < CHUNK_WIDTH; ++x)
                {
                    for(int y = 0; y < CHUNK_HEIGHT; ++y)
                    {
                        int xIndex = chunkX * CHUNK_WIDTH + x;
                        int yIndex = chunkY * CHUNK_HEIGHT + y;

                        float xPos = MathHelper::toFloat(xIndex) / MathHelper::toFloat(TILE_WIDTH) * TILESIZE;
                        float zPos = MathHelper::toFloat(yIndex) / MathHelper::toFloat(TILE_HEIGHT) * TILESIZE;

                        QVector3D vertex(xPos, mapTile->getChunk(chunkX, chunkY)->getHeight(x, y), zPos);

                        height[xIndex][yIndex] = vertex;
                    }
                }
            }
        }

        if(stlResolution == Low) // we need to divide array
        {
            for(int y = 0; y < TILE_HEIGHT; y += 4)
            {
                for(int x = 0; x < TILE_WIDTH; x += 4)
                {
                    float _height = height[x + 0][y + 0].y() + height[x + 1][y + 0].y() + height[x + 2][y + 0].y() + height[x + 3][y + 0].y()
                                  + height[x + 0][y + 1].y() + height[x + 1][y + 1].y() + height[x + 2][y + 1].y() + height[x + 3][y + 1].y()
                                  + height[x + 0][y + 2].y() + height[x + 1][y + 2].y() + height[x + 2][y + 2].y() + height[x + 3][y + 2].y()
                                  + height[x + 0][y + 3].y() + height[x + 1][y + 3].y() + height[x + 2][y + 3].y() + height[x + 3][y + 3].y();
                    _height      /= 16.0f;

                    QVector3D vector = height[x / 4][y / 4];
                    vector.setY(_height);

                    height[x / 4][y / 4] = vector;
                }
            }

            for(int y = 0; y < STLheight; ++y)
            {
                for(int x = 0; x < STLwidth; ++x)
                {
                    float xPos = MathHelper::toFloat(x) / MathHelper::toFloat(STLwidth) * TILESIZE;
                    float zPos = MathHelper::toFloat(y) / MathHelper::toFloat(STLheight) * TILESIZE;

                    QVector3D vector = height[x][y];
                    vector.setX(xPos);
                    vector.setZ(zPos);

                    height[x][y] = vector;
                }
            }
        }

        for(int y = 0; y < STLheight; ++y)
        {
            for(int x = 0; x < STLwidth; ++x)
                datas.append(height[x][y]);
        }

        getFace(Top)->setInputData(datas);

        /// South
        datas.clear();

        for(int y = 0; y < 2; ++y)
        {
            for(int x = 0; x < STLwidth; ++x)
            {
                if(y != STLheight - 1)
                    dataSouth[x][STLheight - 1 - y] = QVector3D();
                else
                    dataSouth[x][0] = height[x][STLheight - 1];
            }
        }

        for(int y = 0; y < 2; ++y)
        {
            for(int x = 0; x < STLwidth; ++x)
                datas.append(dataSouth[x][y]);
        }

        getFace(South)->setInputData(datas);

        /// Left
        datas.clear();

        for(int y = 0; y < 2; ++y)
        {
            for(int x = 0; x < STLwidth; ++x)
            {
                if(y != 0)
                    dataLeft[x][y] = QVector3D();
                else
                    dataLeft[x][0] = height[0][x];
            }
        }

        for(int y = 0; y < 2; ++y)
        {
            for(int x = 0; x < STLwidth; ++x)
                datas.append(dataLeft[x][y]);
        }

        getFace(Left)->setInputData(datas);

        /// Right
        datas.clear();

        for(int y = 0; y < 2; ++y)
        {
            for(int x = 0; x < STLwidth; ++x)
            {
                if(y != 0)
                    dataRight[x][y] = QVector3D();
                else
                    dataRight[x][0] = height[STLwidth - 1][x];
            }
        }

        for(int y = 0; y < 2; ++y)
        {
            for(int x = 0; x < STLwidth; ++x)
                datas.append(dataRight[x][y]);
        }

        getFace(Right)->setInputData(datas);

        /// Bottom
        datas.clear();

        dataBottom[0][0] = QVector3D(0.0f,     0.0f, 0.0f);
        dataBottom[1][0] = QVector3D(TILESIZE, 0.0f, 0.0f);
        dataBottom[0][1] = QVector3D(0.0f,     0.0f, TILESIZE);
        dataBottom[1][1] = QVector3D(TILESIZE, 0.0f, TILESIZE);

        for(int y = 0; y < 2; ++y)
        {
            for(int x = 0; x < 2; ++x)
                datas.append(dataBottom[x][y]);
        }

        getFace(Bottom)->setInputData(datas);

        /// North
        datas.clear();

        for(int y = 0; y < 2; ++y)
        {
            for(int x = 0; x < STLwidth; ++x)
            {
                if(y != 0)
                    dataNorth[x][y] = QVector3D();
                else
                    dataNorth[x][0] = height[STLwidth - 1 - x][0];
            }
        }

        for(int y = 0; y < 2; ++y)
        {
            for(int x = 0; x < STLwidth; ++x)
                datas.append(dataNorth[x][y]);
        }

        getFace(North)->setInputData(datas);

        /// Generate vertices
        for(int i = 0; i < faces.count(); ++i)
            faces[i]->createVertices();
    }
}

void STL::setMapTile(MapTile* tile)
{
    mapTile = tile;
}

Face* STL::getFace(Facets facet)
{
    for(int i = 0; i < faces.count(); ++i)
    {
        if(faces[i]->getFacet() == facet)
            return faces[i];
    }
}

Face::Face(STL::Facets face) : facet(face)
{
}

Face::~Face()
{
}

void Face::writeToFile(QTextStream* stream, float scale)
{
    for(int i = 0; i < vertexs.count(); ++i)
        vertexs[i].scale(scale);

    for(int i = 0; i < vertexs.count(); ++i)
    {
        QVector3D vertex1 = vertexs[i].getVertex(1);
        QVector3D vertex2 = vertexs[i].getVertex(2);
        QVector3D vertex3 = vertexs[i].getVertex(3);

        switch(facet)
        {
            case STL::South:
                {
                    QVector3D normal = NORMALS_SOUTH;

                    *stream << "facet normal " << normal.x() << " " << normal.y() << " " << normal.z() << endl;
                }
                break;

            case STL::Left:
                {
                    QVector3D normal = NORMALS_LEFT;

                    *stream << "facet normal " << normal.x() << " " << normal.y() << " " << normal.z() << endl;
                }
                break;

            case STL::Top:
                {
                    QVector3D normal = NORMALS_TOP;

                    *stream << "facet normal " << normal.x() << " " << normal.y() << " " << normal.z() << endl;
                }
                break;

            case STL::Right:
                {
                    QVector3D normal = NORMALS_RIGHT;

                    *stream << "facet normal " << normal.x() << " " << normal.y() << " " << normal.z() << endl;
                }
                break;

            case STL::Bottom:
                {
                    QVector3D normal = NORMALS_BOTTOM;

                    *stream << "facet normal " << normal.x() << " " << normal.y() << " " << normal.z() << endl;
                }
                break;

            case STL::North:
                {
                    QVector3D normal = NORMALS_NORTH;

                    *stream << "facet normal " << normal.x() << " " << normal.y() << " " << normal.z() << endl;
                }
                break;
        }

        *stream << "  outer loop" << endl;
        *stream << "    vertex  " << vertex1.x() << "  " << vertex1.y() << " " << vertex1.z() << endl;
        *stream << "    vertex  " << vertex2.x() << "  " << vertex2.y() << " " << vertex2.z() << endl;
        *stream << "    vertex  " << vertex3.x() << "  " << vertex3.y() << " " << vertex3.z() << endl;
        *stream << "  endloop" << endl;
        *stream << "endfacet" << endl;
    }
}

void Face::createVertices()
{
    /// Resolution settings
    const int STLwidth  = facetResolution == STL::High ? TILE_WIDTH  : TILE_WIDTH  / 4;
    const int STLheight = facetResolution == STL::High ? TILE_HEIGHT : TILE_HEIGHT / 4;

    switch(facet)
    {
        case STL::Top:
            {
                for(int y = 0; y < STLheight - 1; ++y)
                {
                    for(int x = 0; x < STLwidth - 1; ++x)
                    {
                        QVector3D vertexTopLeft  = data[x][y];
                        QVector3D vertexTopRight = data[x + 1][y];
                        QVector3D vertexBotLeft  = data[x][y + 1];
                        QVector3D vertexBotRight = data[x + 1][y + 1];

                        Vertex vertexTop = Vertex(vertexTopLeft,  vertexBotLeft, vertexTopRight);
                        Vertex vertexBot = Vertex(vertexTopRight, vertexBotLeft, vertexBotRight);

                        vertexs.append(vertexTop);
                        vertexs.append(vertexBot);
                    }
                }
            }
            break;

        case STL::South:
        case STL::Left:
        case STL::Right:
        case STL::North:
            {
                for(int y = 0; y < 2 - 1; ++y)
                {
                    for(int x = 0; x < STLwidth - 1; ++x)
                    {
                        QVector3D vertexTopLeft  = data[x][y];
                        QVector3D vertexTopRight = data[x + 1][y];
                        QVector3D vertexBotLeft  = data[x][y + 1];
                        QVector3D vertexBotRight = data[x + 1][y + 1];

                        Vertex vertexTop = Vertex(vertexTopLeft,  vertexBotLeft, vertexTopRight);
                        Vertex vertexBot = Vertex(vertexTopRight, vertexBotLeft, vertexBotRight);

                        vertexs.append(vertexTop);
                        vertexs.append(vertexBot);
                    }
                }

                data[0][0] = QVector3D(0.0f,     1.0f, 0.0f);
                data[1][0] = QVector3D(TILESIZE, 1.0f, 0.0f);
                data[0][1] = QVector3D(0.0f,     0.0f, TILESIZE);
                data[1][1] = QVector3D(TILESIZE, 0.0f, TILESIZE);

                for(int y = 0; y < 2 - 1; ++y)
                {
                    for(int x = 0; x < 2 - 1; ++x)
                    {
                        QVector3D vertexTopLeft  = data[x][y];
                        QVector3D vertexTopRight = data[x + 1][y];
                        QVector3D vertexBotLeft  = data[x][y + 1];
                        QVector3D vertexBotRight = data[x + 1][y + 1];

                        Vertex vertexTop = Vertex(vertexTopLeft,  vertexBotLeft, vertexTopRight);
                        Vertex vertexBot = Vertex(vertexTopRight, vertexBotLeft, vertexBotRight);

                        vertexs.append(vertexTop);
                        vertexs.append(vertexBot);
                    }
                }
            }
            break;

        case STL::Bottom:
            {
                for(int y = 0; y < 2 - 1; ++y)
                {
                    for(int x = 0; x < 2 - 1; ++x)
                    {
                        QVector3D vertexTopLeft  = data[x][y];
                        QVector3D vertexTopRight = data[x + 1][y];
                        QVector3D vertexBotLeft  = data[x][y + 1];
                        QVector3D vertexBotRight = data[x + 1][y + 1];

                        Vertex vertexTop = Vertex(vertexTopLeft,  vertexBotLeft, vertexTopRight);
                        Vertex vertexBot = Vertex(vertexTopRight, vertexBotLeft, vertexBotRight);

                        vertexs.append(vertexTop);
                        vertexs.append(vertexBot);
                    }
                }
            }
            break;
    }
}

void Face::setInputData(QVector<QVector3D>& datas)
{
    /// Resolution settings
    const int STLwidth  = facetResolution == STL::High ? TILE_WIDTH  : TILE_WIDTH  / 4;
    const int STLheight = facetResolution == STL::High ? TILE_HEIGHT : TILE_HEIGHT / 4;

    switch(facet)
    {
        case STL::South:
        case STL::Top: // y = 1024 others got only 2
        case STL::Left:
        case STL::Right:
        case STL::North:
            {
                int _x = 0;
                int _y = 0;

                for(int i = 0; i < datas.count(); ++i)
                {
                    data[_x][_y] = datas[i];

                    if(_x == STLwidth - 1)
                    {
                        _x = 0;

                        ++_y;
                    }
                    else
                        ++_x;
                }
            }
            break;

        case STL::Bottom:
            {
                int _x = 0;
                int _y = 0;

                for(int i = 0; i < datas.count(); ++i)
                {
                    data[_x][_y] = datas[i];

                    if(_x == 1)
                    {
                        _x = 0;

                        ++_y;
                    }
                    else
                        ++_x;
                }
            }
            break;
    }

    switch(facet)
    {
        case STL::South:
            {
                for(int y = 0; y < 2; ++y)
                {
                    for(int x = 0; x < STLwidth; ++x)
                    {
                        float xPos = MathHelper::toFloat(x) / MathHelper::toFloat(STLwidth) * TILESIZE;
                        float zPos = 1.0f * TILESIZE;

                        if(y != 0)
                            data[x][y] = QVector3D(xPos, 1.0f, zPos);
                        else
                            data[x][y] = QVector3D(xPos, data[x][y].y(), zPos);
                    }
                }
            }
            break;

        case STL::Left:
            {
                for(int y = 0; y < 2; ++y)
                {
                    for(int x = 0; x < STLwidth; ++x)
                    {
                        float xPos = 0.0f;
                        float zPos = MathHelper::toFloat(x) / MathHelper::toFloat(STLheight) * TILESIZE;

                        if(y != 0)
                            data[x][y] = QVector3D(xPos, 1.0f, zPos);
                        else
                            data[x][y] = QVector3D(xPos, data[x][y].y(), zPos);
                    }
                }
            }
            break;

        case STL::Right:
            {
                for(int y = 0; y < 2; ++y)
                {
                    for(int x = 0; x < STLwidth; ++x)
                    {
                        float xPos = 1.0f * TILESIZE;
                        float zPos = MathHelper::toFloat(STLheight - x) / MathHelper::toFloat(STLheight) * TILESIZE;

                        if(y != 0)
                            data[x][y] = QVector3D(xPos, 1.0f, zPos);
                        else
                            data[x][y] = QVector3D(xPos, data[x][y].y(), zPos);
                    }
                }
            }
            break;

        case STL::North:
            {
                for(int y = 0; y < 2; ++y)
                {
                    for(int x = 0; x < STLwidth; ++x)
                    {
                        float xPos = MathHelper::toFloat(STLwidth - x) / MathHelper::toFloat(STLwidth) * TILESIZE;
                        float zPos = 0.0f;

                        if(y != 0)
                            data[x][y] = QVector3D(xPos, 1.0f, zPos);
                        else
                            data[x][y] = QVector3D(xPos, data[x][y].y(), zPos);
                    }
                }
            }
            break;
    }
}

void Face::setResolution(STL::Resolution resolution)
{
    facetResolution = resolution;
}

Vertex::Vertex()
{
}

Vertex::Vertex(QVector3D vertex_1, QVector3D vertex_2, QVector3D vertex_3)
: vertex1(vertex_1)
, vertex2(vertex_2)
, vertex3(vertex_3)
{
}

Vertex::~Vertex()
{
}

void Vertex::scale(const float value)
{
    vertex1 *= value;
    vertex2 *= value;
    vertex3 *= value;
}

const QVector3D Vertex::getVertex(const int i) const
{
    switch(i)
    {
        case 1:
            return vertex1;

        case 2:
            return vertex2;

        case 3:
            return vertex3;

        default:
            return QVector3D();
    }
}
