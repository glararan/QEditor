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

    void drawBoundingBox(QOpenGLShaderProgram* shader);

    void setTranslate(const QVector3D& translation) { translate = translation; }
    void setRotation(const QVector3D& rotationVec)  { rotation = rotationVec; }
    void setScale(const QVector3D& scaleVec)        { scale = scaleVec; }

    Model* getModel() const { return model; }

    const float& getHeightOffset() const { return heightOffset; }

    const QVector3D& getTranslate() const { return translate; }
    const QVector3D& getRotation() const  { return rotation; }
    const QVector3D& getScale() const     { return scale; }

private:
    Model* model;

    float heightOffset;

    QVector3D translate;
    QVector3D rotation;
    QVector3D scale;

    //BoundingBox boundingBox;
};

#endif // MAPOBJECT_H
