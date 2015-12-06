#ifndef __TWO_DIMENSIONAL_HPP
#define __TWO_DIMENSIONAL_HPP

#include <object.hpp>
#include <shader.hpp>

class Object2D
{
public:
    Object2D(std::shared_ptr<const Shader> _shader);
    ~Object2D();

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

#endif
