#ifndef FRACTALGENERATION_H
#define FRACTALGENERATION_H

#include <QVector3D>

// Fractal Generation - Simplex Noise
class FractalGeneration
{
public:
    FractalGeneration(unsigned int Xgrid, unsigned int Ygrid, unsigned int Octaves, float Gain, float Lacunarity, float Frequency = 0.002f, float Amplitude = 50.0f);
    ~FractalGeneration();

    void makeSomeNoise(float*& mapData, int z);

    // get
    int getXgrid() const { return xgrid; }
    int getYgrid() const { return ygrid; }

private:
    bool wrapping;

    unsigned int xgrid, ygrid; // width, height
    unsigned int max_octaves;
    unsigned int permutation_table[256];

    float gradient_table[8][3];

    float lacunarity;
    float gain;
    float start_frequency;
    float start_amplitude;

    float make_point(QVector3D& point);
    float radial_attenuation(float* distance_vector, int gradient_index);

    int myFloor(float value);
};

#endif // FRACTALGENERATION_H
