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

#ifndef MODEL_H
#define MODEL_H

#include "texturemanager.h"
#include "modelmanager.h"
#include "modelinterface.h"
#include "animation.h"
#include "pipeline.h"
#include "light.h"

class QOpenGLFunctions_4_2_Core;

class Model
{
public:
    Model(ModelManager* modelManager, const int index);
    ~Model();

    void draw(QOpenGLShaderProgram* shader);

    void enableAnimations();
    void disableAnimations();

    const bool isAnimationEnabled() const { return animations_enabled; }

    ModelInterface* getModelInterface() const { return model_interface; }
    AnimationState* getAnimationState() const { return animation_state; }

    const QVector3D& getCenter() const;

private:
    QOpenGLFunctions_4_2_Core* GLfuncs;
    QOpenGLShaderProgram*      lastShader;

    TextureManager* texture_manager;
    ModelInterface* model_interface;
    AnimationState* animation_state;

    Light light;

    bool animations_enabled;

    void createAttributeArray(QOpenGLShaderProgram* shader);
};

#endif // MODEL_H
