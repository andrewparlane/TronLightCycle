#include <vector>
#include <stdio.h>
#include <string>
#include <cstring>

#include <glm/glm.hpp>

#include "objloader.hpp"

// Include AssImp
#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

// for temporary use as we don't need to store the data
// once it's been loaded into the bufers
struct MeshData
{
    std::vector<unsigned short> indices;
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> normals;
};

ObjLoader::ObjLoader(const std::string &filePath) : path(filePath)
{
}

ObjLoader::~ObjLoader()
{
    for (auto &i : meshes)
    {
        glDeleteBuffers(1, &i.indiceBuffer);
        glDeleteBuffers(1, &i.normalBuffer);
        glDeleteBuffers(1, &i.vertexBuffer);
        if (i.hasTexture)
        {
            glDeleteBuffers(1, &i.uvBuffer);
        }
    }
}

bool ObjLoader::loadObj()
{
    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(path, 0/*aiProcess_JoinIdenticalVertices | aiProcess_SortByPType*/);
    if (!scene) {
        fprintf(stderr, importer.GetErrorString());
        return false;
    }

    // reserve space in meshes, might not need that many due to axis
    // and seperators, but most should be actually there to render
    meshes.empty();
    meshes.reserve(scene->mNumMeshes);
    
    // not sure how many axis to reserve, so don't reserve any
    axis.empty();

    for (unsigned int j = 0; j < scene->mNumMeshes; j++)
    {
        const aiMesh* mesh = scene->mMeshes[j];
        printf("mesh %u) %s\n", j, mesh->mName.C_Str());

        // check if it's an axis, if so add to axis, if not add to meshes
        if (std::string(mesh->mName.C_Str()).find("axis") != std::string::npos)
        {
            // this is an axis, so don't render it
            MeshAxis ma;
            ma.name = mesh->mName.C_Str();

            aiVector3D n = mesh->mNormals[0];
            ma.axis = glm::vec3(n.x, n.y, n.z);

            ma.point = glm::vec3(0, 0, 0);
            for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
                aiVector3D pos = mesh->mVertices[i];
                ma.point += glm::vec3(pos.x, pos.y, pos.z);
            }
            ma.point /= mesh->mNumVertices;

            axis.push_back(ma);
        }
        else if (std::string(mesh->mName.C_Str()).find("seperator") != std::string::npos)
        {
            // this is a seperator, so don't render it
            MeshSeperator ms;
            ms.name = mesh->mName.C_Str();

            aiVector3D n = mesh->mNormals[0];
            ms.normal = glm::vec3(n.x, n.y, n.z);

            ms.point = glm::vec3(0, 0, 0);
            for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
                aiVector3D pos = mesh->mVertices[i];
                ms.point += glm::vec3(pos.x, pos.y, pos.z);
            }
            ms.point /= mesh->mNumVertices;

            seperators.push_back(ms);
        }
        else
        {
            Mesh newMesh;
            MeshData md;

            newMesh.name = mesh->mName.C_Str();

            // Fill vertices positions
            md.vertices.reserve(mesh->mNumVertices);
            for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
                aiVector3D pos = mesh->mVertices[i];
                glm::vec3 v(pos.x, pos.y, pos.z);
                md.vertices.push_back(v);
                if (i == 0)
                {
                    newMesh.firstVertex = v;
                }
            }

            // Fill vertices texture coordinates
            if (mesh->HasTextureCoords(0))
            {
                md.uvs.reserve(mesh->mNumVertices);
                for (unsigned int i = 0; i < mesh->mNumVertices; i++)
                {
                    aiVector3D UVW = mesh->mTextureCoords[0][i]; // Assume only 1 set of UV coords; AssImp supports 8 UV sets.
                    md.uvs.push_back(glm::vec2(UVW.x, -UVW.y));
                }
                newMesh.hasTexture = true;
            }
            else
            {
                printf("\tno texture\n");
                newMesh.hasTexture = false;
            }
            // Fill vertices normals
            md.normals.reserve(mesh->mNumVertices);
            for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
                aiVector3D n = mesh->mNormals[i];
                md.normals.push_back(glm::vec3(n.x, n.y, n.z));
            }


            // Fill face indices
            md.indices.reserve(3 * mesh->mNumFaces);
            for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
                // Assume the model has only triangles.
                md.indices.push_back(mesh->mFaces[i].mIndices[0]);
                md.indices.push_back(mesh->mFaces[i].mIndices[1]);
                md.indices.push_back(mesh->mFaces[i].mIndices[2]);
            }

            newMesh.numIndices = md.indices.size();

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
            /*texture = loadDDS(texturePath);
            if (texture == 0)
            {
            return false;
            }*/

            meshes.push_back(newMesh);
        }
    }

    // clean up the ASSIMP importer
    importer.FreeScene();

	// The "scene" pointer will be deleted automatically by "importer"
    return true;
}
