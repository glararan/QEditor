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

void Material::bind(QOpenGLShaderProgram* shader)
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

    foreach(const GLuint unit, m_FramebufferUnitConfigs.keys())
    {
        const GLuint& config = m_FramebufferUnitConfigs.value(unit);

        m_funcs->glActiveTexture(GL_TEXTURE0 + unit);

        // Bind the framebuffer
        m_funcs->glBindTexture(GL_TEXTURE_2D, config);

        // Associate with sampler uniform in shader (if we know the name or location)
        if(m_FramebufferByteUnitUniforms.contains(unit))
            shader->setUniformValue(m_FramebufferByteUnitUniforms.value(unit).constData(), unit);
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