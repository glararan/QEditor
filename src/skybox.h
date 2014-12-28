#ifndef SKYBOX_H
#define SKYBOX_H

#include <QtCore>
#include <QtOpenGL>
#include <QOpenGLFunctions_4_2_Core>

#include "texture.h"
#include "sampler.h"
#include "material.h"
#include "mesh.h"

class Skybox
{
public:
    explicit Skybox(QOpenGLFunctions_4_2_Core* funcs);
    ~Skybox();

    enum SkyboxSide
    {
        Right  = 0,
        Left   = 1,
        Top    = 2,
        Bottom = 3,
        Front  = 4,
        Back   = 5
    };

    void draw(QOpenGLShaderProgram* shader);

    void loadSide(QOpenGLTexture::CubeMapFace side, const QString fileName);

    Material* getMaterial() { return material; }

private:
    QOpenGLFunctions_4_2_Core* GLfuncs;

    QOpenGLShaderProgram* shaderProgram;

    SamplerPtr sampler;
    TexturePtr texture;

    Material* material;

    Mesh mesh;

    void createAttributeArray(QOpenGLShaderProgram* shader);
};

#endif // SKYBOX_H
