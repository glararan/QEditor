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

    void setTexture(const void* data, int layer);

    const QImage getImage();
    const QImage getImage(int layer);

private:
    QOpenGLFunctions_4_2_Core* GLfuncs;

    QString filePath;
};

typedef QSharedPointer<Texture> TexturePtr;

#endif // TEXTURE_H
