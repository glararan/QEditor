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

    void draw(IPipeline *Pipeline);
    bool isAnimationEnabled();
    void enableAnimations();
    void disableAnimations();

    IModelInterface *getModelInterface();
    IAnimationState *getAnimationState();

private:
    QOpenGLFunctions_3_1 *m_funcs;
    QOpenGLShaderProgram *m_shader;
    ITextureManager *texture_manager;
    IModelInterface *model_interface;
    IAnimationState *animation_state;
    bool animations_enabled;
    ILight Light;

private:
    bool initializeShaders();
};

#endif // IMODEL_H
