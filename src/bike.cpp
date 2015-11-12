#include <bike.hpp>

#include <set>

Bike::Bike(std::shared_ptr<const ObjLoader> _objLoader, 
           std::shared_ptr<World> _world, 
           std::shared_ptr<const Shader> _shader,
           const glm::mat4 &modelMat) 
    : Object(_objLoader, _world, _shader, modelMat),
      wheelAngle(0.0f), engineAngle(0.0f)
{
    const std::vector<Mesh> &meshes = objLoader->getMeshes();
    const std::vector<MeshAxis> &axis = objLoader->getAxis();
    const std::vector<MeshSeperator> &seperators = objLoader->getSeperators();

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
                    i++;
                    continue;
                }
            }
            i++;
        }

        // now match this with the name of the axis
        i = 0;
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
                        frontTyreAxis = i;
                    }
                    else
                    {
                        backTyreAxis = i;
                    }
                    // done, only one axis
                    break;
                }
                else if (sepType.compare("engine") == 0)
                {
                    if (cosTheta > 0)
                    {
                        rightEngineAxis = i;
                    }
                    else
                    {
                        leftEngineAxis = i;
                    }
                    // done only one axis
                    break;
                }
                else
                {
                    // ???
                    i++;
                    continue;
                }
            }
            i++;
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

Bike::~Bike()
{
}

void Bike::translate(const glm::vec3 &vec)
{
    Object::translate(vec);

    // calculate wheel spin based on distance travelled
    // TODO calculate?
    // current value is based off importing tyre vertices into libra office
    // getting min and max for all axis, taking the difference and /2
    const float RADIUS_OF_WHEEL = 1.87f;

    // length of arc of segment = angle * radius
    // length of arc of segment = length of distance moved over the ground
    // angle = distance over the ground / radius
    wheelAngle += vec.length() / RADIUS_OF_WHEEL;

    // update engineAngle, if we move it gets updated regardless of speed
    // TODO tweak to what looks good
    engineAngle += glm::radians(1.0f);
}

void Bike::internalDrawAll(const std::vector<Mesh> &meshes) const
{
    // front tyre
    for (auto it : frontTyreMeshIndexes)
    {
        drawMesh(meshes[it]);
    }
    // back tyre
    for (auto it : backTyreMeshIndexes)
    {
        drawMesh(meshes[it]);
    }
    // left engine
    for (auto it : leftEngineIndexes)
    {
        drawMesh(meshes[it]);
    }
    // right engine
    for (auto it : rightengineIndexes)
    {
        drawMesh(meshes[it]);
    }
    // everything else
    for (auto it : remainderIndexes)
    {
        drawMesh(meshes[it]);
    }
}
