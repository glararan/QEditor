#ifndef BRUSH_H
#define BRUSH_H

#include <QVector4D>
#include <QColor>

#include "material.h"

class Brush
{
public:
    Brush(int Type = 1, float Radius = 10.0f, QColor Color = QColor::fromRgbF(0.0f, 1.0f, 0.0f, 1.0f), float Multiplier = 1.0f);
    ~Brush();

    void draw(QOpenGLShaderProgramPtr shader, QVector2D terrain_pos);

    void setBrush(const int Type);
    void setBrush(const int Type, const float Radius);
    void setBrush(const int Type, const float Radius, const QVector4D Color);
    void setBrush(const int Type, const float Radius, const QColor Color);
    void setBrush(const int Type, const float Radius, const QVector4D Color, const float Multiplier);
    void setBrush(const int Type, const float Radius, const QColor Color, const float Multiplier);

    void setRadius(const float Radius);
    void setColor(const QVector4D Color);
    void setColor(const QColor& Color);
    void setMultiplier(const float Multiplier);

    int Type() { return brush; }

    float Radius()     { return radius / multiplier; }
    float Multiplier() { return multiplier; }

    QVector4D ColorVect() { return color; }
    QColor    Color()     { return QColor::fromRgbF(color.x(), color.y(), color.z(), color.w()); }

private:
    int brush;

    float radius;
    float multiplier;

    QVector4D color;
};

#endif // BRUSH_H
