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

const int MathHelper::toInt(const float value)
{
    return static_cast<int>(value);
}

const int MathHelper::toInt(const double value)
{
    return static_cast<int>(value);
}

const int MathHelper::toInt(const long value)
{
    return static_cast<int>(value);
}

const double MathHelper::toDouble(const int value)
{
    return static_cast<double>(value);
}

const double MathHelper::toDouble(const float value)
{
    return static_cast<double>(value);
}

const double MathHelper::toDouble(const long value)
{
    return static_cast<double>(value);
}

const float MathHelper::toFloat(const int value)
{
    return static_cast<float>(value);
}

const float MathHelper::toFloat(const double value)
{
    return static_cast<float>(value);
}

const float MathHelper::toFloat(const long value)
{
    return static_cast<float>(value);
}

const unsigned char MathHelper::toUChar(const int value)
{
    return static_cast<unsigned char>(value);
}

const unsigned char MathHelper::toUChar(const double value)
{
    return static_cast<unsigned char>(value);
}

const unsigned char MathHelper::toUChar(const float value)
{
    return static_cast<unsigned char>(value);
}

const int MathHelper::round(const int number, const int multiple)
{
    if(multiple == 0)
        return number;

    /*const int remainder = number % multiple;

    if(remainder == 0)
        return number;

    return number + multiple - remainder;*/

    return ((number + multiple - 1) / multiple) * multiple;
}

const float MathHelper::lerp(const float v0, const float v1, const float t)
{
    return v0 + (v1 - v0) * t;
}