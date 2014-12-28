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

                        float xPos = MathHelper::toFloat(xIndex) / MathHelper::toFloat(TILE_WIDTH - 1)  * TILESIZE;
                        float zPos = MathHelper::toFloat(yIndex) / MathHelper::toFloat(TILE_HEIGHT - 1) * TILESIZE;

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
                    float xPos = MathHelper::toFloat(x) / MathHelper::toFloat(STLwidth - 1)  * TILESIZE;
                    float zPos = MathHelper::toFloat(y) / MathHelper::toFloat(STLheight - 1) * TILESIZE;

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
                if(y != 0)
                    dataSouth[x][y] = QVector3D();
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

        // maybe switch X's, now just for proper display in sketchup
        dataBottom[0][0] = QVector3D(TILESIZE,     0.0f, 0.0f);
        dataBottom[1][0] = QVector3D(0.0f, 0.0f, 0.0f);
        dataBottom[0][1] = QVector3D(TILESIZE,     0.0f, TILESIZE);
        dataBottom[1][1] = QVector3D(0.0f, 0.0f, TILESIZE);

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

        /// Optimalize vertices
        for(int i = 0; i < faces.count(); ++i)
            faces[i]->optimalize();
    }
}

void STL::surfaceSize(float mm, bool scaleHeight)
{
    for(int i = 0; i < faces.size(); ++i)
        faces[i]->setSurfaceSize(mm, scaleHeight);
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

Face::Face(STL::Facets face, QObject* parent)
: QObject(parent)
, facet(face)
, readyToWrite(true)
{
}

Face::~Face()
{
}

void Face::writeToFile(QTextStream* stream, float scale)
{
    while(!readyToWrite)
    {
        QThread::sleep(1);
    }

    for(int i = 0; i < vertexs.count(); ++i)
        vertexs[i].scale(scale);

    for(int i = 0; i < vertexs.count(); ++i)
    {
        QVector3D vertex1 = vertexs[i].getVertex(1);
        QVector3D vertex2 = vertexs[i].getVertex(2);
        QVector3D vertex3 = vertexs[i].getVertex(3);
        QVector3D vertex4 = vertexs[i].getVertex(4);

        QString facet_normal = QString();

        switch(facet)
        {
            case STL::South:
                {
                    QVector3D normal = NORMALS_SOUTH;

                    facet_normal = QString("facet normal %1 %2 %3").arg(normal.x()).arg(normal.y()).arg(normal.z());
                }
                break;

            case STL::Left:
                {
                    QVector3D normal = NORMALS_LEFT;

                    facet_normal = QString("facet normal %1 %2 %3").arg(normal.x()).arg(normal.y()).arg(normal.z());
                }
                break;

            case STL::Top:
                {
                    QVector3D normal = NORMALS_TOP;

                    facet_normal = QString("facet normal %1 %2 %3").arg(normal.x()).arg(normal.y()).arg(normal.z());
                }
                break;

            case STL::Right:
                {
                    QVector3D normal = NORMALS_RIGHT;

                    facet_normal = QString("facet normal %1 %2 %3").arg(normal.x()).arg(normal.y()).arg(normal.z());
                }
                break;

            case STL::Bottom:
                {
                    QVector3D normal = NORMALS_BOTTOM;

                    facet_normal = QString("facet normal %1 %2 %3").arg(normal.x()).arg(normal.y()).arg(normal.z());
                }
                break;

            case STL::North:
                {
                    QVector3D normal = NORMALS_NORTH;

                    facet_normal = QString("facet normal %1 %2 %3").arg(normal.x()).arg(normal.y()).arg(normal.z());
                }
                break;
        }

        *stream << facet_normal << endl;
        *stream << "  outer loop" << endl;
        *stream << "    vertex  " << vertex1.x() << "  " << vertex1.y() << " " << vertex1.z() << endl;
        *stream << "    vertex  " << vertex4.x() << "  " << vertex4.y() << " " << vertex4.z() << endl;
        *stream << "    vertex  " << vertex3.x() << "  " << vertex3.y() << " " << vertex3.z() << endl;
        *stream << "  endloop" << endl;
        *stream << "endfacet" << endl;

        *stream << facet_normal << endl;
        *stream << "  outer loop" << endl;
        *stream << "    vertex  " << vertex1.x() << "  " << vertex1.y() << " " << vertex1.z() << endl;
        *stream << "    vertex  " << vertex2.x() << "  " << vertex2.y() << " " << vertex2.z() << endl;
        *stream << "    vertex  " << vertex4.x() << "  " << vertex4.y() << " " << vertex4.z() << endl;
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

                        vertexs.append(Vertex(vertexTopLeft,  vertexBotLeft, vertexTopRight, vertexBotRight));
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

                        vertexs.append(Vertex(vertexTopLeft,  vertexBotLeft, vertexTopRight, vertexBotRight));
                    }
                }

                switch(facet)
                {
                    case STL::North: // maybe switch X's, now just for proper display in sketchup
                        {
                            data[0][0] = QVector3D(TILESIZE, 1.0f, 0.0f);
                            data[1][0] = QVector3D(0.0f,     1.0f, 0.0f);
                            data[0][1] = QVector3D(TILESIZE, 0.0f, 0.0f);
                            data[1][1] = QVector3D(0.0f,     0.0f, 0.0f);
                        }
                        break;

                    case STL::South:
                        {
                            data[0][0] = QVector3D(0.0f,     1.0f, TILESIZE);
                            data[1][0] = QVector3D(TILESIZE, 1.0f, TILESIZE);
                            data[0][1] = QVector3D(0.0f,     0.0f, TILESIZE);
                            data[1][1] = QVector3D(TILESIZE, 0.0f, TILESIZE);
                        }
                        break;

                    case STL::Left:
                        {
                            data[0][0] = QVector3D(0.0f, 1.0f, 0.0f);
                            data[1][0] = QVector3D(0.0f, 1.0f, TILESIZE);
                            data[0][1] = QVector3D(0.0f, 0.0f, 0.0f);
                            data[1][1] = QVector3D(0.0f, 0.0f, TILESIZE);
                        }
                        break;

                    case STL::Right:
                        {
                            data[0][0] = QVector3D(TILESIZE, 1.0f, TILESIZE);
                            data[1][0] = QVector3D(TILESIZE, 1.0f, 0.0f);
                            data[0][1] = QVector3D(TILESIZE, 0.0f, TILESIZE);
                            data[1][1] = QVector3D(TILESIZE, 0.0f, 0.0f);
                        }
                        break;
                }

                for(int y = 0; y < 2 - 1; ++y)
                {
                    for(int x = 0; x < 2 - 1; ++x)
                    {
                        QVector3D vertexTopLeft  = data[x][y];
                        QVector3D vertexTopRight = data[x + 1][y];
                        QVector3D vertexBotLeft  = data[x][y + 1];
                        QVector3D vertexBotRight = data[x + 1][y + 1];

                        vertexs.append(Vertex(vertexTopLeft,  vertexBotLeft, vertexTopRight, vertexBotRight));
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

                        vertexs.append(Vertex(vertexTopLeft,  vertexBotLeft, vertexTopRight, vertexBotRight));
                    }
                }
            }
            break;
    }
}

