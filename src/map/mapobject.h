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

#ifndef MAPOBJECT_H
#define MAPOBJECT_H

#include <QVector3D>
#include <QOpenGLShaderProgram>

#include "model.h"
#include "boundingbox.h"

class MapObject
{
public:
    MapObject();
    MapObject(Model* object, const float& height = 0.0f);
    ~MapObject();

    void drawBoundingBox(QOpenGLShaderProgram* shader, const QMatrix4x4& view, const QMatrix4x4& proj);
    void updateBoundingBox();

    void setTranslate(const QVector3D& translation, const bool& update = false);
    void setRotation(const QVector3D& rotationVec, const bool& update = false);
    void setScale(const QVector3D& scaleVec, const bool& update = false);

    Model* getModel() const { return model; }

    const float& getHeightOffset() const { return heightOffset; }

    const QVector3D& getTranslate() const { return translate; }
    const QVector3D& getRotation() const  { return rotation; }
    const QVector3D& getScale() const     { return scale; }

    // reference wont work :/
    const QVector3D getBoundingBoxMin() const;
    const QVector3D getBoundingBoxMax() const;

    const QVector3D getCenter() const { return model->getModelInterface()->getHeader().getCenter() + translate; }

private:
    Model* model;

    float heightOffset;

    QVector3D translate;
    QVector3D rotation;
    QVector3D scale;

    BoundingBox* boundingBox;
};

#endif // MAPOBJECT_H
