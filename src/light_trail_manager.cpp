#include "light_trail_manager.hpp"
#include "light_trail.hpp"

LightTrailManager::LightTrailManager(std::shared_ptr<World> _world,
                                     std::shared_ptr<const Shader> _shader,
                                     glm::vec3 _colour)
    : world(_world), shader(_shader), colour(_colour),
      state(STATE_STOPPED), lastTurning(NO_TURN), lastAccelerating(SPEED_NORMAL)
{
}

LightTrailManager::~LightTrailManager()
{
}

void LightTrailManager::toggle()
{
    if (state == STATE_ON)
    {
        // we are on, so start stopping now
        state = STATE_STOPPING;
        trails.back()->stop();
    }
    else
    {
        // we are either stopped or stopping.
        // deosn't matter create new light trail
        state = STATE_ON;
        trails.push_back(std::make_unique<LightTrail>(world, shader, colour, lastTurning, lastAccelerating));
    }
}

void LightTrailManager::turnOff()
{
    if (state == STATE_ON)
    {
        // we are on, so start stopping now
        state = STATE_STOPPING;
        trails.back()->stop();
    }
    // else we are already stopping or stopped, nothing to do.
}

void LightTrailManager::update(TurnDirection turning, Accelerating accelerating, float speed, glm::vec3 currentLocation, float currentAngleRads)
{
    lastTurning = turning;
    lastAccelerating = accelerating;

    std::for_each(trails.begin(), trails.end(),
        [&](std::unique_ptr<LightTrail> &trail)
        {
            trail->update(turning, accelerating, speed, currentLocation, currentAngleRads);
        });

    // lets see if we can delete any
    // we can delete if they are stopped,
    //  and have finished fading down
    // we can stop looking when the front item isn't dead.
    //  As any past that can't be dead yet either
    while (trails.size() && trails.front()->isDead())
    {
        trails.erase(trails.begin());
    }
}

bool LightTrailManager::collides(const glm::vec2 &location) const
{
    for (auto &t : trails)
    {
        if (t->collides(location))
        {
            return true;
        }
    }
    return false;
}

bool LightTrailManager::checkSelfCollision() const
{
    if (trails.size())
    {
        return trails.back()->checkSelfCollision();
    }
    return false;
}

void LightTrailManager::drawAll() const
{
    std::for_each(trails.begin(), trails.end(),
                  [](const std::unique_ptr<LightTrail> &trail)
    {
        trail->draw();
    });
}
