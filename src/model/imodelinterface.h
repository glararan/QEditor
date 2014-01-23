#ifndef IMODELINTERFACE_H
#define IMODELINTERFACE_H

#include <QtCore>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "texture.h"
#include "ianimation.h"
#include "imesh.h"
#include "itexturemanager.h"

class IModelInterface
{
public:
    IModelInterface(ITextureManager *textureManager, QString filename);
    ~IModelInterface();
    bool hasAnimations();
    IMeshes *getMeshes();
    IBones *getBones();
    IAnimations *getAnimations();
    QString getFilePath();
    QString getFileName();

private:
    IBones *loadBones(const aiScene *scene);
    IAnimation *loadAnimation(const aiAnimation *ai_animation, int index);
    IMesh *loadMesh(aiMesh *ai_mesh, aiMaterial *ai_material ,int index);
    void buildSkeleton(aiNode *current, IBone *parent);
    static QMatrix4x4 getMatrix(const aiMatrix4x4 *m);

private:
    bool has_animations;
    IMeshes *Meshes;
    IBones *Bones;
    IAnimations *Animations;
    ITextureManager *TextureManager;
    QString filePath;
    QString fileName;
};

#endif // IMODELINTERFACE_H
