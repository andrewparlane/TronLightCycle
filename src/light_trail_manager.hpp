#ifndef __LIGHT_TRAIL_MANAGER_HPP
#define __LIGHT_TRAIL_MANAGER_HPP

#include "world.hpp"
#include "shader.hpp"
#include "light_trail.hpp"
#include "bike_movements.hpp"

#include <vector>
#include <memory>
#include <algorithm>

class LightTrailManager
{
public:
    LightTrailManager(std::shared_ptr<World> _world,
                      std::shared_ptr<const Shader> _shader,
                      glm::vec3 _colour);
    ~LightTrailManager();

    // turn on or off the light trail
    void toggle();
    void turnOff();

    void update(TurnDirection turning, Accelerating accelerating, float speed, glm::vec3 currentLocation, float currentAngleRads);

    bool collides(const glm::vec2 &location) const;
    bool checkSelfCollision() const;

    // draw all the light trails
    void drawAll() const { std::for_each( trails.begin(), trails.end(), [](const std::unique_ptr<LightTrail> &trail){ trail->draw(); }); };

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

    State state;

    std::vector<std::unique_ptr<LightTrail>> trails;

    TurnDirection lastTurning;
    Accelerating lastAccelerating;
};

#endif
