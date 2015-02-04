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

#include "mesh.h"

Mesh::Mesh()
{
    buffers[6] = QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
}

Mesh::~Mesh()
{
    vao.destroy();

    for(int i = 0; i < 7; ++i)
        buffers[i].destroy();
}

void Mesh::createVertexArrayObject()
{
    if(!vao.isCreated())
        vao.create();

    vao.bind();
}

void Mesh::createBuffer(const BufferName name, void* data, const int count)
{
    QOpenGLBuffer* buffer = getBuffer(name);

    if(!buffer->isCreated())
        buffer->create();

    buffer->setUsagePattern(QOpenGLBuffer::StaticDraw);
    buffer->bind();
    buffer->allocate(data, count);
    buffer->release();
}

void Mesh::createAttributeArray(const BufferName name, QOpenGLShaderProgram* shader, const char* location, const GLenum type, const int offset, const int tupleSize)
{
    QOpenGLBuffer* buffer = getBuffer(name);
    buffer->bind();

    shader->enableAttributeArray(location);
    shader->setAttributeBuffer(location, type, offset, tupleSize);
}

void Mesh::bind()
{
    if(!getVertexArrayObject()->isCreated())
        createVertexArrayObject();

    getVertexArrayObject()->bind();

    if(!getBuffer(Mesh::Index)->isCreated())
        getBuffer(Mesh::Index)->create();

    getBuffer(Mesh::Index)->bind();
}

Meshes::Meshes()
{
}

Meshes::~Meshes()
{
    for(int i = 0; i < meshes.size(); ++i)
    {
        Mesh* m = meshes.at(i);

        delete m;

        m = NULL;
    }
}

void Meshes::add(Mesh* mesh)
{
    meshes.push_back(mesh);
}

void Meshes::createAttributeArray(QOpenGLShaderProgram* shader)
{
    for(int i = 0; i < meshes.size(); ++i)
    {
        Mesh* mesh = meshes.at(i);

        mesh->getVertexArrayObject()->bind();
        mesh->createAttributeArray(Mesh::Vertices,  shader, "qt_Vertex",   GL_FLOAT, 0, 3);
        mesh->createAttributeArray(Mesh::Normals,   shader, "qt_Normal",   GL_FLOAT, 0, 3);
        mesh->createAttributeArray(Mesh::TexCoords, shader, "qt_TexCoord", GL_FLOAT, 0, 2);
        mesh->createAttributeArray(Mesh::Tangent,   shader, "qt_Tangent",  GL_FLOAT, 0, 3);
        mesh->createAttributeArray(Mesh::Bones,     shader, "qt_BoneIDs",  GL_FLOAT, 0, 4);
        mesh->createAttributeArray(Mesh::Weight,    shader, "qt_Weights",  GL_FLOAT, 0, 4);
    }
}
