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
    LightTrail(std::shared_ptr<World> _world, 
               std::shared_ptr<const Shader> _shader,
               glm::vec3 _colour);
    ~LightTrail();

    void toggle();

    void turn(float bikeAngleRadians);
    void stopTurning();
    void updateLastVertices(glm::vec3 bikeLocationWorld);   // not passing by const & as we store it

    void update();
    void drawAll() const { if (lightTrailMeshData.size() > 0) lightTrail->drawAll(); }

protected:
    enum State
    {
        STATE_STOPPED = 0,
        STATE_ON,
        STATE_STOPPING,
    };


    std::shared_ptr<World> world;
    std::shared_ptr<const Shader> shader;
    glm::vec3 colour;

    std::vector<MeshData<glm::vec3>> lightTrailMeshData;
    std::shared_ptr<ObjData3D> lightTrailObjData;
    std::unique_ptr<Object> lightTrail;

    glm::vec3 bikeLocation;
    unsigned int trailNumber;
    State state;
    bool turning;
};

#endif
