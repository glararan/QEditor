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

#include "modelinterface.h"

ModelInterface::ModelInterface(TextureManager* manager, const QString filename)
: has_animations(false)
, meshes(0)
, animations(0)
, bones(0)
, textureManager(manager)
{
    if(filename.contains("/"))
    {
        int index = filename.lastIndexOf("/") + 1;

        filePath = filename.left(index);
        fileName = filename.right(filename.size() - index);
    }
    else
        fileName = filename;

    Assimp::Importer importer;
    importer.SetPropertyInteger(AI_CONFIG_PP_LBW_MAX_WEIGHTS, 4);

    const aiScene* scene = importer.ReadFile(filename.toStdString(), aiProcess_GenSmoothNormals | aiProcess_Triangulate | aiProcess_CalcTangentSpace/* | aiProcess_FlipUVs*/);

    if(scene == 0 || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        throw QString("The file wasn't successfuly opened");

        return;
    }

    if(scene->HasAnimations())
    {
        has_animations = true;
        bones          = loadBones(scene);

        buildSkeleton(scene->mRootNode, NULL);

        animations = new Animations();
        animations->setBones(bones);

        for(uint i = 0; i < scene->mNumAnimations; ++i)
            animations->add(loadAnimation(scene->mAnimations[i], i));
    }

    meshes = new Meshes();

    for(uint i = 0; i < scene->mNumMeshes; ++i)
        meshes->add(loadMesh(scene->mMeshes[i], scene->mMaterials[scene->mMeshes[i]->mMaterialIndex], i));

    initialize(scene);
    // try delete scene probably crash
}

ModelInterface::~ModelInterface()
{
    if(animations)
        delete animations;

    if(bones)
        delete bones;

    if(meshes)
        delete meshes;
}

void ModelInterface::initialize(const aiScene* scene)
{
    QVector<QVector3D> vertices;
    QVector<QVector3D> _bones;

    /// Vertices
    for(int i = 0; i < scene->mNumMeshes; ++i)
    {
        for(int j = 0; j < scene->mMeshes[i]->mNumVertices; ++j)
            vertices.append(QVector3D(scene->mMeshes[i]->mVertices[j].x, scene->mMeshes[i]->mVertices[j].y, scene->mMeshes[i]->mVertices[j].z));
    }

    // find min and max
    QVector3D minVertex;
    QVector3D maxVertex;

    for(int i = 0; i < vertices.count(); ++i)
    {
        if(vertices[i].x() < minVertex.x()) // min X
            minVertex.setX(vertices[i].x());

        if(vertices[i].y() < minVertex.y()) // min Y
            minVertex.setY(vertices[i].y());

        if(vertices[i].z() < minVertex.z()) // min Z
            minVertex.setZ(vertices[i].z());

        if(vertices[i].x() > maxVertex.x()) // max X
            maxVertex.setX(vertices[i].x());

        if(vertices[i].y() > maxVertex.y()) // max Y
            maxVertex.setY(vertices[i].y());

        if(vertices[i].z() > maxVertex.z()) // max Z
            maxVertex.setZ(vertices[i].z());
    }

    header.boundingBoxMin = minVertex;
    header.boundingBoxMax = maxVertex;
    header.center         = (maxVertex + minVertex) / 2;
}

Bones* ModelInterface::loadBones(const aiScene* scene)
{
    Bones* bones = new Bones();

    for(uint mesh_index = 0; mesh_index < scene->mNumMeshes; ++mesh_index)
    {
        aiMesh* ai_mesh = scene->mMeshes[mesh_index];

        if(ai_mesh->HasBones())
        {
            for(uint bone_index = 0; bone_index < ai_mesh->mNumBones; ++bone_index)
            {
                aiBone* ai_bone = ai_mesh->mBones[bone_index];

                QString bone_name(ai_bone->mName.data);

                if(!bones->hasBone(bone_name))
                    bones->createEmptyBone(bone_name, getMatrix(&ai_bone->mOffsetMatrix));
            }
        }
    }

    return bones;
}

