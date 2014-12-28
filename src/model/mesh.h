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

#ifndef MESH_H
#define MESH_H

#include <QtWidgets>

#include <assimp/scene.h>

#include "texturemanager.h"

struct MeshMaterial
{
    QVector3D mAmbient;
    QVector3D mDiffuse;
    QVector3D mSpecular;

    float shininess;
    float opacity;
};

struct MeshTextures
{
    MeshTextures()
    {
        hasDiffuseTexture  = false;
        hasSpecularTexture = false;
        hasNormalsTexture  = false;
        hasHeightTexture   = false;
    }

    bool hasDiffuseTexture;
    int diffuseTextureIndex;

    bool hasSpecularTexture;
    int specularTextureIndex;

    bool hasNormalsTexture;
    int normalsTextureIndex;

    bool hasHeightTexture;
    int heightTextureIndex;
};

class Mesh
{
public:
    enum BufferName
    {
        Vertices   = 0,
        Normals    = 1,
        TexCoords  = 2,
        Tangent    = 3,
        Bones      = 4,
        Weight     = 5,
        Index      = 6
    };

    Mesh();
    ~Mesh();

    void bind();

    void createVertexArrayObject();
    void createBuffer(const BufferName name, void* data, const int count);
    void createAttributeArray(const BufferName name, QOpenGLShaderProgram* shader, const char* location, const GLenum type, const int offset, const int tupleSize);

    MeshMaterial* getMeshMaterial() { return &meshMaterial; }
    MeshTextures* getMeshTextures() { return &meshTextures; }

    const int getNumFaces() const         { return numFaces; }
    void setNumFaces(const int _numFaces) { numFaces = _numFaces; }

    QOpenGLBuffer*            getBuffer(const BufferName name) { return &buffers[name]; }
    QOpenGLVertexArrayObject* getVertexArrayObject()           { return &vao; }

private:
    int numFaces;

    MeshMaterial meshMaterial;
    MeshTextures meshTextures;

    QOpenGLBuffer buffers[7];
    QOpenGLVertexArrayObject vao;
};

class Meshes
{
public:
    Meshes();
    ~Meshes();

    void add(Mesh* mesh);

    Mesh* at(const int index) const { return meshes.at(index); }

    const int size() const { return meshes.size(); }

    void createAttributeArray(QOpenGLShaderProgram* shader);

private:
    QList<Mesh*> meshes;
};

#endif // MESH_H
