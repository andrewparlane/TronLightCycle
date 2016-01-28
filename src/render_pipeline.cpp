#include "render_pipeline.hpp"
#include "frame_buffer.hpp"
#include "object_data.hpp"
#include "object.hpp"
#include "two_dimensional.hpp"
#include "shader.hpp"
#include "world.hpp"

#include <GL/glew.h>

RenderPipeline::RenderPipeline(std::shared_ptr<const World> _world,
                               unsigned int _scrWidth, unsigned int _scrHeight)
    : lightingPassShader(Shader::getShader(SHADER_TYPE_LIGHTING_PASS)),
      hdrPassShader(Shader::getShader(SHADER_TYPE_HDR_PASS)),
      world(_world), scrWidth(_scrWidth), scrHeight(_scrHeight),
      screenResolutionVec(scrWidth, scrHeight),
      geometryPassFBO(std::make_unique<FrameBuffer>()),
      lightingPassFBO(std::make_unique<FrameBuffer>()),
      screenQuad(std::make_unique<ObjData2D>())
{
}

RenderPipeline::~RenderPipeline()
{
}

bool RenderPipeline::initialise()
{
    return setupFBOs() &&
           setupScreenQuad();
}

void RenderPipeline::add3DObject(std::shared_ptr<Object> obj)
{
    objects3D.push_back(obj);
}

void RenderPipeline::add2DObject(std::shared_ptr<Object2D> obj)
{
    objects2D.push_back(obj);
}

void RenderPipeline::render() const
{
    doGeometryPass();
    doLightingPass();
    doHDRPass();
    renderLamps();
    render2D();
}

bool RenderPipeline::setupFBOs()
{
    FrameBufferTexture::TextureParameters params;

    // both the lighting pass and geometry pass FBOs use the same depth stencil texture, create it now
    // note no params
    std::shared_ptr<FrameBufferTexture> depthStencil = std::make_shared<FrameBufferTexture>(GL_DEPTH32F_STENCIL8, GL_DEPTH_STENCIL, GL_FLOAT_32_UNSIGNED_INT_24_8_REV, scrWidth, scrHeight, params, true);

    // and add it to both FBOs
    geometryPassFBO->addTexture(depthStencil);
    lightingPassFBO->addTexture(depthStencil);

    // the rest of the textures want to use these texture parameters
    params.push_back({ GL_TEXTURE_MIN_FILTER, GL_NEAREST });
    params.push_back({ GL_TEXTURE_MAG_FILTER, GL_NEAREST });

    // geometry pass textures
    //  position
    geometryPassFBO->addTexture(std::make_shared<FrameBufferTexture>(GL_RGB16F, GL_RGB, GL_FLOAT, scrWidth, scrHeight, params));
    //  normal
    geometryPassFBO->addTexture(std::make_shared<FrameBufferTexture>(GL_RGB16F, GL_RGB, GL_FLOAT, scrWidth, scrHeight, params));
    //  colour (LDR)
    geometryPassFBO->addTexture(std::make_shared<FrameBufferTexture>(GL_RGB, GL_RGB, GL_UNSIGNED_BYTE, scrWidth, scrHeight, params));

    // bind it
    if (!geometryPassFBO->assignAllTexturesToFBO())
    {
        printf("Failed to setup geometry psas FBO\n");
        return false;
    }

    // lighting pass textures
    //  colour (HDR)
    lightingPassFBO->addTexture(std::make_shared<FrameBufferTexture>(GL_RGB16F, GL_RGB, GL_FLOAT, scrWidth, scrHeight, params));

    // bind it
    if (!lightingPassFBO->assignAllTexturesToFBO())
    {
        printf("Failed to setup lighting psas FBO\n");
        return false;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return true;
}

bool RenderPipeline::setupScreenQuad()
{
    MeshData<glm::vec2> md;
    md.hasTexture = false;

    md.vertices.push_back(glm::vec2(-1.0f, -1.0f));
    md.vertices.push_back(glm::vec2(-1.0f,  1.0f));
    md.vertices.push_back(glm::vec2( 1.0f,  1.0f));
    md.vertices.push_back(glm::vec2( 1.0f, -1.0f));

    md.indices.push_back(0); md.indices.push_back(1); md.indices.push_back(2);
    md.indices.push_back(0); md.indices.push_back(2); md.indices.push_back(3);

    if (!screenQuad->addMesh(md))
    {
        printf("Failed to set up screen quad\n");
        return false;
    }
    return true;
}

void RenderPipeline::doGeometryPass() const
{
    // bind the geometry pass FBO
    geometryPassFBO->bind();

    // enable both depth and stencil buffers for writting
    // needed here so we can clear them in glClear below
    glDepthMask(GL_TRUE);
    glStencilMask(0xFF);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);

    // enable the stencil test to always increment
    // this means the stencil buffer will be non 0 for any rendered pixel
    // leaving it 0 for only the background (non rendered pixels)
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 0, 0);
    glStencilOp(GL_INCR, GL_INCR, GL_INCR);

    glDisable(GL_BLEND);
    glDisable(GL_CULL_FACE);

    // draw objects
    for (const auto &obj : objects3D)
    {
        obj->drawAll();
    }
}

