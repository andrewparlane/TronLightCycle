#include <object_data.hpp>

#include <opengl_tutorials_org/texture.hpp>

ObjData::ObjData()
{
}

ObjData::~ObjData()
{
    for (auto &i : meshes)
    {
        glDeleteBuffers(1, &i.indiceBuffer);
        glDeleteBuffers(1, &i.normalBuffer);
        glDeleteBuffers(1, &i.vertexBuffer);
        if (i.hasTexture)
        {
            glDeleteTextures(1, &i.texture);
            glDeleteBuffers(1, &i.uvBuffer);
        }
    }
}

void ObjData::addMesh(const MeshData &md)
{
    meshData.push_back(md);
}

bool ObjData::createBuffers()
{
    meshes.empty();

    for (auto &md : meshData)
    {
        Mesh newMesh;

        newMesh.name = md.name;
        newMesh.hasTexture = md.hasTexture;

        newMesh.numIndices = md.indices.size();

        newMesh.firstVertex = md.vertices[0];

        // generate buffers
        glGenBuffers(1, &newMesh.vertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, newMesh.vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, md.vertices.size() * sizeof(glm::vec3), &md.vertices[0], GL_STATIC_DRAW);

        if (newMesh.hasTexture)
        {
            glGenBuffers(1, &newMesh.uvBuffer);
            glBindBuffer(GL_ARRAY_BUFFER, newMesh.uvBuffer);
            glBufferData(GL_ARRAY_BUFFER, md.uvs.size() * sizeof(glm::vec2), &md.uvs[0], GL_STATIC_DRAW);;
        }

        glGenBuffers(1, &newMesh.normalBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, newMesh.normalBuffer);
        glBufferData(GL_ARRAY_BUFFER, md.normals.size() * sizeof(glm::vec3), &md.normals[0], GL_STATIC_DRAW);

        glGenBuffers(1, &newMesh.indiceBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, newMesh.indiceBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, md.indices.size() * sizeof(unsigned short), &md.indices[0], GL_STATIC_DRAW);

        if (newMesh.hasTexture)
        {
            newMesh.texture = loadDDS(std::string("textures/compressed/") + md.texturePath);
            if (newMesh.texture == 0)
            {
                printf("Couldn't loadDDS texture: %s\n", md.texturePath.c_str());
                return false;
            }
        }
        else
        {
            newMesh.texture = 0;
        }

        meshes.push_back(newMesh);
    }

    return true;
}
