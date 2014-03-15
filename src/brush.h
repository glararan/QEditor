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
        TexturingType::Formula vertexShading;

        Types(ShapingType::Formula shapingType = ShapingType::Linear, SmoothingType::Formula smoothingType = SmoothingType::Linear,
              TexturingType::Formula texturingType = TexturingType::Solid, TexturingType::Formula vertexShadingType = TexturingType::Solid)
        {
            shaping       = shapingType;
            smoothing     = smoothingType;
            texturing     = texturingType;
            vertexShading = vertexShadingType;
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

        void setVertexShading(TexturingType::Formula vertexShadingType)
        {
            vertexShading = vertexShadingType;
        }
    };

    // # iSharpness = 0.75f
    // # oSharpness = 0.8f

    Brush(Types BrushTypes = Types(), float InnerRadius = 8.0f, float OuterRadius = 10.0f, QColor OuterColor = QColor::fromRgbF(0.0f, 1.0f, 0.0f, 1.0f),
          QColor InnerColor = QColor::fromRgbF(0.0f, 1.0f, 0.0f, 1.0f), float Multiplier = 1.0f, Shapes Shape = Circle);
    ~Brush();

    void draw(QOpenGLShaderProgram* shader, QVector2D terrain_pos);

    float calcLossyMultiplier(const float dist) const;

    void setBrush(const Types BrushTypes);
    void setBrush(const Types BrushTypes, const float InnerRadius, const float OuterRadius);
    void setBrush(const Types BrushTypes, const float InnerRadius, const float OuterRadius, const QVector4D OuterColor);
    void setBrush(const Types BrushTypes, const float InnerRadius, const float OuterRadius, const QColor OuterColor);
    void setBrush(const Types BrushTypes, const float InnerRadius, const float OuterRadius, const QVector4D OuterColor, const QVector4D InnerColor);
    void setBrush(const Types BrushTypes, const float InnerRadius, const float OuterRadius, const QColor OuterColor, const QColor InnerColor);
    void setBrush(const Types BrushTypes, const float InnerRadius, const float OuterRadius, const QVector4D OuterColor, const QVector4D InnerColor, const float Multiplier);
    void setBrush(const Types BrushTypes, const float InnerRadius, const float OuterRadius, const QColor OuterColor, const QColor InnerColor, const float Multiplier);
    void setBrush(const Types BrushTypes, const float InnerRadius, const float OuterRadius, const QVector4D OuterColor, const QVector4D InnerColor, const float Multiplier, const Shapes Shape);
    void setBrush(const Types BrushTypes, const float InnerRadius, const float OuterRadius, const QColor OuterColor, const QColor InnerColor, const float Multiplier, const Shapes Shape);

    void setRadius(const float InnerRadius, const float OuterRadius);
    void setInnerRadius(const float InnerRadius);
    void setOuterRadius(const float OuterRadius);
    void setColor(const QVector4D InnerColor, const QVector4D OuterColor);
    void setColor(const QColor& InnerColor, const QColor& OuterColor);
    void setInnerColor(const QVector4D Color);
    void setInnerColor(const QColor& Color);
    void setOuterColor(const QVector4D Color);
    void setOuterColor(const QColor& Color);
    void setMultiplier(const float Multiplier);
    void setShape(const Shapes Shape);

    float ValueAtDistance(float distance) const;
    float ValueInner(float distance) const;
    float ValueOuter(float distance) const;

    Types BrushTypes() const { return types; }

    float InnerRadius() const { return innerRadius / multiplier; }
    float OuterRadius() const { return outerRadius / multiplier; }
    float FallOff() const     { return innerRadius / outerRadius; }
    float Multiplier() const  { return multiplier; }

    QVector4D InnerColorVect() const { return innerColor; }
    QColor    InnerColor() const     { return QColor::fromRgbF(innerColor.x(), innerColor.y(), innerColor.z(), innerColor.w()); }

    QVector4D OuterColorVect() const { return outerColor; }
    QColor    OuterColor() const     { return QColor::fromRgbF(outerColor.x(), outerColor.y(), outerColor.z(), outerColor.w()); }

    Shapes Shape() const { return shape; }

private:
    Types types;

    float innerRadius;
    float outerRadius;
    float multiplier;

    QVector4D innerColor;
    QVector4D outerColor;

    Shapes shape;
};

#endif // BRUSH_H