void RenderPipeline::doLightingPass() const
{
    lightingPassFBO->bind();
    glClear(GL_COLOR_BUFFER_BIT);

    // disable writing to the stencil buffer
    // set stencil test to pass if the stencil value != 0
    // ie. we are an actual object and not the background
    glStencilMask(0x00);
    glStencilFunc(GL_NOTEQUAL, 0, 0xff);

    // disable depth test and writing to depth buffer
    glDepthMask(GL_FALSE);
    glDisable(GL_DEPTH_TEST);

    // enable blending - results of each light get blended together
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE);

    // cull front facing faces of spheres, this stops us rendering each light twice,
    // and works if we are in the light sphere too
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);

    lightingPassShader->useShader();

    // bind the geometry pass textures, so we can sample them in our shader
    // this gives the fragment shader access to position, normal and colour info
    // note: they get bound in the order they were added,
    //       IE. if position texture was first, it bound to GL_TEXTURE0
    geometryPassFBO->bindTextures();

    // set screen resolution
    glUniform2fv(lightingPassShader->getUniformID(SHARDER_UNIFORM_SCREEN_RES), 1, &screenResolutionVec[0]);

    world->sendLightingInfoToShader(lightingPassShader);
}

void RenderPipeline::doHDRPass() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glDepthMask(GL_TRUE); // need to enable so we can clear the depth buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDepthMask(GL_FALSE); // disable depth mask again

    glDisable(GL_BLEND);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);

    // draw a full screen quad this allows the HDR fragment shader to do tone mapping
    hdrPassShader->useShader();
    // bind textures from the lighting pass stage
    lightingPassFBO->bindTextures();

    glUniform1i(hdrPassShader->getUniformID(SHADER_UNIFORM_COLOUR_TEXTURE_SAMPLER), 0);
    glUniform2fv(hdrPassShader->getUniformID(SHARDER_UNIFORM_SCREEN_RES), 1, &screenResolutionVec[0]);

    renderScreenQuad(hdrPassShader);
}

void RenderPipeline::renderLamps() const
{
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    world->drawLamps();
}

void RenderPipeline::render2D() const
{
    glDepthMask(GL_FALSE);
    glDisable(GL_DEPTH_TEST);

    // draw objects
    for (const auto &obj : objects2D)
    {
        obj->drawAll();
    }
}

void RenderPipeline::renderScreenQuad(std::shared_ptr<const Shader> shader) const
{
    GLuint vertexPosition_ScreenID = shader->getAttribID(SHADER_ATTRIB_VERTEX_POS);

    auto sqMeshes = screenQuad->getMeshes();
    for (auto &it : sqMeshes)
    {
        glEnableVertexAttribArray(vertexPosition_ScreenID);

        glBindBuffer(GL_ARRAY_BUFFER, it->vertexBuffer);
        glVertexAttribPointer(vertexPosition_ScreenID, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, it->indiceBuffer);
        glDrawElements(GL_TRIANGLES, it->numIndices, GL_UNSIGNED_SHORT, (void *)0);

        glDisableVertexAttribArray(vertexPosition_ScreenID);
    }
}
