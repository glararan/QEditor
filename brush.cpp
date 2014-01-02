#include "brush.h"

#include "mathhelper.h"

Brush::Brush(Types BrushTypes, float InnerRadius, float OuterRadius, QColor OuterColor, QColor InnerColor, float Multiplier, Shapes Shape)
{
    setBrush(BrushTypes, InnerRadius, OuterRadius, OuterColor, InnerColor, Multiplier, Shape);
}

Brush::~Brush()
{
    innerRadius = 0;
    outerRadius = 0;
    multiplier  = 0;
    outerColor  = QVector4D(0, 0, 0, 0);
    innerColor  = QVector4D(0, 0, 0, 0);
    shape       = Circle;
}

void Brush::draw(QOpenGLShaderProgramPtr shader, QVector2D terrain_pos)
{
    shader->setUniformValue("brush"                , (int)types.texturing);
    shader->setUniformValue("cursorPos"            , QVector2D(terrain_pos.x(), terrain_pos.y()));
    shader->setUniformValue("brushInnerRadius"     , innerRadius);
    shader->setUniformValue("brushOuterRadius"     , outerRadius);
    shader->setUniformValue("brushRadiusMultiplier", multiplier);
    shader->setUniformValue("outerBrushColor"      , outerColor);
    shader->setUniformValue("innerBrushColor"      , innerColor);
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

void Brush::setBrush(const Types BrushTypes, const float InnerRadius, const float OuterRadius, const QVector4D OuterColor)
{
    setBrush(BrushTypes, InnerRadius, OuterRadius);
    setOuterColor(OuterColor);
}

void Brush::setBrush(const Types BrushTypes, const float InnerRadius, const float OuterRadius, const QColor OuterColor)
{
    setBrush(BrushTypes, InnerRadius, OuterRadius);
    setOuterColor(QVector4D(OuterColor.redF(), OuterColor.greenF(), OuterColor.blueF(), OuterColor.alphaF()));
}

void Brush::setBrush(const Types BrushTypes, const float InnerRadius, const float OuterRadius, const QVector4D OuterColor, const QVector4D InnerColor)
{
    setBrush(BrushTypes, InnerRadius, OuterRadius, OuterColor);
    setInnerColor(InnerColor);
}

void Brush::setBrush(const Types BrushTypes, const float InnerRadius, const float OuterRadius, const QColor OuterColor, const QColor InnerColor)
{
    setBrush(BrushTypes, InnerRadius, OuterRadius, OuterColor);
    setInnerColor(QVector4D(InnerColor.redF(), InnerColor.greenF(), InnerColor.blueF(), InnerColor.alphaF()));
}

void Brush::setBrush(const Types BrushTypes, const float InnerRadius, const float OuterRadius, const QVector4D OuterColor, const QVector4D InnerColor, const float Multiplier)
{
    setBrush(BrushTypes, InnerRadius, OuterRadius, OuterColor, InnerColor);
    setMultiplier(Multiplier);
}

void Brush::setBrush(const Types BrushTypes, const float InnerRadius, const float OuterRadius, const QColor OuterColor, const QColor InnerColor, const float Multiplier)
{
    setBrush(BrushTypes, InnerRadius, OuterRadius, OuterColor, InnerColor);
    setMultiplier(Multiplier);
}

void Brush::setBrush(const Types BrushTypes, const float InnerRadius, const float OuterRadius, const QVector4D OuterColor, const QVector4D InnerColor, const float Multiplier, const Shapes Shape)
{
    setBrush(BrushTypes, InnerRadius, OuterRadius, OuterColor, InnerColor, Multiplier);
    setShape(Shape);
}

void Brush::setBrush(const Types BrushTypes, const float InnerRadius, const float OuterRadius, const QColor OuterColor, const QColor InnerColor, const float Multiplier, const Shapes Shape)
{
    setBrush(BrushTypes, InnerRadius, OuterRadius, OuterColor, InnerColor, Multiplier);
    setShape(Shape);
}

void Brush::setRadius(const float InnerRadius, const float OuterRadius)
{
    outerRadius = OuterRadius;

    if(outerRadius < InnerRadius)
        innerRadius = outerRadius;
    else
        innerRadius = InnerRadius;
}

void Brush::setInnerRadius(const float InnerRadius)
{
    if(outerRadius < InnerRadius)
        innerRadius = outerRadius;
    else
        innerRadius = InnerRadius;
}

void Brush::setOuterRadius(const float OuterRadius)
{
    outerRadius = OuterRadius;

    if(outerRadius < innerRadius)
        innerRadius = outerRadius;
}

void Brush::setColor(const QVector4D InnerColor, const QVector4D OuterColor)
{
    innerColor = InnerColor;
    outerColor = OuterColor;
}

void Brush::setColor(const QColor& InnerColor, const QColor& OuterColor)
{
    innerColor = QVector4D(InnerColor.redF(), InnerColor.greenF(), InnerColor.blueF(), InnerColor.alphaF());
    outerColor = QVector4D(OuterColor.redF(), OuterColor.greenF(), OuterColor.blueF(), OuterColor.alphaF());
}

void Brush::setInnerColor(const QVector4D Color)
{
    innerColor = Color;
}

void Brush::setInnerColor(const QColor& Color)
{
    innerColor = QVector4D(Color.redF(), Color.greenF(), Color.blueF(), Color.alphaF());
}

void Brush::setOuterColor(const QVector4D Color)
{
    outerColor = Color;
}

void Brush::setOuterColor(const QColor& Color)
{
    outerColor = QVector4D(Color.redF(), Color.greenF(), Color.blueF(), Color.alphaF());
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