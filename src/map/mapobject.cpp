#include "mapobject.h"

MapObject::MapObject()
: model(NULL)
, heightOffset(0.0f)
{
}

MapObject::MapObject(Model* object, const float& height)
: model(object)
, heightOffset(height)
{
}

MapObject::~MapObject()
{
    delete model;
}

void MapObject::drawBoundingBox(QOpenGLShaderProgram* shader)
{
    if(model)
    {
        //boundingBox = BoundingBox();
        //boundingBox.draw(shader);
    }
}