void Face::optimalize()
{
    switch(facet)
    {
        case STL::Top:
            {
                switch(facetResolution)
                {
                    case STL::High: // 16 threads
                        {
                            qRegisterMetaType<VertexArray>("VertexArray");

                            for(int i = 0; i < 16; ++i)
                            {
                                FaceWorker* worker = new FaceWorker(vertexs, i, this, this);

                                connect(worker, &QThread::finished, worker, &QObject::deleteLater);

                                worker->start();
                            }

                            readyToWrite = false;
                        }
                        break;

                    case STL::Low: // no threads
                        {
                            // until each next vertexs is different
                            bool done = false;

                            while(!done)
                            {
                                for(int i = 0; i < vertexs.count() - 1; ++i)
                                {
                                    Vertex vertex1 = vertexs[i];
                                    Vertex vertex2 = vertexs[i + 1];

                                    // horizontal merge
                                    if(vertex1.getVertex(1).y() - vertex1.getVertex(3).y() != vertex2.getVertex(1).y() - vertex2.getVertex(3).y()
                                    || vertex1.getVertex(2).y() - vertex1.getVertex(4).y() != vertex2.getVertex(2).y() - vertex2.getVertex(4).y()
                                    || vertex1.getVertex(3) != vertex2.getVertex(1) || vertex1.getVertex(4) != vertex2.getVertex(2)
                                    || vertex1.rows != vertex2.rows)
                                    {
                                        // try verticall merge
                                        if(vertex1.getVertex(1).y() - vertex1.getVertex(2).y() != vertex2.getVertex(1).y() - vertex2.getVertex(2).y()
                                        || vertex1.getVertex(3).y() - vertex1.getVertex(4).y() != vertex2.getVertex(3).y() - vertex2.getVertex(4).y()
                                        || vertex1.getVertex(2) != vertex2.getVertex(1) || vertex1.getVertex(4) != vertex2.getVertex(3)
                                        || vertex1.columns != vertex2.columns)
                                        {
                                            done = true;

                                            continue;
                                        }

                                        done = false;

                                        vertexs[i].setVertex(vertex2.getVertex(2), 2);
                                        vertexs[i].setVertex(vertex2.getVertex(4), 4);
                                        vertexs[i].rows += vertex2.rows;

                                        vertexs.removeAt(i + 1);

                                        break;
                                    }

                                    done = false;

                                    vertexs[i].setVertex(vertex2.getVertex(3), 3);
                                    vertexs[i].setVertex(vertex2.getVertex(4), 4);
                                    vertexs[i].columns += vertex2.columns;

                                    vertexs.removeAt(i + 1);

                                    break;
                                }

                                if(vertexs.count() < 2)
                                    done = true;
                            }
                        }
                        break;
                }
            }
            break;

        case STL::North:
        case STL::South:
        case STL::Left:
        case STL::Right:
            {
                // until each next vertexs is different
                bool done = false;

                while(!done)
                {
                    for(int i = 0; i < vertexs.count() - 1 - 1; ++i) // -1 last vertex is already prepared
                    {
                        Vertex vertex1 = vertexs[i];
                        Vertex vertex2 = vertexs[i + 1];

                        // horizontal merge, there is no vertical
                        if(vertex1.getVertex(1).y() - vertex1.getVertex(3).y() != vertex2.getVertex(1).y() - vertex2.getVertex(3).y()
                        || vertex1.getVertex(2).y() - vertex1.getVertex(4).y() != vertex2.getVertex(2).y() - vertex2.getVertex(4).y()
                        || vertex1.getVertex(3) != vertex2.getVertex(1) || vertex1.getVertex(4) != vertex2.getVertex(2))
                        {
                            done = true;

                            continue;
                        }

                        done = false;

                        vertexs[i].setVertex(vertex2.getVertex(3), 3);
                        vertexs[i].setVertex(vertex2.getVertex(4), 4);
                        vertexs[i].columns += vertex2.columns;

                        vertexs.removeAt(i + 1);

                        break;
                    }

                    if(vertexs.count() < 4)
                        done = true;
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
                        float xPos = MathHelper::toFloat(x) / MathHelper::toFloat(STLwidth - 1) * TILESIZE;
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
                        float zPos = MathHelper::toFloat(x) / MathHelper::toFloat(STLheight - 1) * TILESIZE;

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
                        float zPos = MathHelper::toFloat(STLheight - (x + 1)) / MathHelper::toFloat(STLheight - 1) * TILESIZE;

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
                        float xPos = MathHelper::toFloat(STLwidth - (x + 1)) / MathHelper::toFloat(STLwidth - 1) * TILESIZE;
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

void Face::setSurfaceSize(float mm, bool scaleHeight)
{
    while(!readyToWrite)
        QThread::sleep(1);

    for(int i = 0; i < vertexs.count(); ++i)
        vertexs[i].setSurfaceSize(mm, scaleHeight);
}

void Face::handleResults(VertexArray container, int threadID)
{
    qDebug() << "Thread" << threadID << "has done computing...";

    threadsVertexs.insert(threadID, container);

    if(threadsVertexs.keys().count() == 16) // Threads finished
    {
        qDebug() << "Threads finished computing! Main thread calculate last optimalization...";

        vertexs.clear();

        for(int i = 0; i < 16; ++i)
        {
            QVector<Vertex> vertexList = threadsVertexs.value(i);

            for(int j = 0; j < vertexList.count(); ++j)
                vertexs.append(vertexList.at(j));
        }

        // last optimalization
        // until each next vertexs is different
        bool done = false;

        while(!done)
        {
            for(int i = 0; i < vertexs.count() - 1; ++i)
            {
                Vertex vertex1 = vertexs[i];
                Vertex vertex2 = vertexs[i + 1];

                // horizontal merge
                if(vertex1.getVertex(1).y() - vertex1.getVertex(3).y() != vertex2.getVertex(1).y() - vertex2.getVertex(3).y()
                || vertex1.getVertex(2).y() - vertex1.getVertex(4).y() != vertex2.getVertex(2).y() - vertex2.getVertex(4).y()
                || vertex1.getVertex(3) != vertex2.getVertex(1) || vertex1.getVertex(4) != vertex2.getVertex(2)
                || vertex1.rows != vertex2.rows)
                {
                    // try verticall merge
                    if(vertex1.getVertex(1).y() - vertex1.getVertex(2).y() != vertex2.getVertex(1).y() - vertex2.getVertex(2).y()
                    || vertex1.getVertex(3).y() - vertex1.getVertex(4).y() != vertex2.getVertex(3).y() - vertex2.getVertex(4).y()
                    || vertex1.getVertex(2) != vertex2.getVertex(1) || vertex1.getVertex(4) != vertex2.getVertex(3)
                    || vertex1.columns != vertex2.columns)
                    {
                        done = true;

                        continue;
                    }

                    done = false;

                    vertexs[i].setVertex(vertex2.getVertex(2), 2);
                    vertexs[i].setVertex(vertex2.getVertex(4), 4);
                    vertexs[i].rows += vertex2.rows;

                    vertexs.removeAt(i + 1);

                    break;
                }

                done = false;

                vertexs[i].setVertex(vertex2.getVertex(3), 3);
                vertexs[i].setVertex(vertex2.getVertex(4), 4);
                vertexs[i].columns += vertex2.columns;

                vertexs.removeAt(i + 1);

                break;
            }

            if(vertexs.count() < 2)
                done = true;
        }

        readyToWrite = true;
    }
}

FaceWorker::FaceWorker(QVector<Vertex> &container, int threadID, Face* faceParent, QObject* parent)
: QThread(parent)
, face(faceParent)
, vertexs(container)
, threadNumber(threadID)
{
    qRegisterMetaType<VertexArray>("VertexArray");
}

FaceWorker::~FaceWorker()
{
}

void FaceWorker::run()
{
    // remove another thread vertexs
    int vertexsTotal = 256 * 256;
    int startOffset  = vertexsTotal * threadNumber;
    int endOffset    = startOffset + vertexsTotal;

    QVector<Vertex> vertexs2;

    if(vertexs.count() < endOffset)
        endOffset = vertexs.count();

    for(int i = startOffset; i < endOffset; ++i)
        vertexs2.append(vertexs[i]);

    vertexs = vertexs2;

    // until each next vertexs is different
    bool done = false;

    while(!done)
    {
        for(int i = 0; i < vertexs.count() - 1; ++i)
        {
            Vertex vertex1 = vertexs[i];
            Vertex vertex2 = vertexs[i + 1];

            // horizontal merge
            if(vertex1.getVertex(1).y() - vertex1.getVertex(3).y() != vertex2.getVertex(1).y() - vertex2.getVertex(3).y()
            || vertex1.getVertex(2).y() - vertex1.getVertex(4).y() != vertex2.getVertex(2).y() - vertex2.getVertex(4).y()
            || vertex1.getVertex(3) != vertex2.getVertex(1) || vertex1.getVertex(4) != vertex2.getVertex(2)
            || vertex1.rows != vertex2.rows)
            {
                // try verticall merge
                if(vertex1.getVertex(1).y() - vertex1.getVertex(2).y() != vertex2.getVertex(1).y() - vertex2.getVertex(2).y()
                || vertex1.getVertex(3).y() - vertex1.getVertex(4).y() != vertex2.getVertex(3).y() - vertex2.getVertex(4).y()
                || vertex1.getVertex(2) != vertex2.getVertex(1) || vertex1.getVertex(4) != vertex2.getVertex(3)
                || vertex1.columns != vertex2.columns)
                {
                    done = true;

                    continue;
                }

                done = false;

                vertexs[i].setVertex(vertex2.getVertex(2), 2);
                vertexs[i].setVertex(vertex2.getVertex(4), 4);
                vertexs[i].rows += vertex2.rows;

                vertexs.removeAt(i + 1);

                break;
            }

            done = false;

            vertexs[i].setVertex(vertex2.getVertex(3), 3);
            vertexs[i].setVertex(vertex2.getVertex(4), 4);
            vertexs[i].columns += vertex2.columns;

            vertexs.removeAt(i + 1);

            break;
        }

        if(vertexs.count() < 2)
            done = true;
    }

    face->handleResults(vertexs, threadNumber);
}

Vertex::Vertex()
{
}

Vertex::Vertex(QVector3D vertex_1, QVector3D vertex_2, QVector3D vertex_3, QVector3D vertex_4)
: vertex1(vertex_1)
, vertex2(vertex_2)
, vertex3(vertex_3)
, vertex4(vertex_4)
, rows(1)
, columns(1)
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
    vertex4 *= value;
}

void Vertex::setSurfaceSize(float mm, bool scaleHeight)
{
    vertex1.setX(vertex1.x() / TILESIZE * mm);
    vertex1.setZ(vertex1.z() / TILESIZE * mm);

    vertex2.setX(vertex2.x() / TILESIZE * mm);
    vertex2.setZ(vertex2.z() / TILESIZE * mm);

    vertex3.setX(vertex3.x() / TILESIZE * mm);
    vertex3.setZ(vertex3.z() / TILESIZE * mm);

    vertex4.setX(vertex4.x() / TILESIZE * mm);
    vertex4.setZ(vertex4.z() / TILESIZE * mm);

    if(scaleHeight)
    {
        vertex1.setY(vertex1.y() / TILESIZE * mm);
        vertex2.setY(vertex2.y() / TILESIZE * mm);
        vertex3.setY(vertex3.y() / TILESIZE * mm);
        vertex4.setY(vertex4.y() / TILESIZE * mm);
    }
}

void Vertex::setVertex(const QVector3D& vector, const int& index)
{
    switch(index)
    {
        case 1:
            vertex1 = vector;
            break;

        case 2:
            vertex2 = vector;
            break;

        case 3:
            vertex3 = vector;
            break;

        case 4:
            vertex4 = vector;
            break;
    }
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

        case 4:
            return vertex4;

        default:
            return QVector3D();
    }
}
