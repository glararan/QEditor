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

#ifndef MATERIAL_H
#define MATERIAL_H

#include <sampler.h>
#include <texture.h>
#include <texturearray.h>

#include <QMap>
#include <QOpenGLShaderProgram>
#include <QOpenGLFramebufferObject>
#include <QPair>
#include <QSharedPointer>

typedef QSharedPointer<QOpenGLShaderProgram>     QOpenGLShaderProgramPtr;
typedef QSharedPointer<QOpenGLFramebufferObject> FrameBufferPtr;

class TextureUnitConfiguration : public QPair<TexturePtr, SamplerPtr>
{
public:
    TextureUnitConfiguration() : QPair<TexturePtr, SamplerPtr>(TexturePtr(), SamplerPtr())
    {
    }

    explicit TextureUnitConfiguration(const TexturePtr& texture, const SamplerPtr& sampler) : QPair<TexturePtr, SamplerPtr>(texture, sampler)
    {
    }

    void setTexture(const TexturePtr& texture) { first = texture; }
    TexturePtr texture() const                 { return first; }

    void setSampler(const SamplerPtr sampler) { second = sampler; }
    SamplerPtr sampler() const                { return second; }
};

class TextureArrayUnitConfiguration : public QPair<TextureArrayPtr, SamplerPtr>
{
public:
    TextureArrayUnitConfiguration() : QPair<TextureArrayPtr, SamplerPtr>(TextureArrayPtr(), SamplerPtr())
    {
    }

    explicit TextureArrayUnitConfiguration(const TextureArrayPtr& textureArray, const SamplerPtr& sampler) : QPair<TextureArrayPtr, SamplerPtr>(textureArray, sampler)
    {
    }

    void setTextureArray(const TextureArrayPtr& textureArray) { first = textureArray; }
    TextureArrayPtr textureArray() const                      { return first; }

    void setSampler(const SamplerPtr sampler) { second = sampler; }
    SamplerPtr sampler() const                { return second; }
};

class QOpenGLFunctions_4_2_Core;

class Material
{
public:
    Material();
    ~Material();

    void bind(QOpenGLShaderProgram *shader);

    void setTextureUnitConfiguration(GLuint unit, TexturePtr texture, SamplerPtr sampler);
    void setTextureUnitConfiguration(GLuint unit, TexturePtr texture, SamplerPtr sampler, const QByteArray& uniformName);

    void setTextureArrayUnitConfiguration(GLuint unit, TextureArrayPtr textureArray, SamplerPtr sampler);
    void setTextureArrayUnitConfiguration(GLuint unit, TextureArrayPtr textureArray, SamplerPtr sampler, const QByteArray& uniformName);

    void setFramebufferUnitConfiguration(GLuint unit, GLuint textureID);
    void setFramebufferUnitConfiguration(GLuint unit, GLuint textureID, const QByteArray& uniformName);

    TextureUnitConfiguration      textureUnitConfiguration(GLuint unit) const;
    TextureArrayUnitConfiguration textureArrayUnitConfiguration(GLuint unit) const;

    GLuint frameBufferUnitConfiguration(GLuint unit) const;

private:

    // This map contains the configuration for the texture units
    QMap<GLuint, TextureUnitConfiguration> m_unitConfigs;
    QMap<GLuint, QByteArray>               m_samplerUniforms;

    // This map contains the configuration for the texture array units
    QMap<GLuint, TextureArrayUnitConfiguration> m_arrayUnitConfigs;
    QMap<GLuint, QByteArray>                    m_arraySamplerUniforms;

    // This map contains the configuration for the framebuffer units
    QMap<GLuint, GLuint>     m_FramebufferUnitConfigs;
    QMap<GLuint, QByteArray> m_FramebufferByteUnitUniforms;

    QOpenGLFunctions_4_2_Core* m_funcs;
};

typedef QSharedPointer<Material> MaterialPtr;

#endif // MATERIAL_H
