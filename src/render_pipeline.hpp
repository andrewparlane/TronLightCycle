#ifndef __RENDER_PIPELINE_HPP
#define __RENDER_PIPELINE_HPP

#include <glm/glm.hpp>

#include <memory>
#include <vector>

class Object;
class Object2D;
class ObjData2D;
class FrameBuffer;
class Shader;
class World;

class RenderPipeline
{
public:
    RenderPipeline(std::shared_ptr<const World> _world,
                   unsigned int _scrWidth, unsigned int _scrHeight);
    ~RenderPipeline();

    bool initialise();

    void add3DObject(std::shared_ptr<Object> obj);
    void add2DObject(std::shared_ptr<Object2D> obj);

    void render() const;

protected:
    bool setupFBOs();
    bool setupScreenQuad();

    void doGeometryPass() const;
    void doLightingPass() const;
    void doHDRPass() const;
    void renderLamps() const;
    void render2D() const;

    void renderScreenQuad(std::shared_ptr<const Shader> shader) const;

    std::shared_ptr<const Shader> lightingPassShader;
    std::shared_ptr<const Shader> hdrPassShader;
    std::shared_ptr<const World> world;
    unsigned int scrWidth;
    unsigned int scrHeight;
    glm::vec2 screenResolutionVec;

    std::vector<std::shared_ptr<Object>> objects3D;
    std::vector<std::shared_ptr<Object2D>> objects2D;

    std::unique_ptr<ObjData2D> screenQuad;

    // Frame buffer objects (FBOs)
    std::unique_ptr<FrameBuffer> geometryPassFBO;
    std::unique_ptr<FrameBuffer> lightingPassFBO;
};

#endif
