#ifndef __LIGHT_TRAIL_HPP
#define __LIGHT_TRAIL_HPP

#include "object_data.hpp"
#include "object.hpp"
#include "world.hpp"
#include "shader.hpp"

#include <memory>

class LightTrail
{
public:
    LightTrail();
    ~LightTrail();

    bool initialise(std::shared_ptr<World> _world, 
                    std::shared_ptr<const Shader> _shader,
                    glm::vec3 colour);

    void turn(float bikeAngleRadians);
    void stopTurning();
    void updateLastVertices(const glm::vec3 &bikeLocationWorld);

    void update() { lightTrailObjData->updateMesh(lightTrailMeshData); lightTrailObjData->updateBuffers(); }
    void drawAll() { lightTrail->drawAll(); }

protected:
    MeshData lightTrailMeshData;
    std::shared_ptr<ObjData> lightTrailObjData;
    std::unique_ptr<Object> lightTrail;

    bool turning;
};

#endif
