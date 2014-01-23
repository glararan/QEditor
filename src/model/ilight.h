#ifndef ILIGHT_H
#define ILIGHT_H

#include <QtCore>
#include <QtWidgets>

struct ILight
{
    ILight(QVector3D position = QVector3D(0.0f,0.0f,0.0f), QVector3D ambient = QVector3D(0.3f,0.3f,0.3f), QVector3D diffuse = QVector3D(0.6f,0.6f,0.6f), QVector3D specular = QVector3D(1.0f,1.0f,1.0f));

    QVector3D position;
    QVector3D ambient;
    QVector3D diffuse;
    QVector3D specular;
};

#endif // ILIGHT_H
