#include "bike.hpp"
#include "shader.hpp"
#include "world.hpp"
#include "light_trail_manager.hpp"

#include <set>

#define RATE_OF_ACCELERATE      0.005f

Bike::Bike(std::shared_ptr<const ObjData3D> _objData,
           std::shared_ptr<World> _world,
           std::shared_ptr<const Shader> _shader,
           std::shared_ptr<const Shader> _explodeShader,
           const glm::mat4 &modelMat,
           const glm::vec3 &_defaultColour)
    : Object(_objData, _world, _shader, modelMat, _defaultColour),
      bikeAngleAroundYRads(0.0f), wheelAngle(0.0f), engineAngle(0.0f),
      trailManager(std::make_shared<LightTrailManager>(_world, _shader, _defaultColour)),
      speed(BIKE_SPEED_DEFAULT), explodeShader(_explodeShader),
      explodeLevel(0.0f), exploding(false)
{
#ifdef DEBUG
    bikeStateSaved = false;
#endif

    initialiseBikeParts();
}

Bike::~Bike()
{
}

void Bike::updateLocation()
{
    const glm::vec3 vec(0,0,-speed);
    translate(vec);

    // calculate wheel spin based on distance travelled
    // TODO calculate?
    // current value is based off importing tyre vertices into libra office
    // getting min and max for all axis, taking the difference and /2
    const float RADIUS_OF_WHEEL = 1.87f;

    // length of arc of segment = angle * radius
    // length of arc of segment = length of distance moved over the ground
    // angle = distance over the ground / radius
    wheelAngle -= glm::length(vec) / RADIUS_OF_WHEEL;

    // update engineAngle, if we move it gets updated regardless of speed
    // TODO tweak to what looks good
    // TODO base on time since last frame rather than here, it slows and speeds up as frame rate changes
    engineAngle += glm::radians(8.0f);
}

void Bike::turn(TurnDirection dir)
{
    if (dir == NO_TURN)
    {
        return;
    }

    float angleRads = glm::radians((dir == TURN_RIGHT) ? ANGLE_OF_TURNS : -ANGLE_OF_TURNS);
    bikeAngleAroundYRads += angleRads;
    rotate(angleRads, glm::vec3(0,-1,0));
}

Accelerating Bike::updateSpeed(Accelerating a)
{
    Accelerating actualAction = SPEED_NORMAL;
    switch (a)
    {
        case SPEED_NORMAL:
        {
            // not accelerating or braking, move back towards default
            if (speed > BIKE_SPEED_DEFAULT + 0.0049f)
            {
                speed -= RATE_OF_ACCELERATE;
                actualAction = SPEED_BRAKE;
            }
            else if (speed < BIKE_SPEED_DEFAULT - 0.0049f)
            {
                speed += RATE_OF_ACCELERATE;
                actualAction = SPEED_ACCELERATE;
            }
            else
            {
                speed = BIKE_SPEED_DEFAULT;
            }
            break;
        }
        case SPEED_ACCELERATE:
        {
            if (speed < BIKE_SPEED_FASTEST)
            {
                actualAction = SPEED_ACCELERATE;
                speed += RATE_OF_ACCELERATE;
                if (speed > BIKE_SPEED_FASTEST)
                {
                    speed = BIKE_SPEED_FASTEST;
                }
            }
            break;
        }
        case SPEED_BRAKE:
        {
            if (speed > BIKE_SPEED_SLOWEST)
            {
                actualAction = SPEED_BRAKE;
                speed -= RATE_OF_ACCELERATE;
                if (speed < BIKE_SPEED_SLOWEST)
                {
                    speed = BIKE_SPEED_SLOWEST;
                }
            }
            break;
        }
    }
    return actualAction;
}

float Bike::getSpeedPercent() const
{
    float percent = (speed - BIKE_SPEED_SLOWEST) / (BIKE_SPEED_FASTEST - BIKE_SPEED_SLOWEST);

    if (percent > 1.0f)
    {
        percent = 1.0f;
    }
    else if (percent < 0.0f)
    {
        percent = 0.0f;
    }

    return percent;
}

