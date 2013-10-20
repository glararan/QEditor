#include "texturearray.h"

#include "mathhelper.h"

#include <QGLWidget>
#include <QOpenGLContext>
#include <QOpenGLFunctions_4_2_Core>
#include <QDebug>

TextureArray::TextureArray(int textureWidth, int textureHeight, int layersCount, TextureType textureType)
: type(textureType)
, textureId(0)
, width(textureWidth)
, height(textureHeight)
, layers(layersCount)
, columns((int)sqrt((double)layersCount))
, GLfuncs(0)
{
    texels = new GLfloat[width * height * sizeof(float) * layers];
}

TextureArray::~TextureArray()
{
    delete[] texels;
}

void TextureArray::create()
{
    QOpenGLContext* context = QOpenGLContext::currentContext();

    Q_ASSERT(context);

    GLfuncs = context->versionFunctions<QOpenGLFunctions_4_2_Core>();
    GLfuncs->initializeOpenGLFunctions();

    GLfuncs->glGenTextures(1, &textureId);
}

void TextureArray::destroy()
{
    if(textureId)
    {
        GLfuncs->glDeleteTextures(1, &textureId);

        textureId = 0;
    }
}

void TextureArray::bind(bool initialize)
{
    GLfuncs->glBindTexture(type, textureId);

    if(initialize)
        GLfuncs->glTexStorage3D(type, 1, GL_RGBA32F, width, height, layers);
}

void TextureArray::release()
{
    GLfuncs->glBindTexture(type, 0);
}

void TextureArray::empty()
{
    Q_ASSERT(type == Texture2DArray);

    GLfuncs->glBindTexture(type, textureId);

    for(int i = 0; i < width * height * sizeof(float) * layers; ++i)
        texels[i] = 0;

    for(int i = 0; i < layers; ++i)
        GLfuncs->glTexSubImage3D(type, 0, 0, 0, 0, width, height, i, GL_RGBA, GL_FLOAT, 0);
}

void TextureArray::update()
{
    Q_ASSERT(type == Texture2DArray);

    GLfuncs->glBindTexture(type, textureId);

    for(int i = 0; i < layers; ++i)
        GLfuncs->glTexSubImage3D(type, 0, 0, 0, 0, width, height, i, GL_RGBA, GL_FLOAT, texels);
}

void TextureArray::updatePixel(const GLfloat value, QVector2D offset, bool bind)
{
    Q_ASSERT(type == Texture2DArray);

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

    GLfuncs->glTexSubImage3D(type, 0, offset.x(), offset.y(), 0, 1, 1, 1, GL_RGBA, GL_FLOAT, &value);
}

void TextureArray::updateImage(const float* pixelArray, const int layer, const int arraySize)
{
    Q_ASSERT(type == Texture2DArray);

    GLfuncs->glBindTexture(type, textureId);

    int block = width * height / layers;

    if(arraySize != block)
    {
        qFatal(QString("pixelArray is not same size as block size! pixelArray: %1 block: %2").arg(arraySize).arg(block).toStdString().c_str());

        return;
    }

    int columnW = layer / columns;
    int columnH = layer % columns;

    int columnWidth = width / columns;
    int rowsHeight  = height / columns;

    int index            = (rowsHeight * columnH * height) + (columnWidth * columnW);
    int startColumnIndex = index;

    int j = 0;

    for(int i = 0; i < block; ++i)
    {
        texels[index * sizeof(float)] = pixelArray[j];

        if(index == startColumnIndex + (block * columns / width) - 1)
            index = startColumnIndex += width;
        else
            ++index;

        ++j;
    }

    GLfuncs->glTexSubImage3D(type, 0, 0, 0, 0, width, height, layer, GL_RGBA, GL_FLOAT, texels);
}

void TextureArray::generateMipMaps()
{
    GLfuncs->glGenerateMipmap(type);
}
