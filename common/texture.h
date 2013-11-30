#ifndef TEXTURE_H
#define TEXTURE_H

#include <qopengl.h>
#include <QSharedPointer>

class QImage;
class QOpenGLFunctions_4_2_Core;

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

    Texture(TextureType Type = Texture2D);
    Texture(const int Width, const int Height, TextureType Type = Texture2D);
    ~Texture();

    TextureType Type() const { return type; }

    void create();
    void destroy();
    void bind();
    void release();

    GLuint textureID() const { return textureId; }

    void initializeToEmpty(const QSize& size);

    void setImage(const QImage& image);
    void setCubeMapImage(GLenum face, const QImage& image);
    void setRawData2D(GLenum target, int mipmapLevel, GLenum internalFormat,
                      int width, int height, int borderWidth,
                      GLenum format, GLenum type, const void* data);

    void updatePixel(const GLfloat value, QVector2D offset, bool bind = false);

    void setImageAlpha(void* pixelArray);
    void getImageAlpha(void* pixelArray);

    void toTexture(QSize size);

    void generateMipMaps();

private:
    TextureType type;

    GLuint textureId;

    int width;
    int height;

    QOpenGLFunctions_4_2_Core* GLfuncs;
};

typedef QSharedPointer<Texture> TexturePtr;

#endif // TEXTURE_H
