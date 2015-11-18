#ifndef __WORLD_HPP
#define __WORLD_HPP

#include <shader.hpp>

#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

class World
{
public:
    World();
    ~World();

    void setCamera(const glm::mat4 &view) { viewMatrix = view; }
    void translateCamera(const glm::vec3 &vec) { viewMatrix *= glm::translate(vec); }
    void rotateCamera(float radians, const glm::vec3 &axis) { viewMatrix *= glm::rotate(radians, axis); }

    void setProjection(const glm::mat4 &projection) { projectionMatrix = projection; }

    void setLight(const glm::vec3 &position, const glm::vec3 &colour, float power, const glm::vec3 &ambient);

    void sendMVP(std::shared_ptr<const Shader> shader, const glm::mat4 &model) const;
    void sendLight(std::shared_ptr<const Shader> shader) const;

protected:
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;

    glm::vec3 lightPos;
    glm::vec3 lightColour;
    float lightPower;
    glm::vec3 lightAmbient;
};

#endif
