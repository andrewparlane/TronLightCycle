#include <bike.hpp>

Bike::Bike(std::shared_ptr<const ObjLoader> _objLoader, 
           std::shared_ptr<World> _world, 
           std::shared_ptr<const Shader> _shader,
           const glm::mat4 &modelMat) 
    : Object(_objLoader, _world, _shader, modelMat),
      wheelAngle(0.0f), engineAngle(0.0f)
{
    const std::vector<Mesh> &meshes = objLoader->getMeshes();
    unsigned int i = 0;
    for (auto &it : meshes)
    {
        remainderIndexes.push_back(i++);
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
