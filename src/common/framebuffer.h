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

#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <QtCore>

class QOpenGLFunctions_4_2_Core;

class Framebuffer
{
public:
    Framebuffer(int width, int height);
    ~Framebuffer();

    void bind();
    void release();

    unsigned int texture();
    unsigned int depthTexture();

    int width();
    int height();

private:
    unsigned int textureId;
    unsigned int depthTextureId;
    unsigned int fbo;

    QSize textureSize;

private:
    QOpenGLFunctions_4_2_Core* GLfuncs;
};

#endif // FRAMEBUFFER_H