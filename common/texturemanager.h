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

    void loadTexture(QString textureName, QString texturePath);

    bool hasTexture(QString textureName, QString texturePath);

    const SamplerPtr getSampler() const;
    const TexturePtr getTexture(QString textureName) const;
    const TexturePtr getSelectedTexture();

    const QVector<QPair<QString, TexturePtr>> getTextures() const { return textures; }

    void setAntialiasing(World* world, const float antialiasing);
    void setSelectedTexture(int index);

private:
    SamplerPtr sampler;

    QVector<QPair<QString, TexturePtr>> textures;

    TexturePtr selectedTexture;
};

#endif // TEXTUREMANAGER_H
