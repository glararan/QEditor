#ifndef MATHHELPER_H
#define MATHHELPER_H

#define _USE_MATH_DEFINES
#include <math.h>

class MathHelper
{
public:
    static const float degreesToRadians(const float degrees);
    static const float radiansToDegrees(const float radians);

    static const double PI() { return M_PI; }

private:
    MathHelper();
};

#endif // MATHHELPER_H
