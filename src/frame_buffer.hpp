#ifndef __FRAME_BUFFER_HPP
#define __FRAME_BUFFER_HPP

#include <glm/glm.hpp>

#include <GL/glew.h>

#include <vector>
#include <memory>

class FrameBufferTexture
{
public:
    FrameBufferTexture(GLint internalFormat, GLenum format, GLenum type, const glm::vec2 &screenResolution, bool _isDepthStencil = false);
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

    void bindTextures() const;

protected:
    GLuint fbo;
    std::vector<std::shared_ptr<FrameBufferTexture>> textures;
};

#endif
