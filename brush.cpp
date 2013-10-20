#include "brush.h"

Brush::Brush(int Type, float Radius, QColor Color, float Multiplier)
{
    setBrush(Type, Radius, Color, Multiplier);
}

Brush::~Brush()
{
    brush      = 0;
    radius     = 0;
    multiplier = 0;
    color      = QVector4D(0, 0, 0, 0);
}

void Brush::draw(QOpenGLShaderProgramPtr shader, QVector2D terrain_pos)
{
    shader->setUniformValue("brush"                , brush);
    shader->setUniformValue("cursorPos"            , QVector2D(terrain_pos.x(), terrain_pos.y()));
    shader->setUniformValue("brushRadius"          , radius);
    shader->setUniformValue("brushRadiusMultiplier", multiplier);
    shader->setUniformValue("brushColor"           , color);
}

void Brush::setBrush(const int Type)
{
    brush = Type;
}

void Brush::setBrush(const int Type, const float Radius)
{
    setBrush(Type);

    radius = Radius;
}

void Brush::setBrush(const int Type, const float Radius, const QVector4D Color)
{
    setBrush(Type, Radius);

    color = Color;
}

void Brush::setBrush(const int Type, const float Radius, const QColor Color)
{
    setBrush(Type, Radius);

    color = QVector4D(Color.redF(), Color.greenF(), Color.blueF(), Color.alphaF());
}

void Brush::setBrush(const int Type, const float Radius, const QVector4D Color, const float Multiplier)
{
    setBrush(Type, Radius, Color);

    multiplier = Multiplier;
}

void Brush::setBrush(const int Type, const float Radius, const QColor Color, const float Multiplier)
{
    setBrush(Type, Radius, Color);

    multiplier = Multiplier;
}

void Brush::setRadius(const float Radius)
{
    radius = Radius;
}

void Brush::setColor(const QVector4D Color)
{
    color = Color;
}

void Brush::setColor(const QColor& Color)
{
    color = QVector4D(Color.redF(), Color.greenF(), Color.blueF(), Color.alphaF());
}

void Brush::setMultiplier(const float Multiplier)
{
    multiplier = Multiplier;
}
