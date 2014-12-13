#include "stlexport.h"

#include "maptile.h"

#include <QFile>

STLExport::STLExport()
{
}

STLExport::~STLExport()
{
}

void STLExport::exportIt(QString file)
{
    if(mapTile)
    {
        QFile f(file);

        if(f.open(QIODevice::WriteOnly | QIODevice::Truncate))
        {
            QTextStream out(&f);

            for(int y = 0; y < TILE_HEIGHT; ++y)
            {
                for(int x = 0; x < TILE_WIDTH; ++x)
                    out << data[x][y].x() << " " << data[x][y].y() << " " << data[x][y].z() << "\n";
            }

            out.flush();

            qDebug() << QObject::tr("Success STL data write to") << f.fileName();

            f.close();
        }
    }
}

void STLExport::generateData()
{
    if(mapTile)
    {
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

                        float xPos = MathHelper::toFloat(xIndex) / 1024.0f * TILESIZE;
                        float zPos = MathHelper::toFloat(yIndex) / 1024.0f * TILESIZE;

                        QVector3D vertex(xPos, mapTile->getChunk(chunkX, chunkY)->getHeight(x, y), zPos);

                        data[xIndex][yIndex] = vertex;
                    }
                }
            }
        }
    }
}

void STLExport::setMapTile(MapTile* tile)
{
    mapTile = tile;
}
