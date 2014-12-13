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

#include "framebuffer.h"

#include <QGLWidget>
#include <QOpenGLContext>
#include <QOpenGLFunctions_4_2_Core>

Framebuffer::Framebuffer(int width, int height)
: textureSize(width, height)
{
    QOpenGLContext* context = QOpenGLContext::currentContext();

    Q_ASSERT(context);

    GLfuncs = context->versionFunctions<QOpenGLFunctions_4_2_Core>();
    GLfuncs->initializeOpenGLFunctions();

    GLfuncs->glGenTextures(1, &depthTextureId);
    GLfuncs->glBindTexture(GL_TEXTURE_CUBE_MAP, depthTextureId);

    GLfuncs->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    GLfuncs->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    GLfuncs->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    GLfuncs->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    GLfuncs->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    GLfuncs->glTexImage2D(GL_TEXTURE_CUBE_MAP, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

    GLfuncs->glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    GLfuncs->glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    GLfuncs->glGenTextures(1, &textureId);
    GLfuncs->glBindTexture(GL_TEXTURE_CUBE_MAP, textureId);

    for(GLuint face = 0; face < 6; ++face)
        GLfuncs->glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, NULL);

    GLfuncs->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);;
    GLfuncs->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    GLfuncs->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    GLfuncs->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    GLfuncs->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    GLfuncs->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
    GLfuncs->glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, 0);

    GLfuncs->glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    GLfuncs->glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    GLfuncs->glGenFramebuffers(1, &fbo);
    GLfuncs->glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    for(GLuint face = 0; face < 6; ++face)
        GLfuncs->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, textureId, 0);

    GLfuncs->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTextureId, 0);

    int i = GLfuncs->glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if(i != GL_FRAMEBUFFER_COMPLETE)
        qDebug() << "Framebuffer is not OK, status=" << i;

    GLfuncs->glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

Framebuffer::~Framebuffer()
{
    GLfuncs->glDeleteTextures(1, &textureId);
    GLfuncs->glDeleteTextures(1, &depthTextureId);

    GLfuncs->glBindFramebuffer(GL_FRAMEBUFFER, 0);
    GLfuncs->glDeleteFramebuffers(1, &fbo);
}

void Framebuffer::bind()
{
    GLfuncs->glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    GLfuncs->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP, textureId, 0);
    GLfuncs->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP, depthTextureId, 0);
}

void Framebuffer::release()
{
    GLfuncs->glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

unsigned int Framebuffer::texture()
{
    return textureId;
}

unsigned int Framebuffer::depthTexture()
{
    return depthTextureId;
}

int Framebuffer::width()
{
    return textureSize.width();
}

int Framebuffer::height()
{
    return textureSize.height();
}
