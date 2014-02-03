#ifndef MATHHELPER_H
#define MATHHELPER_H

#define _USE_MATH_DEFINES
#include <qmath.h>

#include <QtGlobal>
#include <qgl.h> // GLuint

class MathHelper
{
public:
    static const float degreesToRadians(const float degrees);
    static const float radiansToDegrees(const float radians);

    static const double PI() { return M_PI; }

    static const int toInt(const float value);
    static const int toInt(const double value);
    static const int toInt(const long value);

    static const double toDouble(const int value);
    static const double toDouble(const float value);
    static const double toDouble(const long value);

    static const float toFloat(const int value);
    static const float toFloat(const double value);
    static const float toFloat(const long value);

    static const unsigned char toUChar(const int value);
    static const unsigned char toUChar(const double value);
    static const unsigned char toUChar(const float value);

    static const GLuint toGLuint(const int value);

    static const float closerTo(const unsigned char value1, const qreal value2, const float formula);

    static const float toRGB(const qreal value);
    static const int   toRGBInt(const qreal value);

    static const int round(const int number, const int multiple);

    static const float lerp(const float v0, const float v1, const float t);

    static const bool isNaN(const float value);

private:
    MathHelper();
};

#define ToInt(value)    MathHelper::toInt(value)
#define ToDouble(value) MathHelper::toDouble(value)
#define ToFloat(value)  MathHelper::toFloat(value)
#define ToUChar(value)  MathHelper::toUChar(value)
#define ToGLuint(value) MathHelper::toGLuint(value)

#endif // MATHHELPER_H