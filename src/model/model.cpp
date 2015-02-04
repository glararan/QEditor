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

#include "model.h"

#include <QOpenGLFunctions_4_2_Core>

Model::Model(ModelManager* modelManager, ModelInterface* modelInterface)
: lastShader(0)
, animation_state(0)
, animations_enabled(true)
, texture_manager(modelManager->getTextureManager())
, model_interface(modelInterface)
{
    GLfuncs = QOpenGLContext::currentContext()->versionFunctions<QOpenGLFunctions_4_2_Core>();

    if(!GLfuncs)
    {
        qWarning() << "Requires multi-texturing support";

        exit(-1);
    }

    GLfuncs->initializeOpenGLFunctions();

    if(model_interface->hasAnimations())
        animation_state = new AnimationState(model_interface->getAnimations());
}

Model::~Model()
{
    delete animation_state;
}

void Model::draw(QOpenGLShaderProgram* shader)
{
    createAttributeArray(shader);

    shader->setUniformValue("qt_lPosition",   light.position);
    shader->setUniformValue("qt_lAmbient",    light.ambient);
    shader->setUniformValue("qt_lDiffuse",    light.diffuse);
    shader->setUniformValue("qt_lSpecular",   light.specular);
    shader->setUniformValue("diffuseTexture", 0);

    if(model_interface->hasAnimations() && animations_enabled)
    {
        shader->setUniformValue("animationEnabled", true);
        shader->setUniformValueArray("boneMatrix", animation_state->getTransforms().data(), 100);
    }
    else
        shader->setUniformValue("animationEnabled", false);

    Meshes* meshes = model_interface->getMeshes();

    for(int i = 0; i < meshes->size(); ++i)
    {
        Mesh* mesh = meshes->at(i);

        shader->setUniformValue("qt_mAmbient",  mesh->getMeshMaterial()->mAmbient);
        shader->setUniformValue("qt_mDiffuse",  mesh->getMeshMaterial()->mDiffuse);
        shader->setUniformValue("qt_mSpecular", mesh->getMeshMaterial()->mSpecular);
        shader->setUniformValue("qt_Shininess", mesh->getMeshMaterial()->shininess);
        shader->setUniformValue("qt_Opacity",   mesh->getMeshMaterial()->opacity);

        shader->setUniformValue("hasDiffuse", mesh->getMeshTextures()->hasDiffuseTexture);

        if(mesh->getMeshTextures()->hasDiffuseTexture)
        {
            GLfuncs->glActiveTexture(GL_TEXTURE0);

            texture_manager->getTexture(mesh->getMeshTextures()->diffuseTextureIndex)->bind();
            texture_manager->getSampler()->bind(0);
        }

        if(mesh->getMeshMaterial()->opacity != 1.0f)
        {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }

        mesh->bind();

        glDrawElements(GL_TRIANGLES, mesh->getNumFaces(), GL_UNSIGNED_INT, 0);

        if(mesh->getMeshMaterial()->opacity != 1.0f)
            glDisable(GL_BLEND);
    }
}

void Model::enableAnimations()
{
    animations_enabled = true;
}

void Model::disableAnimations()
{
    animations_enabled = false;
}

void Model::createAttributeArray(QOpenGLShaderProgram* shader)
{
    if(lastShader == shader)
        return;

    model_interface->getMeshes()->createAttributeArray(shader);

    lastShader = shader;
}
