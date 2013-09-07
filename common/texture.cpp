#include "texture.h"

#include <QImage>
#include <QGLWidget>
#include <QOpenGLContext>
#include <QOpenGLFunctions>

#include <QDebug>

Texture::Texture(TextureType type) : m_type(type), m_textureId(0), m_funcs(0)
{
}

Texture::~Texture()
{
}

void Texture::create()
{
    QOpenGLContext* context = QOpenGLContext::currentContext();
    Q_ASSERT(context);

    m_funcs = context->functions();
    m_funcs->initializeOpenGLFunctions();

    glGenTextures(1, &m_textureId);
}

void Texture::destroy()
{
    if(m_textureId)
    {
        glDeleteTextures(1, &m_textureId);

        m_textureId = 0;
    }
}

void Texture::bind()
{
    glBindTexture(m_type, m_textureId);
}

void Texture::release()
{
    glBindTexture(m_type, 0);
}

void Texture::initializeToEmpty(const QSize& size)
{
    Q_ASSERT(size.isValid());
    Q_ASSERT(m_type == Texture2D);

    setRawData2D(m_type, 0, GL_RGBA, size.width(), size.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
}

void Texture::setImage(const QImage& image)
{
    Q_ASSERT(m_type == Texture2D);

    QImage glImage = QGLWidget::convertToGLFormat(image);
    setRawData2D(m_type, 0, GL_RGBA, glImage.width(), glImage.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, glImage.bits());
}

void Texture::setImage(float* pixelArray, int width, int height)
{
    Q_ASSERT(m_type == Texture2D);

    setRawData2D(m_type, 0, GL_RGBA32F_ARB, width, height, 0, GL_RGBA32F_ARB, GL_FLOAT, &pixelArray[0]);
}

void Texture::setCubeMapImage(GLenum face, const QImage& image)
{
    Q_ASSERT(m_type == TextureCubeMap);

    QImage glImage = QGLWidget::convertToGLFormat(image);
    setRawData2D(face, 0, GL_RGBA8, glImage.width(), glImage.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, glImage.bits());
}

void Texture::setRawData2D(GLenum target, int mipmapLevel, GLenum internalFormat,
                           int width, int height, int borderWidth,
                           GLenum format, GLenum type, const void* data)
{
    glTexImage2D(target, mipmapLevel, internalFormat, width, height, borderWidth, format, type, data);
}

void Texture::generateMipMaps()
{
    m_funcs->glGenerateMipmap(m_type);
}
