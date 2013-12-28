#ifndef BRUSH_H
#define BRUSH_H

#include <QVector4D>
#include <QColor>

#include "material.h"

class Brush
{
public:
    enum Shapes
    {
        Circle = 0,
        Squad  = 1,
        Spray  = 2,
        ShapesCount
    };

    struct ShapingType
    {
        enum Formula
        {
            Linear        = 0,
            Flat          = 1,
            Smooth        = 2,
            Polynomial    = 3,
            Trigonometric = 4,
            Square        = 5,
            ShapingTypeCount
        };
    };

    struct SmoothingType
    {
        enum Formula
        {
            Linear = 0,
            Flat   = 1,
            Smooth = 2,
            SmoothingTypeCount
        };
    };

    struct TexturingType
    {
        enum Formula
        {
            Solid = 0,
            Spray = 1,
            TexturingTypeCount
        };
    };

    struct Types
    {
        ShapingType::Formula   shaping;
        SmoothingType::Formula smoothing;
        TexturingType::Formula texturing;

        Types(ShapingType::Formula shapingType = ShapingType::Linear, SmoothingType::Formula smoothingType = SmoothingType::Linear, TexturingType::Formula texturingType = TexturingType::Solid)
        {
            shaping   = shapingType;
            smoothing = smoothingType;
            texturing = texturingType;
        }

        void setShaping(ShapingType::Formula shapingType)
        {
            shaping = shapingType;
        }

        void setSmoothing(SmoothingType::Formula smoothingType)
        {
            smoothing = smoothingType;
        }

        void setTexturing(TexturingType::Formula texturingType)
        {
            texturing = texturingType;
        }
    };

    // # iSharpness = 0.75f
    // # oSharpness = 0.8f

    Brush(Types BrushTypes = Types(), float InnerRadius = 8.0f, float OuterRadius = 10.0f, QColor Color = QColor::fromRgbF(0.0f, 1.0f, 0.0f, 1.0f), float Multiplier = 1.0f, Shapes Shape = Circle);
    ~Brush();

    void draw(QOpenGLShaderProgramPtr shader, QVector2D terrain_pos);

    void setBrush(const Types BrushTypes);
    void setBrush(const Types BrushTypes, const float InnerRadius, const float OuterRadius);
    void setBrush(const Types BrushTypes, const float InnerRadius, const float OuterRadius, const QVector4D Color);
    void setBrush(const Types BrushTypes, const float InnerRadius, const float OuterRadius, const QColor Color);
    void setBrush(const Types BrushTypes, const float InnerRadius, const float OuterRadius, const QVector4D Color, const float Multiplier);
    void setBrush(const Types BrushTypes, const float InnerRadius, const float OuterRadius, const QColor Color, const float Multiplier);
    void setBrush(const Types BrushTypes, const float InnerRadius, const float OuterRadius, const QVector4D Color, const float Multiplier, const Shapes Shape);
    void setBrush(const Types BrushTypes, const float InnerRadius, const float OuterRadius, const QColor Color, const float Multiplier, const Shapes Shape);

    void setRadius(const float InnerRadius, const float OuterRadius);
    void setInnerRadius(const float InnerRadius);
    void setOuterRadius(const float OuterRadius);
    void setColor(const QVector4D Color);
    void setColor(const QColor& Color);
    void setMultiplier(const float Multiplier);
    void setShape(const Shapes Shape);

    float ValueAtDistance(float distance) const;
    float ValueInner(float distance) const;
    float ValueOuter(float distance) const;

    Types BrushTypes() const { return types; }

    float InnerRadius() const { return innerRadius / multiplier; }
    float OuterRadius() const { return outerRadius / multiplier; }
    float Multiplier() const  { return multiplier; }

    QVector4D ColorVect() const { return color; }
    QColor    Color() const     { return QColor::fromRgbF(color.x(), color.y(), color.z(), color.w()); }

    Shapes Shape() const { return shape; }

private:
    Types types;

    float innerRadius;
    float outerRadius;
    float multiplier;

    QVector4D color;

    Shapes shape;
};

#endif // BRUSH_H