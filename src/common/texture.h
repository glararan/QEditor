#ifndef TEXTURE_H
#define TEXTURE_H

#include <qopengl.h>
#include <QOpenGLTexture>
#include <QSharedPointer>

class QImage;
class QOpenGLFunctions_4_2_Core;

class Texture : public QOpenGLTexture
{
public:
    Texture();
    Texture(Target target, QString path = QString());
    Texture(const QImage& image, MipMapGeneration genMipMaps = GenerateMipMaps, QString path = QString());
    Texture(const QImage& image, QString path, MipMapGeneration genMipMaps = GenerateMipMaps);
    ~Texture();

    const QString getPath() const { return filePath; }

    const bool isNull() const      { return !isCreated(); }
    const bool isPathEmpty() const { return filePath.isEmpty(); }

    void setHeight(const GLfloat data, QVector2D offset, bool bindTexture = false);
    void setHeightmap(void* data);

    void setAlpha(const unsigned char data, QVector2D offset, bool bindTexture = false);
    void setAlphamap(void* data);

    void setVertexShade(void* data, QVector2D offset, bool bindTexture = false);
    void setVertexShading(const void* data);

    const QImage getImage();

private:
    QOpenGLFunctions_4_2_Core* GLfuncs;

    QString filePath;
};

typedef QSharedPointer<Texture> TexturePtr;

#endif // TEXTURE_H