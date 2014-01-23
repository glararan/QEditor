#include "imesh.h"


IMesh::IMesh()
{
    indexBuffer = new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);

    hasDiffuse = false;
    hasSpecular = false;
    hasNormals = false;
    hasHeight = false;
}

IMesh::~IMesh()
{
    vao.destroy();
    bufferVertices.destroy();
    bufferNormals.destroy();
    bufferTexCoords.destroy();
    bufferTangent.destroy();
    bufferIDs.destroy();
    bufferWeight.destroy();
    indexBuffer->destroy();
    delete indexBuffer;
}

void IMesh::addWeightData(QVector4D *boneIdTarget, QVector4D *weightTarget, float id, float w)
{
    if(weightTarget->x() == 0.0)
    {
        boneIdTarget->setX(id);
        weightTarget->setX(w);
        return;
    }
    if(weightTarget->y() == 0.0)
    {
        boneIdTarget->setY(id);
        weightTarget->setY(w);
        return;
    }
    if(weightTarget->z() == 0.0)
    {
        boneIdTarget->setZ(id);
        weightTarget->setZ(w);
        return;
    }
    if(weightTarget->w() == 0.0)
    {
        boneIdTarget->setW(id);
        weightTarget->setW(w);
        return;
    }
}

void IMesh::createBuffer(QOpenGLBuffer *buffer, void *data, int count)
{
    buffer->create();
    buffer->setUsagePattern(QOpenGLBuffer::StaticDraw);
    buffer->bind();
    buffer->allocate(data,count);
    buffer->release();
}

void IMesh::createAttributeArray(QOpenGLBuffer *buffer, QOpenGLShaderProgram *shader, const char *location, GLenum type, int offset, int tupleSize)
{
    buffer->bind();
    shader->enableAttributeArray(location);
    shader->setAttributeBuffer(location,type,offset,tupleSize);
}

void IMesh::loadTextures(aiMaterial *ai_material, aiTextureType type, int &index, bool &succes, QString filePath, ITextureManager *textureManager)
{
    aiString str;
    if(ai_material->GetTextureCount(type) > 0)
    {
        ai_material->GetTexture(type,0,&str);
        QString textureFilename = filePath + QString(str.C_Str());

        textureFilename = textureFilename.replace(".tga",".png"); //doesnt support tga formats, so try find png

        if(textureManager->hasTexture(textureFilename))
        {
            index = textureManager->getIndex(textureFilename);
            succes = true;
        }
        else
        {
            if(textureManager->loadTexture(textureFilename))
            {
                index = textureManager->getIndex(textureFilename);
                succes = true;
            }
            else
            {
                succes = false;
                qDebug() << "error load: " + textureFilename;
            }
        }
    }
}

QOpenGLBuffer *IMesh::getVerticesBuffer()
{
    return &bufferVertices;
}

QOpenGLBuffer *IMesh::getNormalsBuffer()
{
    return &bufferNormals;
}

QOpenGLBuffer *IMesh::getTexCoordsBuffer()
{
    return &bufferTexCoords;
}

QOpenGLBuffer *IMesh::getTangentBuffer()
{
    return &bufferTangent;
}

QOpenGLBuffer *IMesh::getBonesBuffer()
{
    return &bufferIDs;
}

QOpenGLBuffer *IMesh::getWeightBuffer()
{
    return &bufferWeight;
}

QOpenGLBuffer *IMesh::getIndexBuffer()
{
    return indexBuffer;
}

QOpenGLVertexArrayObject *IMesh::getVertexArrayObject()
{
    return &vao;
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

void IMeshes::createAttributeArray(QOpenGLShaderProgram *shader, IMesh *mesh)
{
    mesh->getVertexArrayObject()->bind();
    IMesh::createAttributeArray(mesh->getVerticesBuffer(),shader,"qt_Vertex",GL_FLOAT,0,3);
    IMesh::createAttributeArray(mesh->getNormalsBuffer(),shader,"qt_Normal",GL_FLOAT,0,3);
    IMesh::createAttributeArray(mesh->getTexCoordsBuffer(),shader,"qt_TexCoord",GL_FLOAT,0,2);
    IMesh::createAttributeArray(mesh->getTangentBuffer(),shader,"qt_Tangent",GL_FLOAT,0,3);
    IMesh::createAttributeArray(mesh->getBonesBuffer(),shader,"qt_BoneIDs",GL_FLOAT,0,4);
    IMesh::createAttributeArray(mesh->getWeightBuffer(),shader,"qt_Weights",GL_FLOAT,0,4);
}
