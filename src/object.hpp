#ifndef __OBJECT_HPP
#define __OBJECT_HPP

#include <memory>

#include <object_data.hpp>
#include <shader.hpp>
#include <world.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

class Object
{
public:
    Object(std::shared_ptr<const ObjData> _objData, 
           std::shared_ptr<World> _world, 
           std::shared_ptr<const Shader> _shader,
           const glm::mat4 &modelMat);
    ~Object();

    virtual void translate(const glm::vec3 &vec) { modelMatrix *= glm::translate(vec); }
    void rotate(float radians, const glm::vec3 &axis) { modelMatrix *= glm::rotate(radians, axis); }

    glm::vec3 applyModelMatrx(const glm::vec3 &input) const { return glm::vec3(modelMatrix * glm::vec4(input,1.0f)); }

    void drawAll() const;

    void setDefaultColour(const glm::vec3 &col) { defaultColour = col; }
    const glm::vec3 &getDefaultColour() { return defaultColour; }

protected:

    virtual void internalDrawAll(const std::vector<Mesh> &meshes) const;
    void drawMesh(const Mesh &mesh) const;

    std::shared_ptr<const ObjData> objData;

    std::shared_ptr<World> world;
    std::shared_ptr<const Shader> shader;

    glm::mat4 modelMatrix;

    glm::vec3 defaultColour;
};
    
#endif
