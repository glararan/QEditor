/*This file is part of QEditor.

QEditor is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

QEditor is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with QEditor.  If not, see <http://www.gnu.org/licenses/>.*/

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
