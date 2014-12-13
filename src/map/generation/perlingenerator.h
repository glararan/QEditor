#ifndef PERLINGENERATOR_H
#define PERLINGENERATOR_H

#include <QVector3D>

class PerlinGenerator
{
public:
    PerlinGenerator(int random_seed);
    ~PerlinGenerator();

    float noise(QVector3D& point);

    // get
    int getSeed() const { return seed; }

private:
    int gradientTableSize; // 256
    int seed;

    float* gradientTable;

    unsigned int* perm;

    void initGradients();

    int permutate(int x);
    int index(int ix, int iy, int iz);

    float lattice(int ix, int iy, int iz, QVector3D& point);
    float lerp(QVector3D& values);
    float smooth(float x);
};

#endif // PERLINGENERATOR_H
