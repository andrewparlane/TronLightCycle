#ifndef __FRAME_BUFFER_HPP
#define __FRAME_BUFFER_HPP

#include <glm/glm.hpp>

#include <GL/glew.h>

#include <vector>
#include <memory>

class FrameBufferTexture
{
public:
    typedef std::vector<std::pair<GLenum, GLint>> TextureParameters;

    FrameBufferTexture(bool multiSample, GLint internalFormat, GLenum format, GLenum type, unsigned int scrWidth, unsigned int scrHeight, const TextureParameters &parameters, bool _isDepthStencil = false);
    ~FrameBufferTexture();

    void bind() const;
    void attachToFBO(GLenum attachmentPoint) const;

    bool getIsDepthStencil() const { return isDepthStencil; }

protected:
    bool isDepthStencil;
    GLuint texture;
    GLenum target;
};

class FrameBuffer
{
public:
    FrameBuffer();
    ~FrameBuffer();

    void addTexture(std::shared_ptr<FrameBufferTexture> texture);

    bool assignAllTexturesToFBO() const;

    void bind(GLenum frameBufferType = GL_FRAMEBUFFER) const;

    GLenum bindTextures(GLenum startTexture = GL_TEXTURE0) const;

protected:
    GLuint fbo;
    std::vector<std::shared_ptr<FrameBufferTexture>> textures;
};

#endif
