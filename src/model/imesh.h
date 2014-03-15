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

#ifndef IMESH_H
#define IMESH_H

#include <QtWidgets>
#include <assimp/scene.h>
#include "itexturemanager.h"

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
    MeshTextures() {
        hasDiffuseTexture = false;
        hasSpecularTexture = false;
        hasNormalsTexture = false;
        hasHeightTexture = false;
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

class IMesh
{
public:
    enum BufferName { Vertices = 0, Normals, TexCoords, Tangent, Bones, Weight, Index };

    IMesh();
    ~IMesh();
    void createVertexArrayObject();
    void createBuffer(BufferName name, void *data, int count);
    void createAttributeArray(BufferName name, QOpenGLShaderProgram *shader, const char *location, GLenum type, int offset, int tupleSize);

    MeshMaterial *getMeshMaterial() { return &meshMaterial; }
    MeshTextures *getMeshTextures() { return &meshTextures; }
    int getNumFaces() { return numFaces; }
    void setNumFaces(int numFaces) { this->numFaces = numFaces; }

    QOpenGLBuffer *getBuffer(BufferName name);
    QOpenGLVertexArrayObject *getVertexArrayObject();

    void bind();

private:
    int numFaces;
    MeshMaterial meshMaterial;
    MeshTextures meshTextures;
    QOpenGLBuffer buffers[7];
    QOpenGLVertexArrayObject vao;
};

class IMeshes
{
public:
    IMeshes();
    ~IMeshes();
    void Add(IMesh *mesh);
    IMesh* at(int index);
    int size();
    void createAttributeArray(QOpenGLShaderProgram *shader);

private:
    QList<IMesh*> meshes;
};

#endif // IMESH_H
