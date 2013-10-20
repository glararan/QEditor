#include "texture.h"

#include <QImage>
#include <QGLWidget>
#include <QOpenGLContext>
#include <QOpenGLFunctions_4_2_Core>

#include <QDebug>

Texture::Texture(TextureType type) : type(type), textureId(0), GLfuncs(0)
{
}

Texture::~Texture()
{
}

void Texture::create()
{
    QOpenGLContext* context = QOpenGLContext::currentContext();
    Q_ASSERT(context);

    GLfuncs = context->versionFunctions<QOpenGLFunctions_4_2_Core>();
    GLfuncs->initializeOpenGLFunctions();

    GLfuncs->glGenTextures(1, &textureId);
}

void Texture::destroy()
{
    if(textureId)
    {
        GLfuncs->glDeleteTextures(1, &textureId);

        textureId = 0;
    }
}

void Texture::bind()
{
    GLfuncs->glBindTexture(type, textureId);
}

void Texture::release()
{
    GLfuncs->glBindTexture(type, 0);
}

void Texture::initializeToEmpty(const QSize& size)
{
    Q_ASSERT(size.isValid());
    Q_ASSERT(type == Texture2D);

    setRawData2D(type, 0, GL_RGBA, size.width(), size.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
}

void Texture::setImage(const QImage& image)
{
    Q_ASSERT(type == Texture2D);

    QImage glImage = QGLWidget::convertToGLFormat(image);
    setRawData2D(type, 0, GL_RGBA, glImage.width(), glImage.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, glImage.bits());
}

void Texture::setCubeMapImage(GLenum face, const QImage& image)
{
    Q_ASSERT(type == TextureCubeMap);

    QImage glImage = QGLWidget::convertToGLFormat(image);
    setRawData2D(face, 0, GL_RGBA8, glImage.width(), glImage.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, glImage.bits());
}

void Texture::setRawData2D(GLenum target, int mipmapLevel, GLenum internalFormat,
                           int width, int height, int borderWidth,
                           GLenum format, GLenum type, const void* data)
{
    GLfuncs->glTexImage2D(target, mipmapLevel, internalFormat, width, height, borderWidth, format, type, data);
}

void Texture::generateMipMaps()
{
    GLfuncs->glGenerateMipmap(type);
}
