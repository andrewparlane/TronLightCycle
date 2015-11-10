#ifndef __OBJECT_HPP
#define __OBJECT_HPP

#include <memory>

#include <objloader.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

class Object
{
public:
    Object(std::shared_ptr<const ObjLoader> _objLoader, const glm::mat4 &mat);
    ~Object();

    void translate(const glm::vec3 &vec) { modelMatrix *= glm::translate(vec); }
    void rotate(float radians, const glm::vec3 &axis) { modelMatrix *= glm::rotate(radians, axis); }

    void drawAll() const;

    void setDefaultColour(const glm::vec3 &col) { defaultColour = col; }
    const glm::vec3 &getDefaultColour() { return defaultColour; }

protected:

    void drawMesh(const Mesh &mesh) const;

    std::shared_ptr<const ObjLoader> objLoader;

    glm::mat4 modelMatrix;

    glm::vec3 defaultColour;
};
    
#endif
