#ifndef __OBJECT_DATA_HPP
#define __OBJECT_DATA_HPP

#include <string>
#include <vector>
#include <memory>

#include <texture.hpp>

#include <glm/glm.hpp>

#include <GL/glew.h>

template<typename T> struct MeshData
{
    std::vector<unsigned short> indices;
    std::vector<T> vertices;
    std::vector<glm::vec2> uvs;
    std::vector<T> normals;
    std::vector<glm::vec3> colours;

    std::string name;
    std::string texturePath;            // only one of texturePath
    std::shared_ptr<Texture> texture;   // and texture are used
    bool hasTexture;

    bool needsUpdate;
};

template<typename T> struct Mesh
{
    ~Mesh()
    {
        glDeleteBuffers(1, &indiceBuffer);
        glDeleteBuffers(1, &normalBuffer);
        glDeleteBuffers(1, &vertexBuffer);
        glDeleteBuffers(1, &colourBuffer);
        if (hasTexture)
        {
            glDeleteBuffers(1, &uvBuffer);
        }
    }

    std::string name;
    bool hasTexture;

    T firstVertex; // needed for use with seperators

    GLuint vertexBuffer;
    GLuint uvBuffer;
    GLuint normalBuffer;
    GLuint indiceBuffer;
    GLuint colourBuffer;
    std::shared_ptr<Texture> texture;
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

template<typename T> struct BoundingBox
{
    T vertices[8];
};

template<typename T> class ObjData
{
public:
    ObjData();
    virtual ~ObjData();

    bool addMesh(const MeshData<T> &data);

    bool updateMesh(const MeshData<T> &data);

    void deleteMesh(const std::string &name);
    void deleteAll();

    void updateBuffers();

    const std::vector<std::shared_ptr<Mesh<T>>> &getMeshes() const { return meshes; }

    BoundingBox<T> getBoundingBox();

protected:
    bool createBuffers(MeshData<T> &data);

    virtual void calculateBoundingBox() = 0;

    std::vector<MeshData<T>> meshData;
    std::vector<std::shared_ptr<Mesh<T>>> meshes;

    BoundingBox<T> cachedBoundingBox;
    bool boundingBoxIsCached;
};

class ObjData2D : public ObjData<glm::vec2>
{
public:
    ObjData2D();
    ~ObjData2D();

protected:
    void calculateBoundingBox() override;
};

class ObjData3D : public ObjData<glm::vec3>
{
public:
    ObjData3D();
    ~ObjData3D();

    const std::vector<MeshAxis> &getAxis() const { return axis; }
    const std::vector<MeshSeperator> &getSeperators() const { return seperators; }

protected:
    void calculateBoundingBox() override;

    std::vector<MeshAxis> axis;
    std::vector<MeshSeperator> seperators;
};

#endif
