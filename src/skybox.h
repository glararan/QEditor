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

#ifndef SKYBOX_H
#define SKYBOX_H

#include <QtCore>
#include <QtOpenGL>
#include <QOpenGLFunctions_4_2_Core>

#include "texture.h"
#include "sampler.h"
#include "material.h"
#include "mesh.h"

class Skybox
{
public:
    explicit Skybox(QOpenGLFunctions_4_2_Core* funcs);
    ~Skybox();

    enum SkyboxSide
    {
        Right  = 0,
        Left   = 1,
        Top    = 2,
        Bottom = 3,
        Front  = 4,
        Back   = 5
    };

    void draw(QOpenGLShaderProgram* shader);

    void loadSide(QOpenGLTexture::CubeMapFace side, const QString fileName);

    Material* getMaterial() { return material; }

private:
    QOpenGLFunctions_4_2_Core* GLfuncs;

    QOpenGLShaderProgram* shaderProgram;

    SamplerPtr sampler;
    TexturePtr texture;

    Material* material;

    Mesh mesh;

    void createAttributeArray(QOpenGLShaderProgram* shader);
};

#endif // SKYBOX_H
