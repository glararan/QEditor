/*This file is part of QEditor.

QEditor is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

QEditor is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with QEditor.  If not, see <http://www.gnu.org/licenses/>.*/

#include "texturemanager.h"

#include "world.h"

TextureManager::TextureManager(World* world, float antialiasing)
: sampler(new Sampler())
, selectedTexture(new Texture())
{
    if(antialiasing != 16.0f && antialiasing != 8.0f && antialiasing != 4.0f && antialiasing != 2.0f && antialiasing != 1.0f)
    {
        qWarning(QString(QObject::tr("Antialiasing with value %1 is incorrect! Setting antialiasing to 1.")).arg(antialiasing).toLatin1().data());

        antialiasing = 1.0f;
    }

    sampler->create();
    sampler->setMinificationFilter(GL_LINEAR_MIPMAP_LINEAR);
    sampler->setMagnificationFilter(GL_LINEAR);

    world->getGLFunctions()->glSamplerParameterf(sampler->samplerId(), GL_TEXTURE_MAX_ANISOTROPY_EXT, antialiasing);

    sampler->setWrapMode(Sampler::DirectionS, GL_REPEAT);
    sampler->setWrapMode(Sampler::DirectionT, GL_REPEAT);

    blackImage = QImage(MAX_TEXTURE_SIZE, MAX_TEXTURE_SIZE, QImage::Format_RGB888);
    blackImage.fill(Qt::black);

    whiteImage = QImage(MAX_TEXTURE_SIZE, MAX_TEXTURE_SIZE, QImage::Format_RGB888);
    whiteImage.fill(Qt::white);
}

TextureManager::~TextureManager()
{
    sampler->destroy();

    for(int i = 0; i < textures.count(); ++i)
        textures[i].second->destroy();

    for(int i = 0; i < depthTextures.count(); ++i)
        depthTextures[i].second->destroy();
}

void TextureManager::loadTexture(QString textureName, QString texturePath, bool depth, QString extension)
{
    if(!QFile(texturePath).exists())
        qWarning() << texturePath << QObject::tr("is not exist!");

    QImage textureImage(texturePath);

    TexturePtr texture(new Texture(textureImage.mirrored(), texturePath));

    QPair<QString, TexturePtr> pair;

    foreach(pair, textures)
    {
        if(textureName == pair.first && texturePath == pair.second->getPath())
        {
            if(depth)
                loadDepthTexture(textureName, texturePath.replace(QFileInfo(texturePath).baseName(), QFileInfo(texturePath).baseName() + extension));

            //qDebug() << "going destroy texture!";
            //texture->destroy(); // without memory leak? Is QSharedPointer smart to delete class?

            return;
        }
    }

    textures.append(qMakePair<QString, TexturePtr>(textureName, texture));

    if(depth)
        loadDepthTexture(textureName, texturePath.replace(QFileInfo(texturePath).baseName(), QFileInfo(texturePath).baseName() + extension));
}

void TextureManager::loadDepthTexture(QString textureName, QString texturePath)
{
    if(!QFile(texturePath).exists())
        qWarning() << texturePath << QObject::tr("is not exist!");

    QImage textureImage(texturePath);

    TexturePtr texture(new Texture(textureImage.mirrored(), texturePath));

    QPair<QString, TexturePtr> pair;

    foreach(pair, depthTextures)
    {
        if(textureName == pair.first && texturePath == pair.second->getPath())
        {
            //qDebug() << "going destroy texture!";
            //texture->destroy(); // without memory leak? Is QSharedPointer smart to delete class?

            return;
        }
    }

    depthTextures.append(qMakePair<QString, TexturePtr>(textureName, texture));
}

bool TextureManager::hasTexture(QString textureName, QString texturePath)
{
    QPair<QString, TexturePtr> pair;

    foreach(pair, textures)
    {
        if(pair.first == textureName && pair.second->getPath() == texturePath)
            return true;
    }

    return false;
}

/*bool TextureManager::hasTexture(QString texturePath)
{
    QPair<QString, TexturePtr> pair;

    foreach(pair, textures)
    {
        if(pair.second->getPath() == texturePath)
            return true;
    }

    return false;
}*/

