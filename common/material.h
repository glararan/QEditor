#ifndef MATERIAL_H
#define MATERIAL_H

#include <sampler.h>
#include <texture.h>
#include <texturearray.h>

#include <QMap>
#include <QOpenGLShaderProgram>
#include <QPair>
#include <QSharedPointer>

typedef QSharedPointer<QOpenGLShaderProgram> QOpenGLShaderProgramPtr;

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

class QOpenGLFunctions_3_1;

class Material
{
public:
    Material();
    ~Material();

    void bind();

    void setShaders(const QString& vertexShader, const QString& fragmentShader);
    void setShaders(const QString& vertexShader, const QString& geometryShader, const QString& fragmentShader);
    void setShaders(const QString& vertexShader,
                    const QString& tessellationControlShader,
                    const QString& tessellationEvaluationShader,
                    const QString& geometryShader,
                    const QString& fragmentShader);

    void setShader(const QOpenGLShaderProgramPtr& shader);

    QOpenGLShaderProgramPtr shader() const { return m_shader; }

    void setTextureUnitConfiguration(GLuint unit, TexturePtr texture, SamplerPtr sampler);
    void setTextureUnitConfiguration(GLuint unit, TexturePtr texture, SamplerPtr sampler, const QByteArray& uniformName);

    void setTextureArrayUnitConfiguration(GLuint unit, TextureArrayPtr textureArray, SamplerPtr sampler);
    void setTextureArrayUnitConfiguration(GLuint unit, TextureArrayPtr textureArray, SamplerPtr sampler, const QByteArray& uniformName);

    TextureUnitConfiguration      textureUnitConfiguration(GLuint unit) const;
    TextureArrayUnitConfiguration textureArrayUnitConfiguration(GLuint unit) const;

private:
    // For now we assume that we own the shader
    /** \todo Allow this to use reference to non-owned shader */
    QOpenGLShaderProgramPtr m_shader;

    // This map contains the configuration for the texture units
    QMap<GLuint, TextureUnitConfiguration> m_unitConfigs;
    QMap<GLuint, QByteArray>               m_samplerUniforms;

    // This map contains the configuration for the texture array units
    QMap<GLuint, TextureArrayUnitConfiguration> m_arrayUnitConfigs;
    QMap<GLuint, QByteArray>                    m_arraySamplerUniforms;

    QOpenGLFunctions_3_1* m_funcs;
};

typedef QSharedPointer<Material> MaterialPtr;

#endif // MATERIAL_H
