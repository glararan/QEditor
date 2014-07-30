#ifndef MAPGENERATION_H
#define MAPGENERATION_H

#include <QDialog>

#include "mapheaders.h"

namespace Ui
{
    class MapGeneration;
}

class MapGeneration : public QDialog
{
    Q_OBJECT

public:
    explicit MapGeneration(QWidget* parent = 0);
    ~MapGeneration();

private:
    Ui::MapGeneration* ui;

    MapGenerationData data;

private slots:
    void regenerate();

    void setSeed(int value);
    void setPerlinNoiseLevel(QString level);
    void setPerlinNoiseHeight(int height);
    void setPerlinNoiseMultiple(int multiple);
    void setPerturbFrequency(QString frequency);
    void setPerturbD(QString d);
    void setErodeSmoothLevel(int level);
    void setErode(QString erode);

signals:
    void generate(MapGenerationData& data);
};

#endif // MAPGENERATION_H
