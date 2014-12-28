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

#ifndef MODELINTERFACE_H
#define MODELINTERFACE_H

#include <QtCore>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "texture.h"

#include "animation.h"
#include "mesh.h"
#include "texturemanager.h"

class ModelInterface
{
public:
    ModelInterface(TextureManager* manager, const QString filename);
    ~ModelInterface();

    const bool hasAnimations() const { return has_animations; }

    Meshes*     getMeshes() const { return meshes; }
    Bones*      getBones() const { return bones; }
    Animations* getAnimations() const { return animations; }

    const QString getFilePath() const { return filePath; }
    const QString getFileName() const { return fileName; }

private:
    Bones*     loadBones(const aiScene* scene);
    Animation* loadAnimation(const aiAnimation* ai_animation, const int index);
    Mesh*      loadMesh(aiMesh* ai_mesh, aiMaterial* ai_material, const int index);

    void loadMaterial(aiMaterial* ai_material, Mesh* meshTarget);
    void buildSkeleton(aiNode* current, Bone* parent);

    static void addWeightData(QVector4D* boneIdTarget, QVector4D* weightTarget, float id, float w);
    static void loadTextures(aiMaterial* ai_material, aiTextureType type, int& index, bool& succes, QString filePath, TextureManager* manager);

    static QMatrix4x4 getMatrix(const aiMatrix4x4* m);

private:
    Meshes*         meshes;
    Bones*          bones;
    Animations*     animations;
    TextureManager* textureManager;

    QString filePath;
    QString fileName;

    bool has_animations;
};

#endif // MODELINTERFACE_H
