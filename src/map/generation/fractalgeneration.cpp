#include "fractalgeneration.h"

#include "mapheaders.h"
#include "mathhelper.h"

#include <QDebug>

FractalGeneration::FractalGeneration(unsigned int Xgrid, unsigned int Ygrid, unsigned int Octaves, float Gain, float Lacunarity, float Frequency, float Amplitude)
: xgrid(Xgrid)
, ygrid(Ygrid)
, max_octaves(Octaves)
, gain(Gain)
, lacunarity(Lacunarity)
, start_amplitude(Amplitude)
, start_frequency(Frequency)
{
    for(int i = 0; i < 256; ++i)
        permutation_table[i] = i;

    for(int i = 0; i < 256; ++i)
    {
        int j = qrand() & 255;
        int k = permutation_table[i];

        permutation_table[i] = permutation_table[j];
        permutation_table[j] = k;
    }

    unsigned int j, k;

    // gradient
    for(int i = 0; i < 8; ++i)
    {
        for(j = 0, k = 1; j < 3; ++j, k <<= 1)
            gradient_table[i][j] = (i & k) ? -1 : 1;
    }
}

FractalGeneration::~FractalGeneration()
{
}

void FractalGeneration::makeSomeNoise(float*& mapData, int z)
{
    float amplitude, frequency, final_val;

    for(int x = 0; x < xgrid; ++x)
    {
        for(int y = 0; y < ygrid; ++y)
        {
            frequency = start_frequency;
            amplitude = start_amplitude;
            final_val = 0.0f;

            for(int octave = 0; octave < max_octaves; ++octave)
            {
                final_val += amplitude * make_point(QVector3D(MathHelper::toFloat(x) * frequency, MathHelper::toFloat(y) * frequency, MathHelper::toFloat(z) * frequency));

                frequency *= lacunarity;
                amplitude *= gain;
            }

            mapData[xgrid * y + x] = final_val;
        }
    }

    float min = mapData[0];

    // normalize mapData
    for(int x = 0; x < xgrid; ++x)
    {
        for(int y = 0; y < ygrid; ++y)
        {
            if(mapData[xgrid * y + x] < min)
                min = mapData[xgrid * y + x];
        }
    }

    // add to everythink min
    for(int x = 0; x < xgrid; ++x)
    {
        for(int y = 0; y < ygrid; ++y)
            mapData[xgrid * y + x] += min;
    }
}

float FractalGeneration::make_point(QVector3D& point)
{
    int   corners[4][3];
    float distances[4][3];

    float general_skew  = 1.0f / 3.0f;
    float specific_skew = point.x() + point.y() + point.z();

    corners[0][0] = myFloor(point.x() + specific_skew);
    corners[0][1] = myFloor(point.y() + specific_skew);
    corners[0][2] = myFloor(point.z() + specific_skew);

    float general_unskew  = 1.0f / 6.0f;
    float specific_unskew = MathHelper::toFloat(corners[0][0] + corners[0][1] + corners[0][2]) * general_unskew;

    distances[0][0] = point.x() - MathHelper::toFloat(corners[0][0]) + specific_unskew;
    distances[0][1] = point.y() - MathHelper::toFloat(corners[0][1]) + specific_unskew;
    distances[0][2] = point.z() - MathHelper::toFloat(corners[0][2]) + specific_unskew;

    if(distances[0][0] < distances[0][1])
    {
        if(distances[0][1] < distances[0][2])
        {
            corners[1][0] = 0;
            corners[1][1] = 0;
            corners[1][2] = 1;

            corners[2][0] = 0;
            corners[2][1] = 1;
            corners[2][2] = 1;
        }
        else if(distances[0][0] < distances[0][2])
        {
            corners[1][0] = 0;
            corners[1][1] = 1;
            corners[1][2] = 0;

            corners[2][0] = 0;
            corners[2][1] = 0;
            corners[2][2] = 1;
        }
        else
        {
            corners[1][0] = 0;
            corners[1][1] = 1;
            corners[1][2] = 0;

            corners[2][0] = 1;
            corners[2][1] = 1;
            corners[2][2] = 0;
        }
    }
    else
    {
        if(distances[0][0] < distances[0][2])
        {
            corners[1][0] = 0;
            corners[1][1] = 0;
            corners[1][2] = 1;

            corners[2][0] = 1;
            corners[2][1] = 0;
            corners[2][2] = 1;
        }
        else if(distances[0][1] < distances[0][2])
        {
            corners[1][0] = 1;
            corners[1][1] = 0;
            corners[1][2] = 0;

            corners[2][0] = 1;
            corners[2][1] = 0;
            corners[2][2] = 1;
        }
        else
        {
            corners[1][0] = 1;
            corners[1][1] = 0;
            corners[1][2] = 0;

            corners[2][0] = 1;
            corners[2][1] = 1;
            corners[2][2] = 0;
        }
    }

    corners[3][0] = 1;
    corners[3][1] = 1;
    corners[3][2] = 1;

    for(int i = 0; i <= 3; ++i)
    {
        for(int j = 0; j < 3; ++j)
            distances[i][j] = distances[0][j] - MathHelper::toFloat(corners[i][j]) + general_unskew * MathHelper::toFloat(i);
    }

    int gradient_index[4];

    gradient_index[0] = permutation_table[(corners[0][0] + permutation_table[(corners[0][1] + permutation_table[corners[0][2] & 255]) & 255]) & 255] & 7;

    for(int i = 1; i < 4; ++i)
        gradient_index[i] = permutation_table[(corners[0][0] + corners[i][0] + permutation_table[(corners[0][1] + corners[i][1] + permutation_table[(corners[0][2] + corners[i][2]) & 255]) & 255]) & 255] & 7;

    float final_sum = 0.0f;

    for(int i = 0; i < 4; ++i)
        final_sum += radial_attenuation(distances[i], gradient_index[i]);

    return (32.0f * final_sum);
}

float FractalGeneration::radial_attenuation(float* distances, int gradient_index)
{
    float test_product = 0.6f - distances[0] * distances[0] - distances[1] * distances[1] - distances[2] * distances[2];

    if(test_product < 0.0f)
        return 0.0f;

    float dot_product = distances[0] * gradient_table[gradient_index][0] + distances[1] * gradient_table[gradient_index][1] + distances[2] * gradient_table[gradient_index][2];

    test_product *= test_product;

    return (test_product * test_product * dot_product);
}

int FractalGeneration::myFloor(float value)
{
    return (value >= 0 ? MathHelper::toInt(value) : MathHelper::toInt(value) - 1);
}
