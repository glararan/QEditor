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

#include "mapobject.h"

MapObject::MapObject()
: model(NULL)
, heightOffset(0.0f)
, boundingBox(NULL)
{
}

MapObject::MapObject(Model* object, const float& height)
: model(object)
, heightOffset(height)
, boundingBox(new BoundingBox())
{
}

MapObject::~MapObject()
{
    delete boundingBox;
}

void MapObject::drawBoundingBox(QOpenGLShaderProgram* shader, const QMatrix4x4& view, const QMatrix4x4& proj)
{
    if(model && boundingBox)
        boundingBox->draw(shader, view, proj);
}

void MapObject::updateBoundingBox()
{
    boundingBox->translate(translate);
    boundingBox->scale(scale);
    boundingBox->rotation(rotation);

    if(!boundingBox->isInitialized())
        boundingBox->initialize(model->getModelInterface()->getHeader().boundingBoxMin, model->getModelInterface()->getHeader().boundingBoxMax);
}

void MapObject::setTranslate(const QVector3D& translation, const bool& update)
{
    translate = translation;

    if(update)
        boundingBox->translate(translate);
}

void MapObject::setRotation(const QVector3D& rotationVec, const bool& update)
{
    rotation = rotationVec;

    if(update)
        boundingBox->rotation(rotationVec);
}

void MapObject::setScale(const QVector3D& scaleVec, const bool& update)
{
    scale = scaleVec;

    if(update)
        boundingBox->scale(scale);
}

const QVector3D MapObject::getBoundingBoxMin() const
{
    QMatrix4x4 m;

    QVector3D min = model->getModelInterface()->getHeader().boundingBoxMin;
    QVector3D max = model->getModelInterface()->getHeader().boundingBoxMax;

    m.rotate(rotation.x() * 360.0f, 1.0f, 0.0f, 0.0f);
    m.rotate(rotation.y() * 360.0f, 0.0f, 1.0f, 0.0f);
    m.rotate(rotation.z() * 360.0f, 0.0f, 0.0f, 1.0f);
    m.scale(scale.x(), scale.y(), scale.z());

    min =  m * min;
    min += translate;

    max =  m * max;
    max += translate;

    QVector3D minimum;

    if(min.x() > max.x())
        minimum.setX(max.x());
    else
        minimum.setX(min.x());

    if(min.y() > max.y())
        minimum.setY(max.y());
    else
        minimum.setY(min.y());

    if(min.z() > max.z())
        minimum.setZ(max.z());
    else
        minimum.setZ(min.z());

    return minimum;
}

const QVector3D MapObject::getBoundingBoxMax() const
{
    QMatrix4x4 m;

    QVector3D min = model->getModelInterface()->getHeader().boundingBoxMin;
    QVector3D max = model->getModelInterface()->getHeader().boundingBoxMax;

    m.rotate(rotation.x() * 360.0f, 1.0f, 0.0f, 0.0f);
    m.rotate(rotation.y() * 360.0f, 0.0f, 1.0f, 0.0f);
    m.rotate(rotation.z() * 360.0f, 0.0f, 0.0f, 1.0f);
    m.scale(scale.x(), scale.y(), scale.z());

    min =  m * min;
    min += translate;

    max =  m * max;
    max += translate;

    QVector3D maximum;

    if(min.x() > max.x())
        maximum.setX(min.x());
    else
        maximum.setX(max.x());

    if(min.y() > max.y())
        maximum.setY(min.y());
    else
        maximum.setY(max.y());

    if(min.z() > max.z())
        maximum.setZ(min.z());
    else
        maximum.setZ(max.z());

    return maximum;
}
