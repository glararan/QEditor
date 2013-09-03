#ifndef TEXTURE_H
#define TEXTURE_H

#include <qopengl.h>
#include <QSharedPointer>

class QImage;
class QOpenGLFunctions;

class Texture
{
public:
    enum TextureType
    {
        Texture1D      = GL_TEXTURE_1D,
        Texture2D      = GL_TEXTURE_2D,
        Texture3D      = GL_TEXTURE_3D,
        TextureCubeMap = GL_TEXTURE_CUBE_MAP
    };

    Texture(TextureType type = Texture2D);
    ~Texture();

    TextureType type() const { return m_type; }

    void create();
    void destroy();
    GLuint textureId() const { return m_textureId; }
    void bind();
    void release();

    void initializeToEmpty(const QSize& size);

    void setImage(const QImage& image);
    void setImage(float* pixelArray, int width, int height);
    void setCubeMapImage(GLenum face, const QImage& image);
    void setRawData2D(GLenum target, int mipmapLevel, GLenum internalFormat,
                      int width, int height, int borderWidth,
                      GLenum format, GLenum type, const void* data);

    void generateMipMaps();

private:
    TextureType m_type;
    GLuint m_textureId;
    QOpenGLFunctions* m_funcs;
};

typedef QSharedPointer<Texture> TexturePtr;

#endif // TEXTURE_H
