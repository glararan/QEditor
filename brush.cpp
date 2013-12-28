#include "brush.h"

#include "mathhelper.h"

Brush::Brush(Types BrushTypes, float InnerRadius, float OuterRadius, QColor Color, float Multiplier, Shapes Shape)
{
    setBrush(BrushTypes, InnerRadius, OuterRadius, Color, Multiplier, Shape);
}

Brush::~Brush()
{
    innerRadius = 0;
    outerRadius = 0;
    multiplier  = 0;
    color       = QVector4D(0, 0, 0, 0);
    shape       = Circle;
}

void Brush::draw(QOpenGLShaderProgramPtr shader, QVector2D terrain_pos)
{
    shader->setUniformValue("brush"                , (int)types.texturing);
    shader->setUniformValue("cursorPos"            , QVector2D(terrain_pos.x(), terrain_pos.y()));
    shader->setUniformValue("brushInnerRadius"     , innerRadius);
    shader->setUniformValue("brushOuterRadius"     , outerRadius);
    shader->setUniformValue("brushRadiusMultiplier", multiplier);
    shader->setUniformValue("brushColor"           , color);
}

void Brush::setBrush(const Types BrushTypes)
{
    types = BrushTypes;
}

void Brush::setBrush(const Types BrushTypes, const float InnerRadius, const float OuterRadius)
{
    setBrush(BrushTypes);
    setRadius(InnerRadius, OuterRadius);
}

void Brush::setBrush(const Types BrushTypes, const float InnerRadius, const float OuterRadius, const QVector4D Color)
{
    setBrush(BrushTypes, InnerRadius, OuterRadius);
    setColor(Color);
}

void Brush::setBrush(const Types BrushTypes, const float InnerRadius, const float OuterRadius, const QColor Color)
{
    setBrush(BrushTypes, InnerRadius, OuterRadius);
    setColor(QVector4D(Color.redF(), Color.greenF(), Color.blueF(), Color.alphaF()));
}

void Brush::setBrush(const Types BrushTypes, const float InnerRadius, const float OuterRadius, const QVector4D Color, const float Multiplier)
{
    setBrush(BrushTypes, InnerRadius, OuterRadius, Color);
    setMultiplier(Multiplier);
}

void Brush::setBrush(const Types BrushTypes, const float InnerRadius, const float OuterRadius, const QColor Color, const float Multiplier)
{
    setBrush(BrushTypes, InnerRadius, OuterRadius, Color);
    setMultiplier(Multiplier);
}

void Brush::setBrush(const Types BrushTypes, const float InnerRadius, const float OuterRadius, const QVector4D Color, const float Multiplier, const Shapes Shape)
{
    setBrush(BrushTypes, InnerRadius, OuterRadius, Color, Multiplier);
    setShape(Shape);
}

void Brush::setBrush(const Types BrushTypes, const float InnerRadius, const float OuterRadius, const QColor Color, const float Multiplier, const Shapes Shape)
{
    setBrush(BrushTypes, InnerRadius, OuterRadius, Color, Multiplier);
    setShape(Shape);
}

void Brush::setRadius(const float InnerRadius, const float OuterRadius)
{
    innerRadius = InnerRadius;
    outerRadius = OuterRadius;
}

void Brush::setInnerRadius(const float InnerRadius)
{
    innerRadius = InnerRadius;
}

void Brush::setOuterRadius(const float OuterRadius)
{
    outerRadius = OuterRadius;
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

void Brush::setShape(const Shapes Shape)
{
    shape = Shape;
}

float Brush::ValueAtDistance(float distance) const
{
    if(distance > outerRadius)
        return 0.0f;

    if(distance <= innerRadius)
        return ValueInner(distance);

    return ValueOuter(distance);
}

float Brush::ValueInner(float distance) const
{
    return MathHelper::lerp(1.0f, 0.9f, distance / innerRadius);
}

float Brush::ValueOuter(float distance) const
{
    float u = (distance - innerRadius) / (outerRadius - innerRadius);

    if(u < 0.9f)
        return 0.9f;

    return MathHelper::lerp(0.9f, 0, (u - 0.95f) / (1.0f - 0.95f));
}