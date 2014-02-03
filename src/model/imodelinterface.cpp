#include "imodelinterface.h"


IModelInterface::IModelInterface(ITextureManager *textureManager, QString filename) :
    has_animations(false),
    Meshes(0),
    Animations(0),
    Bones(0),
    TextureManager(textureManager)
{
    if(filename.contains("/"))
    {
        int index = filename.lastIndexOf("/") + 1;
        filePath = filename.left(index);
        fileName = filename.right(filename.size() - index);
    }
    else
    {
        fileName = filename;
    }

    Assimp::Importer importer;
    importer.SetPropertyInteger(AI_CONFIG_PP_LBW_MAX_WEIGHTS, 4);
    const aiScene* scene = importer.ReadFile(filename.toStdString(), aiProcess_GenSmoothNormals | aiProcess_Triangulate | aiProcess_CalcTangentSpace/* | aiProcess_FlipUVs*/);

    if(scene==0 || scene->mFlags==AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        throw QString("The file wasn't successfuly opened");
        return;
    }

    if (scene->HasAnimations())
    {
        has_animations = true;
        Bones = loadBones(scene);
        buildSkeleton(scene->mRootNode, NULL);
        Animations = new IAnimations();
        Animations->setBones(Bones);
        for (uint i = 0; i < scene->mNumAnimations; ++i)
        {
            Animations->add(loadAnimation(scene->mAnimations[i], i));
        }
    }

    Meshes = new IMeshes();
    for (uint i = 0; i < scene->mNumMeshes; ++i)
    {
        Meshes->Add(loadMesh(scene->mMeshes[i],scene->mMaterials[scene->mMeshes[i]->mMaterialIndex] ,i));
    }
}

IModelInterface::~IModelInterface()
{
    if(Animations)
        delete Animations;
    if(Bones)
        delete Bones;
    if(Meshes)
        delete Meshes;
}

bool IModelInterface::hasAnimations()
{
    return has_animations;
}

IMeshes *IModelInterface::getMeshes()
{
    return Meshes;
}

IBones *IModelInterface::getBones()
{
    return Bones;
}

IAnimations *IModelInterface::getAnimations()
{
    return Animations;
}

QString IModelInterface::getFilePath()
{
    return filePath;
}

QString IModelInterface::getFileName()
{
    return fileName;
}

IBones *IModelInterface::loadBones(const aiScene *scene)
{
    IBones *bones = new IBones();

    for (uint mesh_index = 0; mesh_index < scene->mNumMeshes; mesh_index++)
    {
        aiMesh *ai_mesh = scene->mMeshes[mesh_index];
        if (ai_mesh->HasBones())
        {
            for (uint bone_index = 0; bone_index < ai_mesh->mNumBones; bone_index++)
            {
                aiBone *ai_bone = ai_mesh->mBones[bone_index];
                QString bone_name(ai_bone->mName.data);
                if (!bones->hasBone(bone_name))
                    bones->createEmptyBone(bone_name,getMatrix(&ai_bone->mOffsetMatrix));
            }
        }
    }

    return bones;
}

IAnimation *IModelInterface::loadAnimation(const aiAnimation *ai_animation, int index)
{
    QString animation_name(ai_animation->mName.data);
    double duration = ai_animation->mDuration;
    //qDebug() << "Animation: " + animation_name + " duration " + QString::number(duration);

    IAnimation *animation = new IAnimation(animation_name, duration, index);
    animation->setBoneCount(Bones->getBoneNames().size());

    for (uint bone_index = 0; bone_index < ai_animation->mNumChannels; bone_index++)
    {
        aiNodeAnim *channel = ai_animation->mChannels[bone_index];
        QString bone_name(channel->mNodeName.data);

        if(!Bones->hasBone(bone_name))
            continue;

        int bone_id = Bones->getBone(bone_name)->getId();

        animation->registerBone(bone_id);

        for (unsigned int i = 0; i < channel->mNumPositionKeys; i++)
        {
            aiVectorKey pos_key = channel->mPositionKeys[i];
            animation->addBonePosition(bone_id, float(pos_key.mTime), QVector3D(pos_key.mValue.x, pos_key.mValue.y, pos_key.mValue.z));
        }
        for (unsigned int i = 0; i < channel->mNumRotationKeys; i++)
        {
            aiQuatKey rot_key = channel->mRotationKeys[i];
            animation->addBoneRotation(bone_id, float(rot_key.mTime), QQuaternion(rot_key.mValue.w, rot_key.mValue.x, rot_key.mValue.y, rot_key.mValue.z));
        }
        for (unsigned int i = 0; i < channel->mNumScalingKeys; i++)
        {
            aiVectorKey scale_key = channel->mScalingKeys[i];
            animation->addBoneScaling(bone_id, float(scale_key.mTime), QVector3D(scale_key.mValue.x, scale_key.mValue.y, scale_key.mValue.z));
        }
    }

    return animation;
}