Animation* ModelInterface::loadAnimation(const aiAnimation* ai_animation, const int index)
{
    QString animation_name(ai_animation->mName.data);

    double duration = ai_animation->mDuration;
    //qDebug() << "Animation: " + animation_name + " duration " + QString::number(duration);

    Animation* animation = new Animation(animation_name, duration, index);
    animation->setBoneCount(bones->getBoneNames().size());

    for(uint bone_index = 0; bone_index < ai_animation->mNumChannels; ++bone_index)
    {
        aiNodeAnim* channel = ai_animation->mChannels[bone_index];

        QString bone_name(channel->mNodeName.data);

        if(!bones->hasBone(bone_name))
            continue;

        int bone_id = bones->getBone(bone_name)->getId();

        animation->registerBone(bone_id);

        for(uint i = 0; i < channel->mNumPositionKeys; ++i)
        {
            aiVectorKey pos_key = channel->mPositionKeys[i];

            animation->addBonePosition(bone_id, float(pos_key.mTime), QVector3D(pos_key.mValue.x, pos_key.mValue.y, pos_key.mValue.z));
        }

        for(uint i = 0; i < channel->mNumRotationKeys; ++i)
        {
            aiQuatKey rot_key = channel->mRotationKeys[i];

            animation->addBoneRotation(bone_id, float(rot_key.mTime), QQuaternion(rot_key.mValue.w, rot_key.mValue.x, rot_key.mValue.y, rot_key.mValue.z));
        }

        for(uint i = 0; i < channel->mNumScalingKeys; ++i)
        {
            aiVectorKey scale_key = channel->mScalingKeys[i];

            animation->addBoneScaling(bone_id, float(scale_key.mTime), QVector3D(scale_key.mValue.x, scale_key.mValue.y, scale_key.mValue.z));
        }
    }

    return animation;
}

Mesh* ModelInterface::loadMesh(aiMesh* ai_mesh, aiMaterial* ai_material, const int index)
{
    Q_UNUSED(index)

    Mesh* mesh = new Mesh();

    QVector3D* vertices  = new QVector3D[ai_mesh->mNumVertices];
    QVector3D* normals   = new QVector3D[ai_mesh->mNumVertices];
    QVector3D* tangent   = new QVector3D[ai_mesh->mNumVertices];
    QVector2D* texCoords = new QVector2D[ai_mesh->mNumVertices];
    QVector4D* boneIDs   = new QVector4D[ai_mesh->mNumVertices];
    QVector4D* weight    = new QVector4D[ai_mesh->mNumVertices];

    QVector<uint> indices;

    if(ai_mesh->HasBones() && bones)
    {
        for(uint i = 0; i < ai_mesh->mNumBones; ++i)
        {
            for(uint j = 0; j < ai_mesh->mBones[i]->mNumWeights; ++j)
            {
                QString boneName(ai_mesh->mBones[i]->mName.data);

                aiVertexWeight w  = ai_mesh->mBones[i]->mWeights[j];
                int vertexId      = w.mVertexId;
                float weightValue = w.mWeight;

                int boneID = bones->getBone(boneName)->getId();

                addWeightData(&boneIDs[vertexId], &weight[vertexId], boneID, weightValue);
            }
        }
    }

    for(uint i = 0; i < ai_mesh->mNumVertices; ++i)
    {
        vertices[i] = QVector3D(ai_mesh->mVertices[i].x, ai_mesh->mVertices[i].y, ai_mesh->mVertices[i].z);
        normals[i]  = QVector3D(ai_mesh->mNormals[i].x,  ai_mesh->mNormals[i].y,  ai_mesh->mNormals[i].z);

        if(ai_mesh->mTangents)
            tangent[i] = QVector3D(ai_mesh->mTangents[i].x, ai_mesh->mTangents[i].y, ai_mesh->mTangents[i].z);
        else
            tangent[i] = QVector3D(1.0f, 0.0f, 0.0f);

        if(ai_mesh->mTextureCoords[0])
            texCoords[i] = QVector2D(ai_mesh->mTextureCoords[0][i].x, ai_mesh->mTextureCoords[0][i].y);
        else
            texCoords[i] = QVector2D(0.0f, 0.0f);
    }

    for(uint i = 0; i < ai_mesh->mNumFaces; ++i)
    {
        aiFace face = ai_mesh->mFaces[i];

        for(uint j = 0; j < face.mNumIndices; ++j)
            indices.push_back(face.mIndices[j]);
    }

    loadMaterial(ai_material, mesh);

    mesh->createVertexArrayObject();
    mesh->createBuffer(Mesh::Vertices,  vertices, sizeof(QVector3D)  * ai_mesh->mNumVertices);
    mesh->createBuffer(Mesh::Normals,   normals, sizeof(QVector3D)   * ai_mesh->mNumVertices);
    mesh->createBuffer(Mesh::TexCoords, texCoords, sizeof(QVector2D) * ai_mesh->mNumVertices);
    mesh->createBuffer(Mesh::Tangent,   tangent, sizeof(QVector3D)   * ai_mesh->mNumVertices);
    mesh->createBuffer(Mesh::Bones,     boneIDs, sizeof(QVector4D)   * ai_mesh->mNumVertices);
    mesh->createBuffer(Mesh::Weight,    weight, sizeof(QVector4D)    * ai_mesh->mNumVertices);
    mesh->createBuffer(Mesh::Index,     indices.data(), sizeof(int)  * indices.size());
    mesh->setNumFaces(indices.size());

    delete[] vertices;
    delete[] normals;
    delete[] tangent;
    delete[] texCoords;
    delete[] weight;
    delete[] boneIDs;

    return mesh;
}

