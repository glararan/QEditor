#include "texturemanager.h"

#include "world.h"

TextureManager::TextureManager(World* world, float antialiasing) : sampler(new Sampler())
{
    if(antialiasing != 16.0f && antialiasing != 8.0f && antialiasing != 4.0f && antialiasing != 2.0f && antialiasing != 1.0f)
    {
        qWarning(QString("Antialiasing with value %1 is incorrect! Setting antialiasing to 1.").arg(antialiasing).toLatin1().data());

        antialiasing = 1.0f;
    }

    sampler->create();
    sampler->setMinificationFilter(GL_LINEAR_MIPMAP_LINEAR);
    sampler->setMagnificationFilter(GL_LINEAR);

    world->getGLFunctions()->glSamplerParameterf(sampler->samplerId(), GL_TEXTURE_MAX_ANISOTROPY_EXT, antialiasing);

    sampler->setWrapMode(Sampler::DirectionS, GL_REPEAT);
    sampler->setWrapMode(Sampler::DirectionT, GL_REPEAT);
}

TextureManager::~TextureManager()
{
    sampler->destroy();

    for(int i = 0; i < textures.count(); ++i)
        textures[i].second->destroy();
}

bool TextureManager::loadTexture(QString textureName, QString texturePath)
{
    QImage textureImage(texturePath);

    TexturePtr texture(new Texture);
    texture->create();
    texture->bind();
    texture->setImage(textureImage);
    texture->generateMipMaps();

    QPair<QString, TexturePtr> pair;

    foreach(pair, textures)
    {
        if(textureName == pair.first && texture.data() == pair.second.data())
            return false;
    }

    textures.append(qMakePair<QString, TexturePtr>(textureName, texture));

    return true;
}

bool TextureManager::hasTexture(QString textureName, QString texturePath)
{
    // todo make path to texture class and check ...
    QPair<QString, TexturePtr> pair;

    foreach(pair, textures)
    {
        if(pair.first == textureName)
            return true;
    }

    return false;
}

const SamplerPtr TextureManager::getSampler() const
{
    return sampler;
}

const TexturePtr TextureManager::getTexture(QString textureName) const
{
    QPair<QString, TexturePtr> pair;

    foreach(pair, textures)
    {
        if(pair.first == textureName)
            return pair.second;
    }

    qDebug() << "We didn't find texture with name" << textureName << "returning black texture!";

    QImage textureImage(1, 1, QImage::Format_RGB32);
    textureImage.setPixel(1, 1, qRgb(0, 0, 0));

    TexturePtr texture(new Texture);
    texture->create();
    texture->bind();
    texture->setImage(textureImage);
    texture->generateMipMaps();

    return texture;
}

void TextureManager::setAntialiasing(World* world, const float antialiasing)
{
    if(antialiasing != 16.0f && antialiasing != 8.0f && antialiasing != 4.0f && antialiasing != 2.0f && antialiasing != 1.0f)
    {
        qWarning(QString("Antialiasing with value %1 is incorrect! Returning...").arg(antialiasing).toLatin1().data());

        return;
    }

    world->getGLFunctions()->glSamplerParameterf(sampler->samplerId(), GL_TEXTURE_MAX_ANISOTROPY_EXT, antialiasing);
}