void Bike::toggleLightTrail()
{
    trailManager->toggle();
}

bool Bike::checkSelfCollision() const
{
    return trailManager->checkSelfCollision();
}

void Bike::setExploding()
{
    exploding = true;
    trailManager->turnOff();
    switchShader(explodeShader);
}

void Bike::internalDrawAll(const std::vector<std::shared_ptr<Mesh<glm::vec3>>> &meshes) const
{
    GLuint explodeID = shader->getUniformID(SHADER_UNIFORM_EXPLODE);

    // only draw the bike if not fully exploded
    if (explodeLevel < 1.0f)
    {
        glm::mat4 ftmm = modelMatrix *                                      // finally apply overal model transformation
                         glm::translate(frontTyreAxis.point) *              // 3rd translate back to initial point
                         glm::rotate(wheelAngle, frontTyreAxis.axis) *      // 2nd rotate around origin
                         glm::translate(-frontTyreAxis.point);              // 1st translate axis to origin

        glm::mat4 btmm = modelMatrix *                                      // finally apply overal model transformation
                         glm::translate(backTyreAxis.point) *               // 3rd translate back to initial point
                         glm::rotate(wheelAngle, backTyreAxis.axis) *       // 2nd rotate around origin
                         glm::translate(-backTyreAxis.point);               // 1st translate axis to origin

        glm::mat4 remm = modelMatrix *                                      // finally apply overal model transformation
                         glm::translate(rightEngineAxis.point) *            // 3rd translate back to initial point
                         glm::rotate(-engineAngle, rightEngineAxis.axis) *   // 2nd rotate around origin
                         glm::translate(-rightEngineAxis.point);            // 1st translate axis to origin

        glm::mat4 lemm = modelMatrix *                                      // finally apply overal model transformation
                         glm::translate(leftEngineAxis.point) *             // 3rd translate back to initial point
                         glm::rotate(engineAngle, leftEngineAxis.axis) *   // 2nd rotate around origin
                         glm::translate(-leftEngineAxis.point);             // 1st translate axis to origin

        // set explode
        if (explodeID >= 0)
        {
            glUniform1f(explodeID, explodeLevel);
        }

        // front tyre
        world->sendMVP(shader, ftmm);
        for (auto it : frontTyreMeshIndexes)
        {
            drawMesh(meshes[it]);
        }

        // back tyre
        world->sendMVP(shader, btmm);
        for (auto it : backTyreMeshIndexes)
        {
            drawMesh(meshes[it]);
        }

        // left engine
        world->sendMVP(shader, lemm);
        for (auto it : leftEngineIndexes)
        {
            drawMesh(meshes[it]);
        }

        // right engine
        world->sendMVP(shader, remm);
        for (auto it : rightengineIndexes)
        {
            drawMesh(meshes[it]);
        }

        // everything else
        world->sendMVP(shader, modelMatrix);
        for (auto it : remainderIndexes)
        {
            drawMesh(meshes[it]);
        }
    }

    // light trail
    if (explodeID >= 0)
    {
        glUniform1f(explodeID, 0.0f);
    }
    trailManager->drawAll();
}

void Bike::update(TurnDirection turning, Accelerating accelerating, bool stop)
{
    if (exploding)
    {
        if (explodeLevel < 1.0f)
        {
            explodeLevel += 1.0f/30.0f;
        }
        // fade light trail
        trailManager->update(NO_TURN, SPEED_NORMAL, 0.0f, applyModelMatrx(glm::vec3(0.0f)), bikeAngleAroundYRads);
        return;
    }

    if (stop)
    {
        return;
    }

    // update speed
    // returns what actually happened.
    // ie. if you were at max speed and no longer held the accelerate button
    // you'd be deaccelerating (braking)
    float oldSpeed = speed;
    Accelerating actualAccelerating = updateSpeed(accelerating);

    // update the light trail before we change angle or location
    trailManager->update(turning, actualAccelerating, oldSpeed, applyModelMatrx(glm::vec3(0.0f)), bikeAngleAroundYRads);

    // update angle
    turn(turning);

    // update location
    updateLocation();
}

