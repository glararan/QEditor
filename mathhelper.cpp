#include "mathhelper.h"

MathHelper::MathHelper()
{
}

const float MathHelper::degreesToRadians(const float degrees)
{
    return degrees * float(M_PI / 180.0f);
}

const float MathHelper::radiansToDegrees(const float radians)
{
    return radians / float(M_PI / 180.0f);
}