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

#include "sampler.h"

#include <QOpenGLContext>
#include <QOpenGLFunctions_4_2_Core>

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

    m_funcs = context->versionFunctions<QOpenGLFunctions_4_2_Core>();
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
