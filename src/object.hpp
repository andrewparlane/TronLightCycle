#ifndef __OBJECT_HPP
#define __OBJECT_HPP

#include <memory>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

class ObjData3D;
class World;
class Shader;
template<typename T> struct Mesh;

class Object
{
public:
    Object(std::shared_ptr<const ObjData3D> _objData,
           std::shared_ptr<World> _world,
           std::shared_ptr<const Shader> _shader,
           const glm::mat4 &modelMat,
           const glm::vec3 &_defaultColour = glm::vec3(0,0,0));
    virtual ~Object();

    virtual void translate(const glm::vec3 &vec) { modelMatrix *= glm::translate(vec); }
    virtual void rotate(float radians, const glm::vec3 &axis) { modelMatrix *= glm::rotate(radians, axis); }

    glm::vec3 applyModelMatrx(const glm::vec3 &input) const { return glm::vec3(modelMatrix * glm::vec4(input,1.0f)); }

    void drawAll() const;

    void setDefaultColour(const glm::vec3 &col) { defaultColour = col; }
    const glm::vec3 &getDefaultColour() { return defaultColour; }

    void switchShader(std::shared_ptr<const Shader> newShader) { shader = newShader; }

protected:

    virtual void internalDrawAll(const std::vector<std::shared_ptr<Mesh<glm::vec3>>> &meshes) const;
    void drawMesh(const std::shared_ptr<Mesh<glm::vec3>> &mesh) const;

    std::shared_ptr<const ObjData3D> objData;

    std::shared_ptr<World> world;
    std::shared_ptr<const Shader> shader;

    glm::mat4 modelMatrix;

    glm::vec3 defaultColour;
};

#endif
