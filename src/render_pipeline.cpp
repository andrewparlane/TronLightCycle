#include "render_pipeline.hpp"
#include "frame_buffer.hpp"
#include "object_data.hpp"
#include "object.hpp"
#include "two_dimensional.hpp"
#include "shader.hpp"
#include "world.hpp"

#include <GL/glew.h>

#define BLUR_TEXTURE_DOWNSCALE_FACTOR 2

RenderPipeline::RenderPipeline(std::shared_ptr<const World> _world,
                               unsigned int _scrWidth, unsigned int _scrHeight)
    : lightingPassShader(Shader::getShader(SHADER_TYPE_LIGHTING_PASS)),
      hdrPassShader(Shader::getShader(SHADER_TYPE_HDR_PASS)),
      blurPassShader(Shader::getShader(SHADER_TYPE_BLUR)),
      world(_world), scrWidth(_scrWidth), scrHeight(_scrHeight),
      screenResolutionVec(scrWidth, scrHeight),
      blurOutputResolutionVec(screenResolutionVec / (float)BLUR_TEXTURE_DOWNSCALE_FACTOR),
      screenQuad(std::make_unique<ObjData2D>()),
      geometryPassFBO(std::make_unique<FrameBuffer>()),
      lightingPassFBO(std::make_unique<FrameBuffer>()),
      brightMultiSampleFBO(std::make_unique<FrameBuffer>()),
      brightFBO(std::make_unique<FrameBuffer>())
{
    blurFBOs[0] = std::make_unique<FrameBuffer>();
    blurFBOs[1] = std::make_unique<FrameBuffer>();
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
    renderLamps();
    doBlurPass();
    doHDRPass();
    render2D();
}

