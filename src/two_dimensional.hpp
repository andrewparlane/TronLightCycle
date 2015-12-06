#ifndef __TWO_DIMENSIONAL_HPP
#define __TWO_DIMENSIONAL_HPP

#include <object.hpp>
#include <shader.hpp>

class Object2D
{
public:
    Object2D(std::shared_ptr<const Shader> _shader, glm::vec3 _defaultColour = glm::vec3(1.0f,1.0f,1.0f));
    ~Object2D();

    void deleteAllObjData() { objData.deleteAll(); }

    void drawAll() const;

protected:
    void drawMesh(const std::shared_ptr<Mesh<glm::vec2>> &mesh) const;
    ObjData2D objData;

    std::shared_ptr<const Shader> shader;

    glm::vec3 defaultColour;
};

class Text : public Object2D
{
public:
    Text(std::shared_ptr<const Shader> _shader);
    ~Text();

    void addText2D(const char *text, int x, int y, int size, unsigned int textureID);

protected:
    unsigned int numTextStrings;
};

class Shape2D : public Object2D
{
public:
    Shape2D(std::shared_ptr<const Shader> _shader, glm::vec3 _defaultColour);
    ~Shape2D();

    void addLine(glm::vec2 start, glm::vec2 end, float thickness);
    void addRect(glm::vec2 tl, glm::vec2 tr, glm::vec2 br, glm::vec2 bl);

protected:
    unsigned int numRects;
};

#endif
