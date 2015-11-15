#ifndef __OBJECT_DATA_HPP
#define __OBJECT_DATA_HPP

#include <string>
#include <vector>

#include <glm/glm.hpp>

#include <GL/glew.h>

struct MeshData
{
    std::vector<unsigned short> indices;
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> normals;

    std::string name;
    std::string texturePath;
    bool hasTexture;
};

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

class ObjData
{
public:
    ObjData();
    ~ObjData();

    void addMesh(const MeshData &data);

    bool createBuffers();

    const std::vector<Mesh> &getMeshes() const { return meshes; }
    const std::vector<MeshAxis> &getAxis() const { return axis; }
    const std::vector<MeshSeperator> &getSeperators() const { return seperators; }

protected:
    std::vector<MeshData> meshData;
    std::vector<Mesh> meshes;
    std::vector<MeshAxis> axis;
    std::vector<MeshSeperator> seperators;
};

#endif
