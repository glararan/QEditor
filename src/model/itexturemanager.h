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
