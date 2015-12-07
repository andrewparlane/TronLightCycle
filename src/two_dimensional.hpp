#ifndef __TWO_DIMENSIONAL_HPP
#define __TWO_DIMENSIONAL_HPP

#include <object.hpp>
#include <shader.hpp>

class Object2D
{
public:
    Object2D(std::shared_ptr<const Shader> _shader, glm::vec3 _defaultColour = glm::vec3(1.0f,1.0f,1.0f));
    ~Object2D();

    void deleteObjData(const std::string &name) { objData.deleteMesh(name); }
    void deleteAllObjData() { objData.deleteAll(); }

    void drawAll() const;

protected:
    void drawMesh(const std::shared_ptr<Mesh<glm::vec2>> &mesh) const;
    ObjData2D objData;

    std::shared_ptr<const Shader> shader;
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

    void addLine(glm::vec2 start, glm::vec2 end, glm::vec3 startColour, glm::vec3 endColour, float thickness);
    void addRect(glm::vec2 tl, glm::vec2 tr, glm::vec2 br, glm::vec2 bl, glm::vec3 tlCol, glm::vec3 trCol, glm::vec3 brCol, glm::vec3 blCol, const std::string &name = "");

protected:
    unsigned int numRects;
};

#endif
