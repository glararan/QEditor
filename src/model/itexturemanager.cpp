#include "itexturemanager.h"

#include <qopenglfunctions_3_1.h>

ITextureManager::ITextureManager()
{
    m_funcs = QOpenGLContext::currentContext()->versionFunctions<QOpenGLFunctions_3_1>();
    if(!m_funcs)
    {
        qWarning() << "Requires multi-texturing support";
        exit(-1);
    }
    m_funcs->initializeOpenGLFunctions();

    sampler.create();
    sampler.setMinificationFilter(GL_LINEAR_MIPMAP_LINEAR);
    sampler.setMagnificationFilter(GL_LINEAR);
    //m_funcs->glSamplerParameterf(sampler.samplerId(), GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0f);
    sampler.setWrapMode(Sampler::DirectionS, GL_REPEAT);
    sampler.setWrapMode(Sampler::DirectionT, GL_REPEAT);
}

ITextureManager::~ITextureManager()
{
    sampler.destroy();

    QList<Texture*> values = textures.values();

    for(int i = 0; i < values.size(); ++i)
    {
        Texture* texture = values.at(i);
        delete texture;
        texture = 0;
    }
}

bool ITextureManager::loadTexture(QString texturePath)
{
    if(hasTexture(texturePath))
        return false;

    QImage img;
    if(!img.load(texturePath))
        return false;

    Texture *texture = new Texture(img.mirrored(), texturePath);

    textures.insert(texturePath,texture);
    return true;
}

int ITextureManager::getIndex(QString texturePath)
{
    for(int i = 0; i < textures.keys().size(); ++i)
    {
        if(textures.keys().at(i) == texturePath)
            return i;
    }
    return -1;
}

bool ITextureManager::hasTexture(QString texturePath)
{
    return textures.keys().contains(texturePath);
}

Sampler *ITextureManager::getSampler()
{
    return &sampler;
}

Texture *ITextureManager::getTexture(int index)
{
    return textures.values().at(index);
}