IMesh* IModelInterface::loadMesh(aiMesh *ai_mesh, aiMaterial *ai_material, int index)
{
    Q_UNUSED(index);

    IMesh *mesh = new IMesh();

    QVector3D * vertices = new QVector3D[ai_mesh->mNumVertices];
    QVector3D * normals = new QVector3D[ai_mesh->mNumVertices];
    QVector3D * tangent = new QVector3D[ai_mesh->mNumVertices];
    QVector2D* texCoords = new QVector2D[ai_mesh->mNumVertices];
    QVector4D * boneIDs = new QVector4D[ai_mesh->mNumVertices];
    QVector4D * weight  = new QVector4D[ai_mesh->mNumVertices];
    QVector<unsigned int> indices;

    if(ai_mesh->HasBones() && Bones)
    {
        for(uint i = 0; i < ai_mesh->mNumBones; ++i)
        {
            for(uint j = 0; j < ai_mesh->mBones[i]->mNumWeights; ++j)
            {
                QString boneName(ai_mesh->mBones[i]->mName.data);
                aiVertexWeight w = ai_mesh->mBones[i]->mWeights[j];
                int vertexId = w.mVertexId;
                float weightValue = w.mWeight;

                int boneID = Bones->getBone(boneName)->getId();

                IMesh::addWeightData(&boneIDs[vertexId],&weight[vertexId],boneID,weightValue);
            }
        }
    }

    for(uint i = 0;i < ai_mesh->mNumVertices; ++i)
    {
        vertices[i] = QVector3D(ai_mesh->mVertices[i].x, ai_mesh->mVertices[i].y, ai_mesh->mVertices[i].z);
        normals[i] = QVector3D(ai_mesh->mNormals[i].x, ai_mesh->mNormals[i].y, ai_mesh->mNormals[i].z);

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
    mesh->numFaces = indices.size();

    aiColor3D color (0.f,0.f,0.f);
    float shininess = 0.0f;
    float opacity = 1.0f;
    ai_material->Get(AI_MATKEY_COLOR_AMBIENT,color);
    mesh->mAmbient = QVector3D(color.r,color.g,color.b);
    ai_material->Get(AI_MATKEY_COLOR_DIFFUSE,color);
    mesh->mDiffuse = QVector3D(color.r,color.g,color.b);
    ai_material->Get(AI_MATKEY_COLOR_SPECULAR,color);
    mesh->mSpecular = QVector3D(color.r,color.g,color.b);
    ai_material->Get(AI_MATKEY_SHININESS,shininess);
    mesh->shininess = shininess;
    ai_material->Get(AI_MATKEY_OPACITY,opacity);
    mesh->opacity = opacity;

    QOpenGLVertexArrayObject *vao = mesh->getVertexArrayObject();
    vao->create();
    vao->bind();

    IMesh::createBuffer(mesh->getVerticesBuffer(),vertices,sizeof(QVector3D) * ai_mesh->mNumVertices);
    IMesh::createBuffer(mesh->getNormalsBuffer(),normals,sizeof(QVector3D) * ai_mesh->mNumVertices);
    IMesh::createBuffer(mesh->getTexCoordsBuffer(),texCoords,sizeof(QVector2D) * ai_mesh->mNumVertices);
    IMesh::createBuffer(mesh->getTangentBuffer(),tangent,sizeof(QVector3D) * ai_mesh->mNumVertices);
    IMesh::createBuffer(mesh->getBonesBuffer(),boneIDs,sizeof(QVector4D) * ai_mesh->mNumVertices);
    IMesh::createBuffer(mesh->getWeightBuffer(),weight,sizeof(QVector4D) * ai_mesh->mNumVertices);
    IMesh::createBuffer(mesh->getIndexBuffer(),indices.data(),sizeof(int) * indices.size());

    QString textureFolder = fileName.left(fileName.indexOf(".")) + "/";

    IMesh::loadTextures(ai_material,aiTextureType_DIFFUSE,mesh->diffuseIndex,mesh->hasDiffuse,filePath + textureFolder,TextureManager);
    IMesh::loadTextures(ai_material,aiTextureType_SPECULAR,mesh->specularIndex,mesh->hasSpecular,filePath + textureFolder,TextureManager);
    IMesh::loadTextures(ai_material,aiTextureType_NORMALS,mesh->normalsIndex,mesh->hasNormals,filePath + textureFolder,TextureManager);
    IMesh::loadTextures(ai_material,aiTextureType_HEIGHT,mesh->heightIndex,mesh->hasHeight,filePath + textureFolder,TextureManager);

    delete [] vertices;
    delete [] normals;
    delete [] tangent;
    delete [] texCoords;
    delete [] weight;
    delete [] boneIDs;

    return mesh;
}

void IModelInterface::buildSkeleton(aiNode* current, IBone* parent)
{
    IBone* bone;

    if(!Bones->hasBone(QString(current->mName.data)))
        bone = Bones->createEmptyBone(QString(current->mName.data));
    else
        bone = Bones->getBone(QString(current->mName.data));

    bone->setNodeTransformation(getMatrix(&current->mTransformation));

    if(parent != NULL)
        parent->addChild(bone);

    for (uint child_index = 0; child_index < current->mNumChildren; child_index++)
        buildSkeleton(current->mChildren[child_index], bone);
}

QMatrix4x4 IModelInterface::getMatrix(const aiMatrix4x4* m)
{
    QMatrix4x4 nodeMatrix;
    if (m->IsIdentity())
        return nodeMatrix;
    aiQuaternion rotation;
    aiVector3D position;
    aiVector3D scale;
    m->Decompose(scale, rotation, position);
    QVector3D qscale(scale.x,scale.y, scale.z);
    QVector3D qposition(position.x, position.y, position.z);
    QQuaternion qrotation(rotation.w, rotation.x, rotation.y, rotation.z);
    if (!qscale.isNull())
        nodeMatrix.scale(qscale);
    if (!qposition.isNull())
        nodeMatrix.translate(qposition);
    if (!qrotation.isNull())
        nodeMatrix.rotate(qrotation);
    return nodeMatrix;
}
