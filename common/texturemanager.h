#ifndef TEXTUREMANAGER_H
#define TEXTUREMANAGER_H

#include <QPair>
#include <QVector>

#include "sampler.h"
#include "texture.h"

class World;

class TextureManager
{
public:
    TextureManager(World* world, float antialiasing = 1.0f);
    ~TextureManager();

    bool loadTexture(QString textureName, QString texturePath);
    bool hasTexture(QString textureName, QString texturePath);

    const SamplerPtr getSampler() const;
    const TexturePtr getTexture(QString textureName) const;

    void setAntialiasing(World* world, const float antialiasing);

private:
    SamplerPtr sampler;

    QVector<QPair<QString, TexturePtr>> textures;
};

#endif // TEXTUREMANAGER_H
