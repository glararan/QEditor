#include "ilight.h"


ILight::ILight(QVector3D position, QVector3D ambient, QVector3D diffuse, QVector3D specular)
{
    ILight::position = position;
    ILight::ambient = ambient;
    ILight::diffuse = diffuse;
    ILight::specular = specular;
}
