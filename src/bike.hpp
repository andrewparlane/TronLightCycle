#ifndef __BIKE_HPP
#define __BIKE_HPP

#include <object.hpp>

class Bike : public Object
{
public:
    Bike(std::shared_ptr<const ObjLoader> _objLoader, 
        std::shared_ptr<World> _world, 
        std::shared_ptr<const Shader> _shader,
        const glm::mat4 &modelMat);
    ~Bike();

    // overrides
    void translate(const glm::vec3 &vec);

protected:
    void internalDrawAll(const std::vector<Mesh> &meshes) const;

    float wheelAngle;
    float engineAngle;

    std::vector<unsigned int> frontTyreMeshIndexes;
    std::vector<unsigned int> backTyreMeshIndexes;
    std::vector<unsigned int> leftEngineIndexes;
    std::vector<unsigned int> rightengineIndexes;
    std::vector<unsigned int> remainderIndexes;
};

#endif
