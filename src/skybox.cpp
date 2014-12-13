#include "skybox.h"

Skybox::Skybox(QOpenGLFunctions_4_2_Core* funcs)
: GLfuncs(funcs)
, shaderProgram(0)
, sampler(new Sampler())
, texture(new Texture(QOpenGLTexture::TargetCubeMap))
, material(new Material())
{
    sampler->create();
    sampler->setMinificationFilter(GL_LINEAR);
    sampler->setMagnificationFilter(GL_LINEAR);
    sampler->setWrapMode(Sampler::DirectionS, GL_CLAMP_TO_EDGE);
    sampler->setWrapMode(Sampler::DirectionT, GL_CLAMP_TO_EDGE);
    sampler->setWrapMode(Sampler::DirectionR, GL_CLAMP_TO_EDGE);

    texture->setFormat(QOpenGLTexture::RGBA8_UNorm);

    material->setTextureUnitConfiguration(18, texture, sampler, QByteArray("cubeMap"));

    float data[] =
    {
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };

    mesh.createVertexArrayObject();
    mesh.createBuffer(IMesh::Vertices, &data, sizeof(data));
    mesh.setNumFaces(36);
}

Skybox::~Skybox()
{
    texture->destroy();
    sampler->destroy();

    delete material;
}

void Skybox::draw(QOpenGLShaderProgram* shader)
{
    createAttributeArray(shader);

    material->bind(shader);

    mesh.bind();

    GLfuncs->glDrawArrays(GL_TRIANGLES, 0, mesh.getNumFaces());
}

void Skybox::loadSide(QOpenGLTexture::CubeMapFace side, QString fileName)
{
    texture->bind();

    QImage glImage = QGLWidget::convertToGLFormat(QImage(fileName).mirrored());

    GLfuncs->glTexImage2D(side, 0, texture->format(), glImage.width(), glImage.height(), 0, QOpenGLTexture::RGBA, QOpenGLTexture::UInt8, glImage.bits());
}

void Skybox::createAttributeArray(QOpenGLShaderProgram* shader)
{
    if(shaderProgram == shader)
        return;

    mesh.getVertexArrayObject()->bind();
    mesh.createAttributeArray(IMesh::Vertices, shader, "qt_Vertex", GL_FLOAT, 0, 3);

    shaderProgram = shader;
}