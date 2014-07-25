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

#ifndef MATHHELPER_H
#define MATHHELPER_H

#define _USE_MATH_DEFINES
#include <qmath.h>

#include <QtGlobal>
#include <qgl.h> // GLuint
#include <QVector3D>
#include <QVector4D>
#include <QColor>

class BezierCurve;

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

    static const QColor    toColor(const QVector4D& color);
    static const QVector4D toVector4D(const QColor& color);

    static const QVector2D getDirections(const QMatrix4x4& matrix);

    static const float closerTo(const unsigned char value1, const qreal value2, const float formula);

    static const float toRGB(const qreal value);
    static const int   toRGBInt(const qreal value);

    static const int round(const int number, const int multiple);

    static const float lerp(const float v0, const float v1, const float t);
    static void lerp(QVector3D& dest, const QVector3D& a, const QVector3D& b, const float t);

    static void bezier(QVector3D& dest, const QVector3D& a, const QVector3D& b, const QVector3D& c, const QVector3D& d, const float t);
    static QVector3D bezierOnSpline(BezierCurve* curve, float t);

    static void addGravity(float& step, float dt, float weight = 50.0f);       // 50.0f == 50kg
    static void addKineticEnergy(float& step, float dt, float weight = 50.0f); // 50.0f == 50kg

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