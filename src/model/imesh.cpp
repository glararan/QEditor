#include "imesh.h"

IMesh::IMesh()
{
    buffers[6] = QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
}

IMesh::~IMesh()
{
    vao.destroy();

    for(int i = 0; i < 7; ++i)
    {
        buffers[i].destroy();
    }
}

void IMesh::createVertexArrayObject()
{
    vao.create();
    vao.bind();
}

QOpenGLBuffer *IMesh::getBuffer(IMesh::BufferName name)
{
    return &buffers[name];
}

void IMesh::createBuffer(BufferName name, void *data, int count)
{
    QOpenGLBuffer *buffer = getBuffer(name);
    buffer->create();
    buffer->setUsagePattern(QOpenGLBuffer::StaticDraw);
    buffer->bind();
    buffer->allocate(data,count);
    buffer->release();
}

void IMesh::createAttributeArray(BufferName name, QOpenGLShaderProgram *shader, const char *location, GLenum type, int offset, int tupleSize)
{
    QOpenGLBuffer *buffer = getBuffer(name);
    buffer->bind();
    shader->enableAttributeArray(location);
    shader->setAttributeBuffer(location,type,offset,tupleSize);
}

QOpenGLVertexArrayObject *IMesh::getVertexArrayObject()
{
    return &vao;
}

void IMesh::bind()
{
    getVertexArrayObject()->bind();
    getBuffer(IMesh::Index)->bind();
}


IMeshes::IMeshes()
{

}

IMeshes::~IMeshes()
{
    for(int i = 0; i < meshes.size(); ++i)
    {
        IMesh *m = meshes.at(i);
        delete m;
        m = NULL;
    }
}

void IMeshes::Add(IMesh *mesh)
{
    meshes.push_back(mesh);
}

IMesh *IMeshes::at(int index)
{
    return meshes.at(index);
}

int IMeshes::size()
{
    return meshes.size();
}

void IMeshes::createAttributeArray(QOpenGLShaderProgram *shader)
{
    for(int i = 0; i < meshes.size(); ++i)
    {
        IMesh *mesh = meshes.at(i);

        mesh->getVertexArrayObject()->bind();
        mesh->createAttributeArray(IMesh::Vertices,shader,"qt_Vertex",GL_FLOAT,0,3);
        mesh->createAttributeArray(IMesh::Normals,shader,"qt_Normal",GL_FLOAT,0,3);
        mesh->createAttributeArray(IMesh::TexCoords,shader,"qt_TexCoord",GL_FLOAT,0,2);
        mesh->createAttributeArray(IMesh::Tangent,shader,"qt_Tangent",GL_FLOAT,0,3);
        mesh->createAttributeArray(IMesh::Bones,shader,"qt_BoneIDs",GL_FLOAT,0,4);
        mesh->createAttributeArray(IMesh::Weight,shader,"qt_Weights",GL_FLOAT,0,4);
    }
}
