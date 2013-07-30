#ifndef MATERIAL_H
#define MATERIAL_H

#include <sampler.h>
#include <texture.h>

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
    TextureUnitConfiguration textureUnitConfiguration(GLuint unit) const;

private:
    // For now we assume that we own the shader
    /** \todo Allow this to use reference to non-owned shader */
    QOpenGLShaderProgramPtr m_shader;

    // This map contains the configuration for the texture units
    QMap<GLuint, TextureUnitConfiguration> m_unitConfigs;
    QMap<GLuint, QByteArray> m_samplerUniforms;

    QOpenGLFunctions_3_1* m_funcs;
};

typedef QSharedPointer<Material> MaterialPtr;

#endif // MATERIAL_H
