#ifndef __TWO_DIMENSIONAL_HPP
#define __TWO_DIMENSIONAL_HPP

#include <object.hpp>
#include <shader.hpp>

class TwoDimensional
{
public:
    TwoDimensional(std::shared_ptr<const Shader> _shader);
    ~TwoDimensional();

    void addText2D(const char *text, int x, int y, int size, unsigned int textureID);
    void deleteAllObjData() { objData.deleteAll(); }

    void drawAll() const;

protected:
    void drawMesh(const std::shared_ptr<Mesh<glm::vec2>> &mesh) const;
    ObjData2D objData;

    std::shared_ptr<const Shader> shader;

    unsigned int numTextStrings;
};

#endif
