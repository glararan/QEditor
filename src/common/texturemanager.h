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
    //bool hasTexture(QString texturePath);

    const SamplerPtr getSampler() const;
    const TexturePtr getTexture(QString textureName) const;
    const TexturePtr getTexture(QString textureName, QString texturePath) const;
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