#ifndef __LIGHT_TRAIL_HPP
#define __LIGHT_TRAIL_HPP

#include "object_data.hpp"
#include "object.hpp"
#include "world.hpp"
#include "shader.hpp"
#include "bike_movements.hpp"

#include <memory>

class LightTrail
{
public:
    LightTrail(std::shared_ptr<World> _world, 
               std::shared_ptr<const Shader> _shader,
               glm::vec3 _colour,
               TurnDirection turning,
               Accelerating accelerating);
    ~LightTrail();

    void update(TurnDirection turning, Accelerating accelerating, float speed, glm::vec3 currentLocation, float currentAngleRads);

    // start fading down the trail
    void stop() { stopping = true; }

    // have we fully faded down the trail?
    // if so we can delete it
    bool isDead() const { return isStopped; }

    void draw() const { if (lightTrailObj) lightTrailObj->drawAll(); }

protected:
    enum State
    {
        STATE_STRAIGHT = 0,
        STATE_CIRCLE_LEFT,
        STATE_CIRCLE_RIGHT,
        STATE_SPIRAL_OUT_LEFT,
        STATE_SPIRAL_OUT_RIGHT,
        STATE_SPIRAL_IN_LEFT,
        STATE_SPIRAL_IN_RIGHT,
    };

    void createObject(glm::vec3 currentLocation, float currentAngleRads);
    State calculateState(TurnDirection turning, Accelerating accelerating) const;
    void LightTrail::turn(float currentAngleRads, bool justStarted);
    void LightTrail::stopTurning();
    void LightTrail::updateLastVertices(glm::vec3 currentLocation);

    std::shared_ptr<World> world;
    std::shared_ptr<const Shader> shader;
    glm::vec3 colour;

    // meshes for drawing to the screen
    MeshData<glm::vec3> lightTrailMeshData;
    std::shared_ptr<ObjData3D> lightTrailObjData;
    std::unique_ptr<Object> lightTrailObj;

    State state;
    bool stopping;
    bool isStopped;
};

#endif
