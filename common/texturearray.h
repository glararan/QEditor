#ifndef TEXTUREARRAY_H
#define TEXTUREARRAY_H

#include <QtOpenGL/qgl.h>
#include <QSharedPointer>
#include <QVector2D>

class QImage;
class QOpenGLFunctions_4_2_Core;

class TextureArray
{
public:
    enum TextureType
    {
        Texture1DArray = GL_TEXTURE_1D_ARRAY,
        Texture2DArray = GL_TEXTURE_2D_ARRAY
    };

    TextureArray(int textureWidth, int textureHeight, int layersCount, TextureType textureType = Texture2DArray);
    ~TextureArray();

    TextureType Type() const { return type; }

    void create();
    void destroy();

    GLuint textureID() const { return textureId; }

    void bind(bool initialize = false);
    void release();

    void empty();

    void update();

    void updatePixel(const GLfloat value, QVector2D offset, bool bind = false);
    void updateImage(const float* pixelArray, const int layer, const int arraySize);

    void generateMipMaps();

private:
    TextureType type;

    GLuint textureId;

    QOpenGLFunctions_4_2_Core* GLfuncs;

    int width;
    int height;
    int layers;
    int columns;

    GLfloat* texels;
};

typedef QSharedPointer<TextureArray> TextureArrayPtr;

#endif // TEXTUREARRAY_H