void ModelInterface::loadMaterial(aiMaterial* ai_material, Mesh* meshTarget)
{
    aiColor3D color(0.f, 0.f, 0.f);

    float shininess = 0.0f;
    float opacity   = 1.0f;

    ai_material->Get(AI_MATKEY_COLOR_AMBIENT, color);
    meshTarget->getMeshMaterial()->mAmbient = QVector3D(color.r, color.g, color.b);

    ai_material->Get(AI_MATKEY_COLOR_DIFFUSE, color);
    meshTarget->getMeshMaterial()->mDiffuse = QVector3D(color.r, color.g, color.b);

    ai_material->Get(AI_MATKEY_COLOR_SPECULAR, color);
    meshTarget->getMeshMaterial()->mSpecular = QVector3D(color.r, color.g, color.b);

    ai_material->Get(AI_MATKEY_SHININESS, shininess);
    meshTarget->getMeshMaterial()->shininess = shininess;

    ai_material->Get(AI_MATKEY_OPACITY, opacity);
    meshTarget->getMeshMaterial()->opacity = opacity;

    QString textureFolder = fileName.left(fileName.indexOf(".")) + "/";

    loadTextures(ai_material, aiTextureType_DIFFUSE,  meshTarget->getMeshTextures()->diffuseTextureIndex,  meshTarget->getMeshTextures()->hasDiffuseTexture,  filePath + textureFolder, textureManager);
    loadTextures(ai_material, aiTextureType_SPECULAR, meshTarget->getMeshTextures()->specularTextureIndex, meshTarget->getMeshTextures()->hasSpecularTexture, filePath + textureFolder, textureManager);
    loadTextures(ai_material, aiTextureType_NORMALS,  meshTarget->getMeshTextures()->normalsTextureIndex,  meshTarget->getMeshTextures()->hasNormalsTexture,  filePath + textureFolder, textureManager);
    loadTextures(ai_material, aiTextureType_HEIGHT,   meshTarget->getMeshTextures()->heightTextureIndex,   meshTarget->getMeshTextures()->hasHeightTexture,   filePath + textureFolder, textureManager);
}

void ModelInterface::buildSkeleton(aiNode* current, Bone* parent)
{
    Bone* bone;

    if(!bones->hasBone(QString(current->mName.data)))
        bone = bones->createEmptyBone(QString(current->mName.data));
    else
        bone = bones->getBone(QString(current->mName.data));

    bone->setNodeTransformation(getMatrix(&current->mTransformation));

    if(parent != NULL)
        parent->addChild(bone);

    for(uint child_index = 0; child_index < current->mNumChildren; ++child_index)
        buildSkeleton(current->mChildren[child_index], bone);
}

void ModelInterface::loadTextures(aiMaterial* ai_material, aiTextureType type, int &index, bool& succes, QString filePath, TextureManager* manager)
{
    aiString str;

    if(ai_material->GetTextureCount(type) > 0)
    {
        ai_material->GetTexture(type, 0, &str);

        QString textureFilename = QString(str.C_Str());
        QString textureFilepath = filePath;
        QString textureFile     = textureFilepath + textureFilename;

        textureFilename = textureFilename.replace(".tga", ".png"); //doesnt support tga formats, so try find png
        textureFile     = textureFile.replace(".tga", ".png");

        QString textureName = QFileInfo(textureFilepath).baseName() + "ModelTexture";

        if(manager->hasTexture(textureName, textureFile))
        {
            index  = manager->getIndex(textureFile);
            succes = true;
        }
        else
        {
            manager->loadTexture(textureName, textureFile);

            if(manager->hasTexture(textureName, textureFile))
            {
                index  = manager->getIndex(textureFile);
                succes = true;
            }
            else
            {
                succes = false;

                qDebug() << "error load: " + textureFile;
            }
        }
    }
}

void ModelInterface::addWeightData(QVector4D* boneIdTarget, QVector4D* weightTarget, float id, float w)
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

QMatrix4x4 ModelInterface::getMatrix(const aiMatrix4x4* m)
{
    QMatrix4x4 nodeMatrix;

    if(m->IsIdentity())
        return nodeMatrix;

    aiQuaternion rotation;
    aiVector3D   position;
    aiVector3D   scale;

    m->Decompose(scale, rotation, position);

    QVector3D   qscale(scale.x,scale.y, scale.z);
    QVector3D   qposition(position.x, position.y, position.z);
    QQuaternion qrotation(rotation.w, rotation.x, rotation.y, rotation.z);

    if(!qscale.isNull())
        nodeMatrix.scale(qscale);

    if(!qposition.isNull())
        nodeMatrix.translate(qposition);

    if(!qrotation.isNull())
        nodeMatrix.rotate(qrotation);

    return nodeMatrix;
}
