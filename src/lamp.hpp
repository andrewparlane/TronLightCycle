#ifndef __LAMP_HPP
#define __LAMP_HPP

#include <memory>
#include <vector>

#include <glm/glm.hpp>

#define MAX_NUM_LAMPS 20    // Note: must synce with shaders/main.fs

class Shader;
class ObjData3D;
template<typename T> struct Mesh;

class Lamp
{
public:
    Lamp(std::shared_ptr<const ObjData3D> _objData, std::shared_ptr<const Shader> _shader,
         const glm::mat4 &modelMatWithoutTransform, const glm::vec3 &_position, float _radius,
         const glm::vec3 &_colour, float _ambient, float _diffuse, float _specular);
    ~Lamp();

    void draw(const glm::mat4 &projectionMatrix, const glm::mat4 &viewMatrix) const;

    void getLampData(glm::vec3 &position, float &radius, glm::vec3 &colour, float &ambient, float &diffuse, float &specular) const;

protected:
    std::shared_ptr<const ObjData3D> objData;
    std::shared_ptr<const Shader> shader;
    glm::mat4 modelMatrix;

    glm::vec3 position;
    float radius;
    glm::vec3 colour;
    float ambient;
    float diffuse;
    float specular;
};

#endif
