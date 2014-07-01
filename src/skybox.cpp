#include "skybox.h"

Skybox::Skybox(QOpenGLFunctions_4_2_Core* funcs)
: GLfuncs(funcs)
, shaderProgram(0)
, sampler(new Sampler())
, texture(new Texture(QOpenGLTexture::TargetCubeMap))
{
    sampler->create();
    sampler->setMinificationFilter(GL_LINEAR);
    sampler->setMagnificationFilter(GL_LINEAR);
    sampler->setWrapMode(Sampler::DirectionS, GL_CLAMP_TO_EDGE);
    sampler->setWrapMode(Sampler::DirectionT, GL_CLAMP_TO_EDGE);

    texture->setFormat(QOpenGLTexture::RGBA8_UNorm);

    material.setTextureUnitConfiguration(0, texture, sampler, QByteArray("cubeMap"));

    float data[] =
    {
        -1.0,  1.0,  1.0,
        -1.0, -1.0,  1.0,
         1.0, -1.0,  1.0,
         1.0,  1.0,  1.0,
        -1.0,  1.0, -1.0,
        -1.0, -1.0, -1.0,
         1.0, -1.0, -1.0,
         1.0,  1.0, -1.0
    };

    unsigned int indices[] =
    {
        0, 1, 2, 0, 3, 2,
        2, 3, 6, 3, 7, 6,
        4, 5, 6, 4, 7, 6,
        0, 1, 5, 0, 4, 5,
        0, 3, 4, 3, 4, 7,
        1, 2, 6, 1, 5, 6
    };

    mesh.createVertexArrayObject();
    mesh.createBuffer(IMesh::Vertices, data,    sizeof(data));
    mesh.createBuffer(IMesh::Index,    indices, sizeof(indices));
    mesh.setNumFaces(36);
}

Skybox::~Skybox()
{
}

void Skybox::draw(QOpenGLShaderProgram* shader)
{
    createAttributeArray(shader);

    material.bind(shader);

    mesh.bind();

    GLfuncs->glDrawElements(GL_TRIANGLES, mesh.getNumFaces(), GL_UNSIGNED_SHORT, 0);
}

void Skybox::loadSide(QOpenGLTexture::CubeMapFace side, QString fileName)
{
    texture->bind();

    QImage glImage = QGLWidget::convertToGLFormat(QImage(fileName).mirrored());

    GLfuncs->glTexImage2D(side, 0, texture->format(), glImage.width(), glImage.height(), 0, QOpenGLTexture::RGBA8_UNorm, QOpenGLTexture::UInt8, glImage.bits());

    material.setTextureUnitConfiguration(0, texture, sampler, QByteArray("cubeMap"));
}

void Skybox::createAttributeArray(QOpenGLShaderProgram* shader)
{
    if(shaderProgram == shader)
        return;

    mesh.getVertexArrayObject()->bind();
    mesh.createAttributeArray(IMesh::Vertices, shader, "qt_Vertex", GL_FLOAT, 0, 3);

    shaderProgram = shader;
}