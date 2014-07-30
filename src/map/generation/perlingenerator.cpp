#include "perlingenerator.h"

#include "mapheaders.h"
#include "mathhelper.h"

#include <QDebug>

PerlinGenerator::PerlinGenerator(int random_seed)
: gradientTableSize(256)
, seed(random_seed)
{
    gradientTable = new float[gradientTableSize * 3];

    unsigned int prePerm[] = {225,155,210,108,175,199,221,144,203,116, 70,213, 69,158, 33,252,
                              5, 82,173,133,222,139,174, 27,  9, 71, 90,246, 75,130, 91,191,
                              169,138,  2,151,194,235, 81,  7, 25,113,228,159,205,253,134,142,
                              248, 65,224,217, 22,121,229, 63, 89,103, 96,104,156, 17,201,129,
                              36,  8,165,110,237,117,231, 56,132,211,152, 20,181,111,239,218,
                              170,163, 51,172,157, 47, 80,212,176,250, 87, 49, 99,242,136,189,
                              162,115, 44, 43,124, 94,150, 16,141,247, 32, 10,198,223,255, 72,
                              53,131, 84, 57,220,197, 58, 50,208, 11,241, 28,  3,192, 62,202,
                              18,215,153, 24, 76, 41, 15,179, 39, 46, 55,  6,128,167, 23,188,
                              106, 34,187,140,164, 73,112,182,244,195,227, 13, 35, 77,196,185,
                              26,200,226,119, 31,123,168,125,249, 68,183,230,177,135,160,180,
                              12,  1,243,148,102,166, 38,238,251, 37,240,126, 64, 74,161, 40,
                              184,149,171,178,101, 66, 29, 59,146, 61,254,107, 42, 86,154,  4,
                              236,232,120, 21,233,209, 45, 98,193,114, 78, 19,206, 14,118,127,
                              48, 79,147, 85, 30,207,219, 54, 88,234,190,122, 95, 67,143,109,
                              137,214,145, 93, 92,100,245,  0,216,186, 60, 83,105, 97,204, 52};

    perm = new unsigned int[gradientTableSize];

    for(int i = 0; i < gradientTableSize; ++i)
        perm[i] = prePerm[i];

    initGradients();
}

PerlinGenerator::~PerlinGenerator()
{
    delete[] gradientTable;
    delete[] perm;
}

float PerlinGenerator::noise(QVector3D& point)
{
    int ix = MathHelper::toInt(floor(point.x()));

    float fx0 = point.x() - ix;
    float fx1 = fx0 - 1.0f;
    float wx  = smooth(fx0);

    int iy = MathHelper::toInt(floor(point.y()));

    float fy0 = point.y() - iy;
    float fy1 = fy0 - 1.0f;
    float wy  = smooth(fy0);

    int iz = MathHelper::toInt(floor(point.z()));

    float fz0 = point.z() - iz;
    float fz1 = fz0 - 1.0f;
    float wz  = smooth(fz0);

    float vx0 = lattice(ix, iy, iz, QVector3D(fx0, fy0, fz0));
    float vx1 = lattice(ix + 1, iy, iz, QVector3D(fx1, fy0, fz0));
    float vy0 = lerp(QVector3D(wx, vx0, vx1));

    vx0 = lattice(ix, iy + 1, iz, QVector3D(fx0, fy1, fz0));
    vx1 = lattice(ix + 1, iy + 1, iz, QVector3D(fx1, fy1, fz0));

    float vy1 = lerp(QVector3D(wx, vx0, vx1));
    float vz0 = lerp(QVector3D(wy, vy0, vy1));

    vx0 = lattice(ix, iy, iz + 1, QVector3D(fx0, fy0, fz1));
    vx1 = lattice(ix + 1, iy, iz + 1, QVector3D(fx1, fy0, fz1));
    vy0 = lerp(QVector3D(wx, vx0, vx1));

    vx0 = lattice(ix, iy + 1, iz + 1, QVector3D(fx0, fy1, fz1));
    vx1 = lattice(ix + 1, iy + 1, iz + 1, QVector3D(fx1, fy1, fz1));
    vy1 = lerp(QVector3D(wx, vx0, vx1));

    float vz1 = lerp(QVector3D(wy, vy0, vy1));

    return lerp(QVector3D(wz, vz0, vz1));
}

void PerlinGenerator::initGradients()
{
    for(int i = 0; i < gradientTableSize; ++i)
    {
        float z     = 1.0f - 2.0f * MathHelper::toFloat(MathHelper::randomDouble());
        float r     = MathHelper::toFloat(sqrt(1.0f - z * z));
        float theta = 2 * MathHelper::toFloat(M_PI) * MathHelper::toFloat(MathHelper::randomDouble());

        gradientTable[i * 3]     = r * MathHelper::toFloat(cos(theta));
        gradientTable[i * 3 + 1] = r * MathHelper::toFloat(sin(theta));
        gradientTable[i * 3 + 2] = z;
    }
}

int PerlinGenerator::permutate(int x)
{
    const int mask = gradientTableSize - 1;

    return perm[x & mask];
}

int PerlinGenerator::index(int ix, int iy, int iz)
{
    return permutate(ix + permutate(iy + permutate(iz)));
}

float PerlinGenerator::lattice(int ix, int iy, int iz, QVector3D& point)
{
    int _index   = index(ix, iy, iz);
    int gradient = _index * 3;

    return gradientTable[gradient] * point.x() + gradientTable[gradient + 1] * point.y() + gradientTable[gradient + 2] * point.z();
}

float PerlinGenerator::lerp(QVector3D& values)
{
    return values.y() + values.x() * (values.z() - values.y());
}

float PerlinGenerator::smooth(float x)
{
    return x * x * (3 - 2 * x);
}