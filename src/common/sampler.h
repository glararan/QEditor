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

#ifndef SAMPLER_H
#define SAMPLER_H

#include <qopengl.h>
#include <QSharedPointer>

class QOpenGLFunctions_4_2_Core;

class Sampler
{
public:
    Sampler();
    ~Sampler();

    void create();
    void destroy();
    GLuint samplerId() const { return m_samplerId; }
    void bind(GLuint unit);
    void release(GLuint unit);

    enum CoordinateDirection
    {
        DirectionS = GL_TEXTURE_WRAP_S,
        DirectionT = GL_TEXTURE_WRAP_T,
        DirectionR = GL_TEXTURE_WRAP_R
    };

    void setWrapMode(CoordinateDirection direction, GLenum wrapMode);

    void setMinificationFilter(GLenum filter);
    void setMagnificationFilter(GLenum filter);

private:
    GLuint m_samplerId;
    QOpenGLFunctions_4_2_Core* m_funcs;
};

typedef QSharedPointer<Sampler> SamplerPtr;

#endif // SAMPLER_H
