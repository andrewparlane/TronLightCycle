#ifndef OBJLOADER_H
#define OBJLOADER_H

#include <vector>
#include <cstring>

#include <glm/glm.hpp>

struct Mesh
{
    std::string name;
    bool hasTexture;

    GLuint vertexBuffer;
    GLuint uvBuffer;
    GLuint normalBuffer;
    GLuint indiceBuffer;
    unsigned int numIndices;
};

struct MeshAxis
{
    std::string name;
    glm::vec3 point;
    glm::vec3 axis;
};

class ObjLoader
{
public:
    ObjLoader(const std::string &path);
    ~ObjLoader();

    bool loadObj();

    const std::vector<Mesh> &getMeshes() { return meshes; }
    const std::vector<MeshAxis> &getAxis() { return axis; }

protected:
    std::string path;

    std::vector<Mesh> meshes;
    std::vector<MeshAxis> axis;
};


#endif
