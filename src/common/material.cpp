#include "material.h"

#include <QOpenGLFunctions_3_1>
#include <QOpenGLShaderProgram>

Material::Material() : m_shader(new QOpenGLShaderProgram)
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
    m_shader->release();
}

void Material::bind()
{
    m_shader->bind();

    foreach(const GLuint unit, m_unitConfigs.keys())
    {
        const TextureUnitConfiguration& config = m_unitConfigs.value(unit);

        // Check the texture
        if(config.texture()->isNull())
            continue;

        // Bind the texture
        config.texture()->bind(GL_TEXTURE0 + unit);

        // Bind the sampler
        config.sampler()->bind(unit);

        // Associate with sampler uniform in shader (if we know the name or location)
        if(m_samplerUniforms.contains(unit))
            m_shader->setUniformValue(m_samplerUniforms.value(unit).constData(), unit);
    }

    m_funcs->glActiveTexture(GL_TEXTURE0);

    foreach(const GLuint unit, m_arrayUnitConfigs.keys())
    {
        const TextureArrayUnitConfiguration& config = m_arrayUnitConfigs.value(unit);

        // Bind the texture array
        config.textureArray()->bind();

        // Bind the sampler
        config.sampler()->bind(unit);

        // Associate with sampler uniform in shader (if we know the name or location)
        if(m_arraySamplerUniforms.contains(unit))
            m_shader->setUniformValue(m_arraySamplerUniforms.value(unit).constData(), unit);
    }
}

void Material::setShaders(const QString& vertexShader, const QString& fragmentShader)
{
    // Create a shader program
    if(!m_shader->addShaderFromSourceFile(QOpenGLShader::Vertex, vertexShader))
        qCritical() << QObject::tr("Could not compile vertex shader. Log:") << m_shader->log();

    if(!m_shader->addShaderFromSourceFile(QOpenGLShader::Fragment, fragmentShader))
        qCritical() << QObject::tr("Could not compile fragment shader. Log:") << m_shader->log();

    if(!m_shader->link())
        qCritical() << QObject::tr("Could not link shader program. Log:") << m_shader->log();
}

void Material::setShaders(const QString& vertexShader, const QString& geometryShader, const QString& fragmentShader)
{
    // Create a shader program
    if(!m_shader->addShaderFromSourceFile(QOpenGLShader::Vertex, vertexShader))
        qCritical() << QObject::tr("Could not compile vertex shader. Log:") << m_shader->log();

    if(!m_shader->addShaderFromSourceFile(QOpenGLShader::Geometry, geometryShader))
        qCritical() << QObject::tr("Could not compile geometry shader. Log:") << m_shader->log();

    if(!m_shader->addShaderFromSourceFile(QOpenGLShader::Fragment, fragmentShader))
        qCritical() << QObject::tr("Could not compile fragment shader. Log:") << m_shader->log();

    if(!m_shader->link())
        qCritical() << QObject::tr("Could not link shader program. Log:") << m_shader->log();
}

void Material::setShaders(const QString& vertexShader,
                          const QString& tessellationControlShader,
                          const QString& tessellationEvaluationShader,
                          const QString& geometryShader,
                          const QString& fragmentShader)
{
    // Create a shader program
    if(!m_shader->addShaderFromSourceFile(QOpenGLShader::Vertex, vertexShader))
        qCritical() << QObject::tr("Could not compile vertex shader. Log:") << m_shader->log();

    if(!m_shader->addShaderFromSourceFile(QOpenGLShader::TessellationControl, tessellationControlShader))
        qCritical() << QObject::tr("Could not compile tessellation control shader. Log:") << m_shader->log();

    if(!m_shader->addShaderFromSourceFile(QOpenGLShader::TessellationEvaluation, tessellationEvaluationShader))
        qCritical() << QObject::tr("Could not compile tessellation evaluation shader. Log:") << m_shader->log();

    if(!m_shader->addShaderFromSourceFile(QOpenGLShader::Geometry, geometryShader))
        qCritical() << QObject::tr("Could not compile geometry shader. Log:") << m_shader->log();

    if(!m_shader->addShaderFromSourceFile(QOpenGLShader::Fragment, fragmentShader))
        qCritical() << QObject::tr("Could not compile fragment shader. Log:") << m_shader->log();

    if(!m_shader->link())
        qCritical() << QObject::tr("Could not link shader program. Log:") << m_shader->log();
}

void Material::setShader(const QOpenGLShaderProgramPtr& shader)
{
    m_shader = shader;
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

/// Chunk Material
ChunkMaterial::ChunkMaterial() : Material()
{
}

ChunkMaterial::~ChunkMaterial()
{
    m_shader->release();
}

void ChunkMaterial::bind()
{
    m_shader->bind();

    foreach(const GLuint unit, m_unitConfigs.keys())
    {
        const TextureUnitConfiguration& config = m_unitConfigs.value(unit);

        // Check the texture
        if(config.texture()->isNull())
            continue;

        // Bind the texture
        m_funcs->glActiveTexture(GL_TEXTURE0 + unit);
        config.texture()->bind();

        // Bind the sampler
        config.sampler()->bind(unit);

        // Associate with sampler uniform in shader (if we know the name or location)
        if(m_samplerUniforms.contains(unit))
            m_shader->setUniformValue(m_samplerUniforms.value(unit).constData(), unit);
    }

    m_funcs->glActiveTexture(GL_TEXTURE0);

    foreach(const GLuint unit, m_FramebufferUnitConfigs.keys())
    {
        const GLuint& textureID = m_FramebufferUnitConfigs.value(unit);

        // Bind the texture
        m_funcs->glActiveTexture(GL_TEXTURE0 + unit);
        m_funcs->glBindTexture(GL_TEXTURE_2D, textureID);

        // Associate with byte uniform in shader (if we know the name or location)
        if(m_FramebufferByteUnitUniforms.contains(unit))
            m_shader->setUniformValue(m_FramebufferByteUnitUniforms.value(unit).constData(), unit);
    }
}