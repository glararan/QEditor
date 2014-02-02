#ifndef IMESH_H
#define IMESH_H

#include <QtWidgets>
#include <assimp/scene.h>
#include "itexturemanager.h"

class IMesh
{
public:
    IMesh();
    ~IMesh();
    static void addWeightData(QVector4D * boneIdTarget, QVector4D * weightTarget, float id, float w);
    static void createBuffer(QOpenGLBuffer *buffer, void *data, int count);
    static void createAttributeArray(QOpenGLBuffer *buffer, QOpenGLShaderProgram *shader, const char *location, GLenum type, int offset, int tupleSize);
    static void loadTextures(aiMaterial *ai_material, aiTextureType type, int &index, bool &succes, QString filePath, ITextureManager *textureManager);

    QVector3D mAmbient;
    QVector3D mDiffuse;
    QVector3D mSpecular;
    float shininess;
    float opacity;

    int numFaces;

    //diffuse
    bool hasDiffuse;
    int diffuseIndex;
    //specular
    bool hasSpecular;
    int specularIndex;
    //normals
    bool hasNormals;
    int normalsIndex;
    //height
    bool hasHeight;
    int heightIndex;

    QOpenGLBuffer *getVerticesBuffer();
    QOpenGLBuffer *getNormalsBuffer();
    QOpenGLBuffer *getTexCoordsBuffer();
    QOpenGLBuffer *getTangentBuffer();
    QOpenGLBuffer *getBonesBuffer();
    QOpenGLBuffer *getWeightBuffer();
    QOpenGLBuffer *getIndexBuffer();
    QOpenGLVertexArrayObject *getVertexArrayObject();


private:
    QOpenGLBuffer bufferVertices;
    QOpenGLBuffer bufferNormals;
    QOpenGLBuffer bufferTexCoords;
    QOpenGLBuffer bufferTangent;
    QOpenGLBuffer bufferIDs;
    QOpenGLBuffer bufferWeight;
    QOpenGLBuffer *indexBuffer;
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
    static void createAttributeArray(QOpenGLShaderProgram *shader, IMesh *mesh);

private:
    QList<IMesh*> meshes;
};

#endif // IMESH_H
