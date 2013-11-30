#include "texture.h"

#include <QImage>
#include <QGLWidget>
#include <QOpenGLContext>
#include <QOpenGLFunctions_4_2_Core>
#include <QVector2D>

#include <QDebug>

Texture::Texture(TextureType type) : type(type), textureId(0), GLfuncs(0)
{
}

Texture::Texture(const int Width, const int Height, TextureType Type)
: type(Type)
, textureId(0)
, width(Width)
, height(Height)
, GLfuncs(0)
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

void Texture::updatePixel(const GLfloat value, QVector2D offset, bool bind)
{
    Q_ASSERT(type == Texture2D);

    if(bind)
        GLfuncs->glBindTexture(type, textureId);

    if(offset.x() > width)
    {
        qFatal(QString("offset is not alowed, because is larger than width! offset: %1 width: %2").arg(offset.x()).arg(width).toStdString().c_str());

        return;
    }

    if(offset.y() > height)
    {
        qFatal(QString("offset is not alowed, because is larger than height! offset: %1 height: %2").arg(offset.x()).arg(height).toStdString().c_str());

        return;
    }

    GLfuncs->glTexSubImage2D(type, 0, offset.x(), offset.y(), 1, 1, GL_RGBA, GL_FLOAT, &value);
}

void Texture::setImageAlpha(void* pixelArray)
{
    Q_ASSERT(type == Texture2D);

    setRawData2D(type, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, pixelArray);
}

void Texture::getImageAlpha(void* pixelArray)
{
    Q_ASSERT(type == Texture2D);

    GLfuncs->glGetTexImage(type, 0, GL_RGBA, GL_FLOAT, pixelArray);
}

void Texture::toTexture(QSize size)
{
    Q_ASSERT(type == Texture2D);

    GLfuncs->glCopyTexImage2D(type, 0, GL_LUMINANCE, 0, 0, size.width(), size.height(), 0);
}

void Texture::generateMipMaps()
{
    GLfuncs->glGenerateMipmap(type);
}
