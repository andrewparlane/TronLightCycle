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

    FrameBufferTexture(GLint internalFormat, GLenum format, GLenum type, unsigned int scrWidth, unsigned int scrHeight, const TextureParameters &parameters, bool _isDepthStencil = false);
    ~FrameBufferTexture();

    GLuint get() const { return texture; }
    bool getIsDepthStencil() const { return isDepthStencil; }

protected:
    bool isDepthStencil;
    GLuint texture;
};

class FrameBuffer
{
public:
    FrameBuffer();
    ~FrameBuffer();

    void addTexture(std::shared_ptr<FrameBufferTexture> texture);

    bool assignAllTexturesToFBO() const;

    void bind() const;

    GLenum bindTextures(GLenum startTexture = GL_TEXTURE0) const;

protected:
    GLuint fbo;
    std::vector<std::shared_ptr<FrameBufferTexture>> textures;
};

#endif
