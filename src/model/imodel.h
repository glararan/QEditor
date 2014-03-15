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

#ifndef IMODEL_H
#define IMODEL_H

#include "itexturemanager.h"
#include "imodelmanager.h"
#include "imodelinterface.h"
#include "ianimation.h"
#include "ipipeline.h"
#include "ilight.h"

class QOpenGLFunctions_3_1;

class IModel
{
public:
    IModel(IModelManager *modelManager, int index);
    ~IModel();

    void draw(QOpenGLShaderProgram *shader);
    bool isAnimationEnabled();
    void enableAnimations();
    void disableAnimations();

    IModelInterface *getModelInterface();
    IAnimationState *getAnimationState();

private:
    QOpenGLFunctions_3_1 *m_funcs;
    QOpenGLShaderProgram *lastShader;
    ITextureManager *texture_manager;
    IModelInterface *model_interface;
    IAnimationState *animation_state;
    bool animations_enabled;
    ILight Light;

private:
    void createAttributeArray(QOpenGLShaderProgram *shader);
};

#endif // IMODEL_H
