#ifndef __WORLD_HPP
#define __WORLD_HPP

#include <memory>
#include <vector>
#include <algorithm>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

class Shader;
class Lamp;
class ObjData3D;
class ObjData2D;

class World
{
public:
    World();
    ~World();

    void setCamera(const glm::mat4 &view) { viewMatrix = view; }
    void translateCamera(const glm::vec3 &vec) { viewMatrix *= glm::translate(vec); }
    void rotateCamera(float radians, const glm::vec3 &axis) { viewMatrix *= glm::rotate(radians, axis); }

    void setProjection(const glm::mat4 &projection) { projectionMatrix = projection; inverseProjectionMatrix = glm::inverse(projection); }

    bool addLamp(std::shared_ptr<const ObjData3D> objData, std::shared_ptr<const ObjData2D> deferredShadingObj, std::shared_ptr<const Shader> shader,
                 const glm::mat4 &modelMatWithoutTransform, const glm::vec3 &position,
                 float radius, const glm::vec3 &colour, float ambient, float diffuse, float specular);

    void sendMVP(std::shared_ptr<const Shader> shader, const glm::mat4 &model) const;
    void sendLightingInfoToShader(std::shared_ptr<const Shader> shader) const;
    void drawLamps() const;

protected:
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;
    glm::mat4 inverseProjectionMatrix;

    std::vector<std::unique_ptr<Lamp>> lamps;
};

#endif
