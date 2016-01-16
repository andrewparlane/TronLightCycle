#ifndef __LAMP_HPP
#define __LAMP_HPP

#include "shader.hpp"
#include "object_data.hpp"

#include <memory>

class Lamp
{
public:
    Lamp(std::shared_ptr<const ObjData3D> _objData,
         std::shared_ptr<const Shader> _shader,
         const glm::mat4 &modelMatWithoutTransform,
         const glm::vec3 &_position,
         const glm::vec3 &_colour,
         float _power,
         const glm::vec3 &_ambient);
    ~Lamp();

    void draw(const glm::mat4 &projectionMatrix, const glm::mat4 &viewMatrix) const;

    void sendLamp(std::shared_ptr<const Shader> shader) const;

protected:
    std::shared_ptr<const ObjData3D> objData;
    std::shared_ptr<const Shader> shader;
    glm::mat4 modelMatrix;

    glm::vec3 position;
    glm::vec3 colour;
    float power;
    glm::vec3 ambient;
};

#endif
