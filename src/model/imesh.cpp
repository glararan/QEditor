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

#include "imesh.h"

IMesh::IMesh()
{
    buffers[6] = QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
}

IMesh::~IMesh()
{
    vao.destroy();

    for(int i = 0; i < 7; ++i)
        buffers[i].destroy();
}

void IMesh::createVertexArrayObject()
{
    vao.create();
    vao.bind();
}

QOpenGLBuffer* IMesh::getBuffer(IMesh::BufferName name)
{
    return &buffers[name];
}

void IMesh::createBuffer(BufferName name, void* data, int count)
{
    QOpenGLBuffer* buffer = getBuffer(name);
    buffer->create();
    buffer->setUsagePattern(QOpenGLBuffer::StaticDraw);
    buffer->bind();
    buffer->allocate(data,count);
    buffer->release();
}

void IMesh::createAttributeArray(BufferName name, QOpenGLShaderProgram* shader, const char* location, GLenum type, int offset, int tupleSize)
{
    QOpenGLBuffer* buffer = getBuffer(name);
    buffer->bind();
    shader->enableAttributeArray(location);
    shader->setAttributeBuffer(location, type, offset, tupleSize);
}

QOpenGLVertexArrayObject* IMesh::getVertexArrayObject()
{
    return &vao;
}

void IMesh::bind()
{
    if(!getVertexArrayObject()->isCreated())
        createVertexArrayObject();

    getVertexArrayObject()->bind();

    if(!getBuffer(IMesh::Index)->isCreated())
        getBuffer(IMesh::Index)->create();

    getBuffer(IMesh::Index)->bind();
}

IMeshes::IMeshes()
{
}

IMeshes::~IMeshes()
{
    for(int i = 0; i < meshes.size(); ++i)
    {
        IMesh* m = meshes.at(i);

        delete m;

        m = NULL;
    }
}

void IMeshes::add(IMesh* mesh)
{
    meshes.push_back(mesh);
}

IMesh* IMeshes::at(int index)
{
    return meshes.at(index);
}

int IMeshes::size()
{
    return meshes.size();
}

void IMeshes::createAttributeArray(QOpenGLShaderProgram* shader)
{
    for(int i = 0; i < meshes.size(); ++i)
    {
        IMesh* mesh = meshes.at(i);

        mesh->getVertexArrayObject()->bind();
        mesh->createAttributeArray(IMesh::Vertices,  shader, "qt_Vertex",   GL_FLOAT, 0, 3);
        mesh->createAttributeArray(IMesh::Normals,   shader, "qt_Normal",   GL_FLOAT, 0, 3);
        mesh->createAttributeArray(IMesh::TexCoords, shader, "qt_TexCoord", GL_FLOAT, 0, 2);
        mesh->createAttributeArray(IMesh::Tangent,   shader, "qt_Tangent",  GL_FLOAT, 0, 3);
        mesh->createAttributeArray(IMesh::Bones,     shader, "qt_BoneIDs",  GL_FLOAT, 0, 4);
        mesh->createAttributeArray(IMesh::Weight,    shader, "qt_Weights",  GL_FLOAT, 0, 4);
    }
}