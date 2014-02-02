#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <QtCore>

class QOpenGLFunctions_4_2_Core;

class Framebuffer
{
public:
    Framebuffer(int width, int height);
    ~Framebuffer();

    void bind();
    void release();

    unsigned int texture();
    unsigned int depthTexture();

    int width();
    int height();

private:
    unsigned int textureId;
    unsigned int depthTextureId;
    unsigned int fbo;

    QSize textureSize;

private:
    QOpenGLFunctions_4_2_Core* GLfuncs;
};

#endif // FRAMEBUFFER_H