bool RenderPipeline::setupFBOs()
{
    FrameBufferTexture::TextureParameters params;

    // both the lighting pass and geometry pass FBOs use the same depth stencil texture, create it now
    // note no params
    std::shared_ptr<FrameBufferTexture> depthStencil = std::make_shared<FrameBufferTexture>(false, GL_DEPTH32F_STENCIL8, GL_DEPTH_STENCIL, GL_FLOAT_32_UNSIGNED_INT_24_8_REV, scrWidth, scrHeight, params, true);

    // and add it to both FBOs
    geometryPassFBO->addTexture(depthStencil);
    lightingPassFBO->addTexture(depthStencil);

    // the rest of the textures want to use these texture parameters
    params.push_back({ GL_TEXTURE_MIN_FILTER, GL_NEAREST });
    params.push_back({ GL_TEXTURE_MAG_FILTER, GL_NEAREST });

    // geometry pass textures
    //  position
    geometryPassFBO->addTexture(std::make_shared<FrameBufferTexture>(false, GL_RGB16F, GL_RGB, GL_FLOAT, scrWidth, scrHeight, params));
    //  normal
    geometryPassFBO->addTexture(std::make_shared<FrameBufferTexture>(false, GL_RGB16F, GL_RGB, GL_FLOAT, scrWidth, scrHeight, params));
    //  colour (LDR)
    geometryPassFBO->addTexture(std::make_shared<FrameBufferTexture>(false, GL_RGB, GL_RGB, GL_UNSIGNED_BYTE, scrWidth, scrHeight, params));

    // bind it
    if (!geometryPassFBO->assignAllTexturesToFBO())
    {
        printf("Failed to setup geometry psas FBO\n");
        return false;
    }

    // lighting pass textures
    //  colour (HDR)
    lightingPassFBO->addTexture(std::make_shared<FrameBufferTexture>(false, GL_RGB16F, GL_RGB, GL_FLOAT, scrWidth, scrHeight, params));

    // bind it
    if (!lightingPassFBO->assignAllTexturesToFBO())
    {
        printf("Failed to setup lighting psas FBO\n");
        return false;
    }

    // Bright FBO - multi sample
    //  colour (LDR, multi sample)
    brightMultiSampleFBO->addTexture(std::make_shared<FrameBufferTexture>(true, GL_RGB, GL_RGB, GL_UNSIGNED_BYTE, scrWidth, scrHeight, params));
    //  depth (multi sample)
    brightMultiSampleFBO->addTexture(std::make_shared<FrameBufferTexture>(true, GL_DEPTH32F_STENCIL8, GL_DEPTH_STENCIL, GL_FLOAT_32_UNSIGNED_INT_24_8_REV, scrWidth, scrHeight, params, true));

    // bind it
    if (!brightMultiSampleFBO->assignAllTexturesToFBO())
    {
        printf("Failed to setup bright FBO\n");
        return false;
    }

    // bright FBO - single sample
    // we clamp the texture UV co-ords at the edge (ie, don't repeat the texture).
    // this is needed as we want to manipulate all surrounding pixels to the current
    // frag co-ord, and we don't want to have to test if it is an edge case or not
    params.push_back({ GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE });
    params.push_back({ GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE });

    //  colour (LDR)
    brightFBO->addTexture(std::make_shared<FrameBufferTexture>(false, GL_RGB, GL_RGB, GL_UNSIGNED_BYTE, scrWidth, scrHeight, params));

    // bind it
    if (!brightFBO->assignAllTexturesToFBO())
    {
        printf("Failed to setup bright FBO\n");
        return false;
    }

    // Blur FBOs
    for (unsigned int i = 0; i < 2; i++)
    {
        //  colour (LDR) - only need LDR as the weights of the blur only add up to 1.0f
        blurFBOs[i]->addTexture(std::make_shared<FrameBufferTexture>(false, GL_RGB, GL_RGB, GL_UNSIGNED_BYTE, scrWidth / BLUR_TEXTURE_DOWNSCALE_FACTOR, scrHeight / BLUR_TEXTURE_DOWNSCALE_FACTOR, params));

        // bind it
        if (!blurFBOs[i]->assignAllTexturesToFBO())
        {
            printf("Failed to setup blur FBO[%d]\n", i);
            return false;
        }
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

void RenderPipeline::renderLamps() const
{
    // render lamps into bright FBO - multi sample
    brightMultiSampleFBO->bind();

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_CULL_FACE);

    glEnable(GL_MULTISAMPLE);

    glClear(GL_COLOR_BUFFER_BIT);

    world->drawLamps();

    glDisable(GL_MULTISAMPLE);

    brightFBO->bind();
    glClear(GL_COLOR_BUFFER_BIT);

    // blit it into the single sampled FBO
    brightMultiSampleFBO->bind(GL_READ_FRAMEBUFFER);
    brightFBO->bind(GL_DRAW_FRAMEBUFFER);
    glBlitFramebuffer(0, 0, scrWidth, scrHeight, 0, 0, scrWidth, scrHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

void RenderPipeline::doBlurPass() const
{
    glDisable(GL_BLEND);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);

    blurPassShader->useShader();

    // NUM_BLUR_PASSES each consisting of two stages:
    //   horiz - reads from [0] writes to [1] (except first pass which reads from brightFBO and writes to [1])
    //   vert  - reads from [1] writes to 0
    const unsigned int NUM_BLUR_PASSES = 1;
    bool first = true;
    for (unsigned int pass = 0; pass < NUM_BLUR_PASSES*2; pass++)
    {
        blurFBOs[(pass + 1) % 2]->bind();
        glClear(GL_COLOR_BUFFER_BIT);

        // bind the read input texture
        if (!first)
        {
            blurFBOs[pass % 2]->bindTextures();
        }
        else
        {
            brightFBO->bindTextures();
            first = false;
        }

        glUniform1i(blurPassShader->getUniformID(SHADER_UNIFORM_HORIZONTAL_FLAG), (pass + 1) % 2);
        glUniform1i(blurPassShader->getUniformID(SHADER_UNIFORM_COLOUR_TEXTURE_SAMPLER), 0);
        glUniform2fv(blurPassShader->getUniformID(SHARDER_UNIFORM_SCREEN_RES), 1, &blurOutputResolutionVec[0]);

        renderScreenQuad(blurPassShader);
    }
}

void RenderPipeline::doHDRPass() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glClear(GL_COLOR_BUFFER_BIT);

    glDisable(GL_BLEND);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);

    // draw a full screen quad this allows the HDR fragment shader to do tone mapping
    hdrPassShader->useShader();
    // bind textures from the lighting pass stage
    GLenum nextTextureToBind = lightingPassFBO->bindTextures();
    // and the blur stage (in the next available texture)
    // we always do an even number of blur passes, n * (horiz + vert)
    // horiz reads [0] and writes to [1], vert reads [1] and writes to [0]
    // therefore [0] always contains uor finished blur
    blurFBOs[0]->bindTextures(nextTextureToBind);

    glUniform1i(hdrPassShader->getUniformID(SHADER_UNIFORM_COLOUR_TEXTURE_SAMPLER), 0);
    glUniform1i(hdrPassShader->getUniformID(SHADER_UNIFORM_BLUR_TEXTURE_SAMPLER), 1);
    glUniform2fv(hdrPassShader->getUniformID(SHARDER_UNIFORM_SCREEN_RES), 1, &screenResolutionVec[0]);

    renderScreenQuad(hdrPassShader);
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
