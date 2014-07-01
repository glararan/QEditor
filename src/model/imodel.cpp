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

#include "imodel.h"

#include <QOpenGLFunctions_3_1>

IModel::IModel(IModelManager* modelManager, int index)
: lastShader(0)
, animation_state(0)
, animations_enabled(true)
, texture_manager(modelManager->getTextureManager())
, model_interface(modelManager->getModel(index)->modelInterface)
{
    m_funcs = QOpenGLContext::currentContext()->versionFunctions<QOpenGLFunctions_3_1>();

    if(!m_funcs)
    {
        qWarning() << "Requires multi-texturing support";
        exit(-1);
    }

    m_funcs->initializeOpenGLFunctions();

    if(model_interface->hasAnimations())
        animation_state = new IAnimationState(model_interface->getAnimations());
}

IModel::~IModel()
{
    delete animation_state;
}

void IModel::draw(QOpenGLShaderProgram* shader)
{
    createAttributeArray(shader);

    shader->setUniformValue("qt_lPosition",   Light.position);
    shader->setUniformValue("qt_lAmbient",    Light.ambient);
    shader->setUniformValue("qt_lDiffuse",    Light.diffuse);
    shader->setUniformValue("qt_lSpecular",   Light.specular);
    shader->setUniformValue("diffuseTexture", 0);

    if(model_interface->hasAnimations() && animations_enabled)
    {
        shader->setUniformValue("animationEnabled", true);
        shader->setUniformValueArray("boneMatrix", animation_state->getTransforms().data(), 100);
    }
    else
        shader->setUniformValue("animationEnabled", false);

    IMeshes* meshes = model_interface->getMeshes();

    for(int i = 0; i < meshes->size(); ++i)
    {
        IMesh* mesh = meshes->at(i);

        shader->setUniformValue("qt_mAmbient",  mesh->getMeshMaterial()->mAmbient);
        shader->setUniformValue("qt_mDiffuse",  mesh->getMeshMaterial()->mDiffuse);
        shader->setUniformValue("qt_mSpecular", mesh->getMeshMaterial()->mSpecular);
        shader->setUniformValue("qt_Shininess", mesh->getMeshMaterial()->shininess);
        shader->setUniformValue("qt_Opacity",   mesh->getMeshMaterial()->opacity);

        shader->setUniformValue("hasDiffuse",mesh->getMeshTextures()->hasDiffuseTexture);

        if(mesh->getMeshTextures()->hasDiffuseTexture)
        {
            m_funcs->glActiveTexture(GL_TEXTURE0);

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

bool IModel::isAnimationEnabled()
{
    return animations_enabled;
}

void IModel::enableAnimations()
{
    animations_enabled = true;
}

void IModel::disableAnimations()
{
    animations_enabled = false;
}

IModelInterface* IModel::getModelInterface()
{
    return model_interface;
}

IAnimationState* IModel::getAnimationState()
{
    return animation_state;
}

void IModel::createAttributeArray(QOpenGLShaderProgram* shader)
{
    if(lastShader == shader)
        return;

    model_interface->getMeshes()->createAttributeArray(shader);

    lastShader = shader;
}