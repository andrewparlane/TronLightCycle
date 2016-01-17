#ifndef __LIGHT_TRAIL_HPP
#define __LIGHT_TRAIL_HPP

#include "bike_movements.hpp"
#include "object_data.hpp"

#include <memory>
#include <vector>

#include <glm/glm.hpp>

class World;
class Shader;
class Object;
class LightTrailSegment;

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

    bool collides(const glm::vec2 &location) const;
    bool checkSelfCollision() const;

    void draw() const;

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
    void createNewPathSegment(float speed, glm::vec3 currentLocation, float currentAngleRads);

    std::shared_ptr<World> world;
    std::shared_ptr<const Shader> shader;
    glm::vec3 colour;

    // meshes for drawing to the screen
    MeshData<glm::vec3> lightTrailMeshData;
    std::shared_ptr<ObjData3D> lightTrailObjData;
    std::unique_ptr<Object> lightTrailObj;

    // abstract path info for collision detection
    std::vector<std::unique_ptr<LightTrailSegment>> pathSegments;

    State state;
    bool stopping;
    bool isStopped;
};

#endif
