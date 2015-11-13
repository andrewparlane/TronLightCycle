#ifndef OBJLOADER_H
#define OBJLOADER_H

#include <vector>
#include <cstring>

#include <glm/glm.hpp>

#include <GL/glew.h>

struct Mesh
{
    std::string name;
    bool hasTexture;

    glm::vec3 firstVertex; // needed for use with seperators

    GLuint vertexBuffer;
    GLuint uvBuffer;
    GLuint normalBuffer;
    GLuint indiceBuffer;
    GLuint texture;
    unsigned int numIndices;
};

struct MeshAxis
{
    std::string name;
    glm::vec3 point;
    glm::vec3 axis;
};

struct MeshSeperator
{
    std::string name;
    glm::vec3 point;
    glm::vec3 normal;
};

class ObjLoader
{
public:
    ObjLoader(const std::string &objPath, const std::string &nameToTexturePath);
    ~ObjLoader();

    bool loadObj();
    bool loadTextures();

    const std::vector<Mesh> &getMeshes() const { return meshes; }
    const std::vector<MeshAxis> &getAxis() const { return axis; }
    const std::vector<MeshSeperator> &getSeperators() const { return seperators; }

protected:
    std::string objPath;
    std::string nameToTexturePath;

    std::vector<Mesh> meshes;
    std::vector<MeshAxis> axis;
    std::vector<MeshSeperator> seperators;
};


#endif
