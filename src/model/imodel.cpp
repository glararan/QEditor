#include "imodel.h"
#include <qopenglfunctions_3_1.h>

IModel::IModel(IModelManager *modelManager, int index) :
    animation_state(0),
    animations_enabled(true),
    texture_manager(modelManager->getTextureManager()),
    model_interface(modelManager->getModel(index)->modelInterface)
{
    m_funcs = QOpenGLContext::currentContext()->versionFunctions<QOpenGLFunctions_3_1>();
    if(!m_funcs)
    {
        qWarning() << "Requires multi-texturing support";
        exit(-1);
    }
    m_funcs->initializeOpenGLFunctions();

    m_shader = new QOpenGLShaderProgram();
    if(!initializeShaders())
        exit(-1);
    m_shader->bind();

    m_shader->setUniformValue("qt_lPosition",Light.position);
    m_shader->setUniformValue("qt_lAmbient",Light.ambient);
    m_shader->setUniformValue("qt_lDiffuse",Light.diffuse);
    m_shader->setUniformValue("qt_lSpecular",Light.specular);
    m_shader->setUniformValue("diffuseTexture",0);

    IMeshes *meshes = model_interface->getMeshes();
    for(int i = 0; i < meshes->size(); ++i)
    {
        IMesh *mesh = meshes->at(i);
        IMeshes::createAttributeArray(m_shader,mesh);
    }

    if(model_interface->hasAnimations())
        animation_state = new IAnimationState(model_interface->getAnimations());
}

IModel::~IModel()
{
    delete animation_state;
    delete m_shader;
}

void IModel::draw(IPipeline *Pipeline)
{
    m_shader->bind();
    Pipeline->updateMatrices(m_shader);

    if(model_interface->hasAnimations() && animations_enabled)
    {
        m_shader->setUniformValue("animationEnabled",true);
        m_shader->setUniformValueArray("boneMatrix",animation_state->getTransforms().data(),100);
    }
    else
    {
        m_shader->setUniformValue("animationEnabled",false);
    }

    IMeshes *meshes = model_interface->getMeshes();
    for(int i = 0; i < meshes->size(); ++i)
    {
        IMesh *mesh = meshes->at(i);

        m_shader->setUniformValue("qt_mAmbient",mesh->mAmbient);
        m_shader->setUniformValue("qt_mDiffuse",mesh->mDiffuse);
        m_shader->setUniformValue("qt_mSpecular",mesh->mSpecular);
        m_shader->setUniformValue("qt_Shininess",mesh->shininess);
        m_shader->setUniformValue("qt_Opacity",mesh->opacity);

        m_shader->setUniformValue("hasDiffuse",mesh->hasDiffuse);
        if(mesh->hasDiffuse)
        {
            m_funcs->glActiveTexture(GL_TEXTURE0);
            texture_manager->getTexture(mesh->diffuseIndex)->bind();
            texture_manager->getSampler()->bind(0);
        }

        if(mesh->opacity != 1.0f)
        {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
        mesh->getVertexArrayObject()->bind();
        mesh->getIndexBuffer()->bind();
        glDrawElements(GL_TRIANGLES, mesh->numFaces, GL_UNSIGNED_INT, 0);
        if(mesh->opacity != 1.0f)
        {
            glDisable(GL_BLEND);
        }
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

IModelInterface *IModel::getModelInterface()
{
    return model_interface;
}

IAnimationState *IModel::getAnimationState()
{
    return animation_state;
}

bool IModel::initializeShaders()
{
    if(!m_shader->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/data/shaders/model.vert"))
    {
        qDebug() << QObject::tr("Could not compile vertex shader. Log:") << m_shader->log();
        return false;
    }

    if(!m_shader->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/data/shaders/model.frag"))
    {
        qDebug() << QObject::tr("Could not compile fragment shader. Log:") << m_shader->log();
        return false;
    }

    if(!m_shader->link())
    {
        qDebug() << QObject::tr("Could not link shader program. Log:") << m_shader->log();
        return false;
    }
    return true;
}
