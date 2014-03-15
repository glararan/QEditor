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

#include "mathhelper.h"

#include "beziercurve.h"

#include <QVector3D>

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

const GLuint MathHelper::toGLuint(const int value)
{
    return static_cast<GLuint>(value);
}

const QColor MathHelper::toColor(const QVector4D& color)
{
    return QColor(toInt(color.x() * 255.0f), toInt(color.y() * 255.0f), toInt(color.z() * 255.0f), toInt(color.w() * 255.0f));
}

const QVector4D MathHelper::toVector4D(const QColor& color)
{
    return QVector4D(toFloat(color.red()) / 255.0f, toFloat(color.green()) / 255.0f, toFloat(color.blue()) / 255.0f, toFloat(color.alpha()) / 255.0f);
}

const float MathHelper::closerTo(const unsigned char value1, const qreal value2, const float formula)
{
    if(toFloat(value1) == toRGB(value2))
        return toFloat(value1);

    if(toFloat(value1) > toRGB(value2))
    {
        if(toFloat(value1) - formula > toRGB(value2))
            return toFloat(value1) - formula;
        else
            return toRGB(value2);
    }
    else
    {
        if(toFloat(value1) + formula < toRGB(value2))
            return toFloat(value1) + formula;
        else
            return toRGB(value2);
    }
}

const float MathHelper::toRGB(const qreal value)
{
    return static_cast<float>(value) * 255.0f;
}

const int MathHelper::toRGBInt(const qreal value)
{
    return static_cast<int>(value * 255.0f);
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

void MathHelper::lerp(QVector3D& dest, const QVector3D& a, const QVector3D& b, const float t)
{
    dest.setX(a.x() + (b.x() - a.x()) * t);
    dest.setY(a.y() + (b.y() - a.y()) * t);
    dest.setZ(a.z() + (b.z() - a.z()) * t);
}

void MathHelper::bezier(QVector3D& dest, const QVector3D& a, const QVector3D& b, const QVector3D& c, const QVector3D& d, const float t)
{
    QVector3D ab, bc, cd, abbc, bccd;

    lerp(ab, a, b, t); // green
    lerp(bc, b, c, t); // green
    lerp(cd, c, d, t); // green

    lerp(abbc, ab, bc, t); // blue
    lerp(bccd, bc, cd, t); // blue

    lerp(dest, abbc, bccd, t); // black
}

QVector3D MathHelper::bezierOnSpline(BezierCurve* curve, float t)
{
    float t0, t1, t2;

    QVector3D temp;

    t0 = 1 - t;
    t1 = powf(t0, 3);
    t2 = powf(t, 3);

    temp.setX(t1 * curve->points[1].x() + 3 * t * t0 * t0 * curve->points[2].x() + 3 * t * t * t0 * curve->points[3].x() + t2 * curve->points[4].x());
    temp.setY(t1 * curve->points[1].y() + 3 * t * t0 * t0 * curve->points[2].y() + 3 * t * t * t0 * curve->points[3].y() + t2 * curve->points[4].y());
    temp.setZ(t1 * curve->points[1].z() + 3 * t * t0 * t0 * curve->points[2].z() + 3 * t * t * t0 * curve->points[3].z() + t2 * curve->points[4].z());

    return temp;
}

void MathHelper::addGravity(float& step, float dt, float weight)
{
    step = lerp(step, 0.001f, (dt * (weight * 9.8f)) * 0.01f);
}

void MathHelper::addKineticEnergy(float& step, float dt, float weight)
{
    step = lerp(step, 0.009f, (dt * (0.5 * weight * 9.8f) * 0.01f));
}

const bool MathHelper::isNaN(const float value)
{
    return value != value;
}