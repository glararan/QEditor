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
    void loadMaterial(aiMaterial *ai_material, IMesh *meshTarget);
    void buildSkeleton(aiNode *current, IBone *parent);

    static void loadTextures(aiMaterial *ai_material, aiTextureType type, int &index, bool &succes, QString filePath, ITextureManager *textureManager);
    static void addWeightData(QVector4D * boneIdTarget, QVector4D * weightTarget, float id, float w);
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
