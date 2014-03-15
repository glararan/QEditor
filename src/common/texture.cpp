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

#include "texture.h"

#include "qeditor.h"

#include <QImage>
#include <QGLWidget>
#include <QOpenGLContext>
#include <QOpenGLFunctions_4_2_Core>
#include <QVector2D>

Texture::Texture()
: QOpenGLTexture(QOpenGLTexture::Target1D)
{
    QOpenGLContext* context = QOpenGLContext::currentContext();

    Q_ASSERT(context);

    GLfuncs = context->versionFunctions<QOpenGLFunctions_4_2_Core>();
    GLfuncs->initializeOpenGLFunctions();
}

Texture::Texture(Target target, QString path)
: QOpenGLTexture(target)
, filePath(path)
{
    QOpenGLContext* context = QOpenGLContext::currentContext();

    Q_ASSERT(context);

    GLfuncs = context->versionFunctions<QOpenGLFunctions_4_2_Core>();
    GLfuncs->initializeOpenGLFunctions();

    create();
    bind();
}

Texture::Texture(const QImage& image, MipMapGeneration genMipMaps, QString path)
: QOpenGLTexture(image, genMipMaps)
, filePath(path)
{
    QOpenGLContext* context = QOpenGLContext::currentContext();

    Q_ASSERT(context);

    GLfuncs = context->versionFunctions<QOpenGLFunctions_4_2_Core>();
    GLfuncs->initializeOpenGLFunctions();

    if(app().getGraphics() == AMD_VENDOR)
    {
        bind();

        QImage glImage = QGLWidget::convertToGLFormat(image);

        GLfuncs->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, glImage.width(), glImage.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, glImage.bits());

        if(genMipMaps == GenerateMipMaps)
            generateMipMaps();
    }
}

Texture::Texture(const QImage& image, QString path, MipMapGeneration genMipMaps)
: QOpenGLTexture(image, genMipMaps)
, filePath(path)
{
    QOpenGLContext* context = QOpenGLContext::currentContext();

    Q_ASSERT(context);

    GLfuncs = context->versionFunctions<QOpenGLFunctions_4_2_Core>();
    GLfuncs->initializeOpenGLFunctions();

    if(app().getGraphics() == AMD_VENDOR)
    {
        bind();

        QImage glImage = QGLWidget::convertToGLFormat(image);

        GLfuncs->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, glImage.width(), glImage.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, glImage.bits());

        if(genMipMaps == GenerateMipMaps)
            generateMipMaps();
    }
}

Texture::~Texture()
{
}

void Texture::setHeight(const GLfloat data, QVector2D offset, bool bindTexture)
{
    if(bindTexture)
        bind();

    GLfuncs->glTexSubImage2D(GL_TEXTURE_2D, 0, offset.x(), offset.y(), 1, 1, GL_RED, GL_FLOAT, &data);
}

void Texture::setHeightmap(void* data)
{
    GLfuncs->glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width(), height(), 0, GL_RED, GL_FLOAT, data);
}

void Texture::setAlpha(const unsigned char data, QVector2D offset, bool bindTexture)
{
    if(bindTexture)
        bind();

    GLfuncs->glTexSubImage2D(GL_TEXTURE_2D, 0, offset.x(), offset.y(), 1, 1, GL_RED, GL_UNSIGNED_BYTE, &data);
}

void Texture::setAlphamap(void* data)
{
    GLfuncs->glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width(), height(), 0, GL_RED, GL_UNSIGNED_BYTE, data);
}

void Texture::setVertexShade(void* data, QVector2D offset, bool bindTexture)
{
    if(bindTexture)
        bind();

    GLfuncs->glTexSubImage2D(GL_TEXTURE_2D, 0, offset.x(), offset.y(), 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);
}

void Texture::setVertexShading(const void* data)
{
    GLfuncs->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width(), height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
}

const QImage Texture::getImage()
{
    int width, height;

    bind();

    GLfuncs->glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH,  &width);
    GLfuncs->glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);

    if(width <= 0 || height <= 0)
        return QImage();

    GLint bytes = width * height * 4;

    unsigned char* data = (unsigned char*)malloc(bytes);

    GLfuncs->glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    QImage img = QImage(data, width, height, QImage::Format_RGBA8888);

    //free(data); // ! investigate memory leaks if this is commented => uncommented => crash cause QImage using data as pointer

    return img;
}