void Bike::initialiseBikeParts()
{
    const std::vector<std::shared_ptr<Mesh<glm::vec3>>> &meshes = objData->getMeshes();
    const std::vector<MeshAxis> &axis = objData->getAxis();
    const std::vector<MeshSeperator> &seperators = objData->getSeperators();

    // need to split bike into front tyre, back tyre, left engine, right engine, remainder
    // so we can look for "tyre" and "engine" in their names.
    // I added seperator planes to the model, so we can look at any vertex on the mesh
    // and see if it's in front or behind the seperator planes.
    // I decided the normal of the plane should point towards the front or the right, depending on orientation
    // we can decide this using the dot product of the normal and the (vertex - centre point of the plane)

    std::set<unsigned int> used;

    for (auto &sep : seperators)
    {
        // get the name of the seperator
        // XXX_seperator, we want XXX
        size_t f = sep.name.find('_');
        if (f == std::string::npos)
        {
            // ???
            continue;
        }
        std::string sepType = sep.name.substr(0,f);

        // OK, now match this with the names of meshes
        unsigned int i = 0;
        for (auto &mesh : meshes)
        {
            // does it start with the same thing the seperator started with?
            if (mesh->name.find(sepType) == 0)
            {
                // ok are we in front or behind
                // note don't need to normalise them, as we only care about the sign
                float cosTheta = glm::dot(sep.normal, mesh->firstVertex - sep.point);

                if (sepType.compare("tyre") == 0)
                {
                    used.insert(i);
                    if (cosTheta > 0)
                    {
                        frontTyreMeshIndexes.push_back(i);
                    }
                    else
                    {
                        backTyreMeshIndexes.push_back(i);
                    }
                }
                else if (sepType.compare("engine") == 0)
                {
                    used.insert(i);
                    if (cosTheta > 0)
                    {
                        rightengineIndexes.push_back(i);
                    }
                    else
                    {
                        leftEngineIndexes.push_back(i);
                    }
                }
                else
                {
                    // ???
                    break;  // this seperator is no good, break out of mesh loop
                }
            }
            i++;
        }

        // now match this with the name of the axis
        for (auto &ax : axis)
        {
            // does it start with the same thing the seperator started with?
            if (ax.name.find(sepType) == 0)
            {
                // ok are we in front or behind
                // note don't need to normalise them, as we only care about the sign
                float cosTheta = glm::dot(sep.normal, ax.point - sep.point);

                if (sepType.compare("tyre") == 0)
                {
                    if (cosTheta > 0)
                    {
                        frontTyreAxis = ax;
                    }
                    else
                    {
                        backTyreAxis = ax;
                    }
                }
                else if (sepType.compare("engine") == 0)
                {
                    if (cosTheta > 0)
                    {
                        rightEngineAxis = ax;
                    }
                    else
                    {
                        leftEngineAxis = ax;
                    }
                }
                else
                {
                    // ???
                    break; // this seperator is no good, break out of axis loop
                }
            }
        }
    }

    // finally populate the remainder
    unsigned int i = 0;
    for (auto u : used)
    {
        if (u != i)
        {
            while (i < u)
            {
                remainderIndexes.push_back(i++);
            }
        }
        i++;
    }
}

#ifdef DEBUG
bool bikeStateSaved;

void Bike::saveBikeState()
{
    bikeStateSaved = true;
    savedBikeAngleAroundYRads = bikeAngleAroundYRads;
    savedSpeed = speed;
    savedModelMatrix = modelMatrix;
}

void Bike::restoreBikeState()
{
    if (bikeStateSaved)
    {
        bikeAngleAroundYRads = savedBikeAngleAroundYRads;
        speed = savedSpeed;
        modelMatrix = savedModelMatrix;
    }
}
#endif
