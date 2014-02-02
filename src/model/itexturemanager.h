#ifndef ITEXTUREMANAGER_H
#define ITEXTUREMANAGER_H

#include "common/texture.h"
#include "common/sampler.h"

#include <QImage>
#include <QGLWidget>
#include <QOpenGLContext>
#include <QOpenGLFunctions>

#include <QDebug>

class QOpenGLFunctions_3_1;

class ITextureManager
{
public:
    ITextureManager();
    ~ITextureManager();

    bool loadTexture(QString texturePath);
    int getIndex(QString texturePath);
    bool hasTexture(QString texturePath);
    Sampler *getSampler();
    Texture *getTexture(int index);

private:
    QOpenGLFunctions_3_1 *m_funcs;
    Sampler sampler;
    QMap<QString, Texture*> textures;
};

#endif // ITEXTUREMANAGER_H
