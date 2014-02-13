#include "material.h"

#include <QOpenGLFunctions_3_1>
#include <QOpenGLShaderProgram>

Material::Material()
{
    m_funcs = QOpenGLContext::currentContext()->versionFunctions<QOpenGLFunctions_3_1>();

    if(!m_funcs)
    {
        qWarning() << QObject::tr("Requires multi-texturing support");

        return;
    }

    m_funcs->initializeOpenGLFunctions();
}

Material::~Material()
{
}

void Material::bind(QOpenGLShaderProgram *shader)
{
    foreach(const GLuint unit, m_unitConfigs.keys())
    {
        const TextureUnitConfiguration& config = m_unitConfigs.value(unit);

        // Check the texture
        if(config.texture()->isNull())
            continue;

        m_funcs->glActiveTexture(GL_TEXTURE0 + unit);

        // Bind the texture
        config.texture()->bind(GL_TEXTURE0 + unit);

        // Bind the sampler
        config.sampler()->bind(unit);

        // Associate with sampler uniform in shader (if we know the name or location)
        if(m_samplerUniforms.contains(unit))
            shader->setUniformValue(m_samplerUniforms.value(unit).constData(), unit);
    }

    foreach(const GLuint unit, m_arrayUnitConfigs.keys())
    {
        const TextureArrayUnitConfiguration& config = m_arrayUnitConfigs.value(unit);

        m_funcs->glActiveTexture(GL_TEXTURE0 + unit);

        // Bind the texture array
        config.textureArray()->bind();

        // Bind the sampler
        config.sampler()->bind(unit);

        // Associate with sampler uniform in shader (if we know the name or location)
        if(m_arraySamplerUniforms.contains(unit))
            shader->setUniformValue(m_arraySamplerUniforms.value(unit).constData(), unit);
    }
}

void Material::setTextureUnitConfiguration(GLuint unit, TexturePtr texture, SamplerPtr sampler)
{
    TextureUnitConfiguration config(texture, sampler);

    m_unitConfigs.insert(unit, config);
}

void Material::setTextureUnitConfiguration(GLuint unit, TexturePtr texture, SamplerPtr sampler, const QByteArray& uniformName)
{
    setTextureUnitConfiguration(unit, texture, sampler);

    m_samplerUniforms.insert(unit, uniformName);
}

void Material::setTextureArrayUnitConfiguration(GLuint unit, TextureArrayPtr textureArray, SamplerPtr sampler)
{
    TextureArrayUnitConfiguration config(textureArray, sampler);

    m_arrayUnitConfigs.insert(unit, config);
}

void Material::setTextureArrayUnitConfiguration(GLuint unit, TextureArrayPtr textureArray, SamplerPtr sampler, const QByteArray& uniformName)
{
    setTextureArrayUnitConfiguration(unit, textureArray, sampler);

    m_arraySamplerUniforms.insert(unit, uniformName);
}

void Material::setFramebufferUnitConfiguration(GLuint unit, GLuint textureID)
{
    m_FramebufferUnitConfigs.insert(unit, textureID);
}

void Material::setFramebufferUnitConfiguration(GLuint unit, GLuint textureID, const QByteArray& uniformName)
{
    setFramebufferUnitConfiguration(unit, textureID);

    m_FramebufferByteUnitUniforms.insert(unit, uniformName);
}

TextureUnitConfiguration Material::textureUnitConfiguration(GLuint unit) const
{
    return m_unitConfigs.value(unit, TextureUnitConfiguration());
}

TextureArrayUnitConfiguration Material::textureArrayUnitConfiguration(GLuint unit) const
{
    return m_arrayUnitConfigs.value(unit, TextureArrayUnitConfiguration());
}

GLuint Material::frameBufferUnitConfiguration(GLuint unit) const
{
    return m_FramebufferUnitConfigs.value(unit, 0);
}
