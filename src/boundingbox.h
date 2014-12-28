#ifndef BOUNDINGBOX_H
#define BOUNDINGBOX_H

#include <QVector4D>
#include <QVector3D>
#include <QVector>
#include <QOpenGLShaderProgram>

#include "mesh.h"

class BoundingBox
{
public:
    BoundingBox(const QVector4D& bbColor = QVector4D(1.0f, 1.0f, 1.0f, 1.0f));
    BoundingBox(const QVector3D& minimum, const QVector3D& maximum, const QVector4D& bbColor = QVector4D(1.0f, 1.0f, 1.0f, 1.0f));

    void draw(QOpenGLShaderProgram* shader);
    void translate(const QVector3D& vector);

    const QVector3D& getCenter() const { return (max + min) / 2; }

private:
    QVector3D min, max;
    QVector4D color;

    Mesh mesh;

    void add(QVector<float>& data, const float& x, const float& y, const float& z);
};

#endif // BOUNDINGBOX_H