bool TextureManager::hasDepthTexture(QString textureName, QString texturePath)
{
    QPair<QString, TexturePtr> pair;

    foreach(pair, depthTextures)
    {
        if(pair.first == textureName && pair.second->getPath() == texturePath)
            return true;
    }

    return false;
}

bool TextureManager::hasDepthTexture(TexturePtr texture)
{
    QPair<QString, TexturePtr> pair;

    foreach(pair, depthTextures)
    {
        if(pair.first == QFileInfo(texture->getPath()).baseName() + "Texture")
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

    qDebug() << QObject::tr("We didn't find texture with name") << textureName << QObject::tr("returning black texture!");

    TexturePtr texture(new Texture(blackImage));

    return texture;
}

const TexturePtr TextureManager::getTexture(QString textureName, QString texturePath) const
{
    QPair<QString, TexturePtr> pair;

    foreach(pair, textures)
    {
        if(pair.first == textureName && pair.second->getPath() == texturePath)
            return pair.second;
    }

    qDebug() << QObject::tr("We didn't find texture with name") << textureName << QObject::tr("returning black texture!");

    TexturePtr texture(new Texture(blackImage));

    return texture;
}

const TexturePtr TextureManager::getTexture(const int index) const
{
    if(index >= textures.size())
        qWarning() << QObject::tr("Index is out of texture list size!");

    return textures.at(index).second;
}

const TexturePtr TextureManager::getDepthTexture(QString textureName) const
{
    QPair<QString, TexturePtr> pair;

    foreach(pair, depthTextures)
    {
        if(pair.first == textureName)
            return pair.second;
    }

    qDebug() << QObject::tr("We didn't find depth texture with name") << textureName << QObject::tr("returning white texture!");

    TexturePtr texture(new Texture(whiteImage));

    return texture;
}

const TexturePtr TextureManager::getDepthTexture(QString textureName, QString texturePath, QString extension) const
{
    QPair<QString, TexturePtr> pair;

    texturePath = texturePath.replace(QFileInfo(texturePath).baseName(), QFileInfo(texturePath).baseName() + extension);

    foreach(pair, depthTextures)
    {
        if(pair.first == textureName && pair.second->getPath() == texturePath)
            return pair.second;
    }

    qDebug() << QObject::tr("We didn't find texture with name") << textureName << QObject::tr("returning white texture!");


    TexturePtr texture(new Texture(whiteImage));

    return texture;
}

const TexturePtr TextureManager::getSelectedTexture()
{
    if(selectedTexture->isNull())
    {
        qDebug() << QObject::tr("Selected texture isn't created, using first texture in TextureManager.");

        if(textures.count() < 1)
            qFatal(QObject::tr("TextureManager doesn't have any texture, and you want to getSelectedTexture => crash!!!").toLatin1().data());

        setSelectedTexture(0);
    }

    return selectedTexture;
}

const int TextureManager::getIndex(QString texturePath) const
{
    QPair<QString, TexturePtr> pair;

    for(int i = 0; i < textures.size(); ++i)
    {
        pair = textures[i];

        if(pair.second->getPath() == texturePath)
            return i;
    }

    return -1;
}

void TextureManager::setAntialiasing(World* world, const float antialiasing)
{
    if(antialiasing != 16.0f && antialiasing != 8.0f && antialiasing != 4.0f && antialiasing != 2.0f && antialiasing != 1.0f)
    {
        qWarning(QString(QObject::tr("Antialiasing with value %1 is incorrect! Returning...")).arg(antialiasing).toLatin1().data());

        return;
    }

    world->getGLFunctions()->glSamplerParameterf(sampler->samplerId(), GL_TEXTURE_MAX_ANISOTROPY_EXT, antialiasing);
}

void TextureManager::setSelectedTexture(int index)
{
    if(index >= textures.count())
    {
        qCritical(QObject::tr("Trying to select texture that is out of range in Texture Manager!").toLatin1().data());

        return;
    }

    selectedTexture = textures.at(index).second;
}

void TextureManager::setSelectedTexture(QString path)
{
    for(int i = 0; i < textures.count(); ++i)
    {
        if(textures.at(i).second->getPath() == path)
        {
            selectedTexture = textures.at(i).second;

            break;
        }
    }
}
