#include <bike.hpp>

#include <set>

Bike::Bike(std::shared_ptr<const ObjData> _objData, 
           std::shared_ptr<World> _world, 
           std::shared_ptr<const Shader> _shader,
           const glm::mat4 &modelMat,
           const glm::vec3 &_defaultColour)
    : Object(_objData, _world, _shader, modelMat, _defaultColour),
      bikeAngleAroundYRads(0.0f), wheelAngle(0.0f), engineAngle(0.0f),
      trail(_world, _shader, _defaultColour)
{
    initialiseBikeParts();
}

Bike::~Bike()
{
}

void Bike::updateLocation()
{
    const glm::vec3 vec(0,0,-0.4f);
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

    trail.updateLastVertices(applyModelMatrx(glm::vec3(0.0f)));
}

void Bike::turn(TurnDirection dir)
{
    if (dir == NO_TURN)
    {
        trail.stopTurning();
        return;
    }

    float angleRads = glm::radians((dir == TURN_RIGHT) ? -2.0f : 2.0f);
    bikeAngleAroundYRads += angleRads;
    rotate(angleRads, glm::vec3(0,1,0));

    trail.turn(bikeAngleAroundYRads);
}

void Bike::internalDrawAll(const std::vector<Mesh> &meshes) const
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

    // light trail
    trail.drawAll();
}

void Bike::updateLightTrail()
{
    // light trail
    trail.update();
}

void Bike::initialiseBikeParts()
{
    const std::vector<Mesh> &meshes = objData->getMeshes();
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
            if (mesh.name.find(sepType) == 0)
            {
                // ok are we in front or behind
                // note don't need to normalise them, as we only care about the sign
                float cosTheta = glm::dot(sep.normal, mesh.firstVertex - sep.point);

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
