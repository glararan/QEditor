#include "sampler.h"

#include <QOpenGLContext>
#include <QOpenGLFunctions_3_3_Core>

Sampler::Sampler()
{
}

Sampler::~Sampler()
{
}

void Sampler::create()
{
    QOpenGLContext* context = QOpenGLContext::currentContext();

    Q_ASSERT(context);

    m_funcs = context->versionFunctions<QOpenGLFunctions_3_3_Core>();
    m_funcs->initializeOpenGLFunctions();
    m_funcs->glGenSamplers(1, &m_samplerId);
}

void Sampler::destroy()
{
    if(m_samplerId)
    {
        m_funcs->glDeleteSamplers(1, &m_samplerId);

        m_samplerId = 0;
    }
}

void Sampler::bind(GLuint unit)
{
    m_funcs->glBindSampler(unit, m_samplerId);
}

void Sampler::release(GLuint unit)
{
    m_funcs->glBindSampler(unit, 0);
}

void Sampler::setWrapMode(CoordinateDirection direction, GLenum wrapMode)
{
    m_funcs->glSamplerParameteri(m_samplerId, direction, wrapMode);
}

void Sampler::setMinificationFilter(GLenum filter)
{
    m_funcs->glSamplerParameteri(m_samplerId, GL_TEXTURE_MIN_FILTER, filter);
}

void Sampler::setMagnificationFilter(GLenum filter)
{
    m_funcs->glSamplerParameteri(m_samplerId, GL_TEXTURE_MAG_FILTER, filter);
}
