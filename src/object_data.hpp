#ifndef __OBJECT_DATA_HPP
#define __OBJECT_DATA_HPP

#include <string>
#include <vector>
#include <memory>

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

    bool needsUpdate;
};

struct Mesh
{
    ~Mesh()
    {
        glDeleteBuffers(1, &indiceBuffer);
        glDeleteBuffers(1, &normalBuffer);
        glDeleteBuffers(1, &vertexBuffer);
        if (hasTexture)
        {
            glDeleteTextures(1, &texture);
            glDeleteBuffers(1, &uvBuffer);
        }
    }

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

struct BoundingBox
{
    glm::vec3 vertices[8];
};

class ObjData
{
public:
    ObjData();
    virtual ~ObjData();

    bool addMesh(const MeshData &data);

    bool updateMesh(const MeshData &data);

    void deleteMesh(const std::string &name);

    void updateBuffers();

    const std::vector<std::shared_ptr<Mesh>> &getMeshes() const { return meshes; }
    const std::vector<MeshAxis> &getAxis() const { return axis; }
    const std::vector<MeshSeperator> &getSeperators() const { return seperators; }

    BoundingBox getBoundingBox();

protected:
    bool createBuffers(MeshData &data);

    std::vector<MeshData> meshData;
    std::vector<std::shared_ptr<Mesh>> meshes;
    std::vector<MeshAxis> axis;
    std::vector<MeshSeperator> seperators;

    BoundingBox cachedBoundingBox;
    bool boundingBoxIsCached;
};

#endif
