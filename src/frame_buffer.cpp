#include "frame_buffer.hpp"

#include <stdlib.h>

FrameBufferTexture::FrameBufferTexture(GLint internalFormat, GLenum format, GLenum type, unsigned int scrWidth, unsigned int scrHeight, const TextureParameters &parameters, bool _isDepthStencil)
    : isDepthStencil(_isDepthStencil)
{
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, scrWidth, scrHeight, 0, format, type, NULL);

    for (auto &param : parameters)
    {
        glTexParameteri(GL_TEXTURE_2D, param.first, param.second);
    }
}

FrameBufferTexture::~FrameBufferTexture()
{
    glDeleteTextures(1, &texture);
}

// -----------------------------------------------------------------------

FrameBuffer::FrameBuffer()
{
    glGenFramebuffers(1, &fbo);
}

FrameBuffer::~FrameBuffer()
{
    glDeleteFramebuffers(1, &fbo);
}

void FrameBuffer::addTexture(std::shared_ptr<FrameBufferTexture> texture)
{
    textures.push_back(texture);
}

bool FrameBuffer::assignAllTexturesToFBO() const
{
    // bind the frame buffer
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // attach the textures
    GLenum currentAttachementPoint = GL_COLOR_ATTACHMENT0;
    std::vector<GLenum> attachmentPoints;
    for (const auto &texture : textures)
    {
        unsigned int ap;
        if (texture->getIsDepthStencil())
        {
            ap = GL_DEPTH_STENCIL_ATTACHMENT;
        }
        else
        {
            ap = currentAttachementPoint++;
            attachmentPoints.push_back(ap);
        }

        glFramebufferTexture2D(GL_FRAMEBUFFER, ap, GL_TEXTURE_2D, texture->get(), 0);
    }

    // tell openGL which attachment points we are using
    glDrawBuffers(attachmentPoints.size(), &attachmentPoints[0]);

    // check all is well
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        printf("Framebuffer not complete!\n");
        return false;
    }

    return true;
}

void FrameBuffer::bind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
}

GLenum FrameBuffer::bindTextures(GLenum startTexture) const
{
    // bind all except the depth / stencil textures
    // not this FBO doesn't need to be bound first
    // and most often will not be

    GLenum textureID = startTexture;
    for (const auto &texture : textures)
    {
        if (!texture->getIsDepthStencil())
        {
            glActiveTexture(textureID++);
            glBindTexture(GL_TEXTURE_2D, texture->get());
        }
    }

    // return next texture ID to be used
    return textureID;
}
