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
