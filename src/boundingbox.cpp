#include "boundingbox.h"

BoundingBox::BoundingBox(const QVector4D& bbColor) : color(bbColor)
{
}

BoundingBox::BoundingBox(const QVector3D& minimum, const QVector3D& maximum, const QVector4D& bbColor)
: min(minimum)
, max(maximum)
, color(bbColor)
{
    QVector<float> positionData(16 * 3); // 16 points * 3 vertex per point

    add(positionData, min.x(), max.y(), min.z());
    add(positionData, min.x(), min.y(), min.z());
    add(positionData, max.x(), min.y(), min.z());
    add(positionData, max.x(), min.y(), max.z());
    add(positionData, max.x(), max.y(), max.z());
    add(positionData, max.x(), max.y(), min.z());
    add(positionData, min.x(), max.y(), min.z());
    add(positionData, min.x(), max.y(), max.z());
    add(positionData, min.x(), min.y(), max.z());
    add(positionData, min.x(), min.y(), min.z());
    add(positionData, min.x(), min.y(), max.z()); //
    add(positionData, max.x(), min.y(), max.z());
    add(positionData, max.x(), max.y(), min.z()); //
    add(positionData, max.x(), min.y(), min.z());
    add(positionData, min.x(), max.y(), max.z()); //
    add(positionData, max.x(), max.y(), max.z());

    mesh.createVertexArrayObject();
    mesh.createBuffer(Mesh::Vertices, positionData.data(), positionData.size() * sizeof(float));
    mesh.setNumFaces(16);
}

void BoundingBox::draw(QOpenGLShaderProgram* shader)
{
    if(min == max)
        return;

    shader->bind();
    shader->setUniformValue("color", color);

    mesh.bind();
    mesh.createAttributeArray(Mesh::Vertices, shader, "position", GL_FLOAT, 0, 3);

    glLineWidth(1.0f);
    glDrawArrays(GL_LINE, 0, mesh.getNumFaces());
}

void BoundingBox::translate(const QVector3D& vector)
{
    min += vector;
    max += vector;
}

void BoundingBox::add(QVector<float>& data, const float& x, const float& y, const float& z)
{
    data.append(x);
    data.append(y);
    data.append(z);
}
