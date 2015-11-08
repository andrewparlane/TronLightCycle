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

bool loadAssImp(const char * path, std::vector<Mesh> &meshes)
{
    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(path, 0/*aiProcess_JoinIdenticalVertices | aiProcess_SortByPType*/);
    if (!scene) {
        fprintf(stderr, importer.GetErrorString());
        return false;
    }

    meshes.reserve(scene->mNumMeshes);

    for (unsigned int j = 0; j < scene->mNumMeshes; j++)
    {
        const aiMesh* mesh = scene->mMeshes[j];
        printf("mesh %u) %s\n", j, mesh->mName.C_Str());

        Mesh newMesh;

        // Fill vertices positions
        newMesh.vertices.reserve(mesh->mNumVertices);
        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
            aiVector3D pos = mesh->mVertices[i];
            newMesh.vertices.push_back(glm::vec3(pos.x, pos.y, pos.z));
        }

        // Fill vertices texture coordinates
        if (mesh->HasTextureCoords(0))
        {
            newMesh.uvs.reserve(mesh->mNumVertices);
            for (unsigned int i = 0; i < mesh->mNumVertices; i++)
            {
                aiVector3D UVW = mesh->mTextureCoords[0][i]; // Assume only 1 set of UV coords; AssImp supports 8 UV sets.
                newMesh.uvs.push_back(glm::vec2(UVW.x, -UVW.y));
            }
            newMesh.hasTexture = true;
        }
        else
        {
            printf("\tno texture\n");
            newMesh.hasTexture = false;
        }
        // Fill vertices normals
        newMesh.normals.reserve(mesh->mNumVertices);
        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
            aiVector3D n = mesh->mNormals[i];
            newMesh.normals.push_back(glm::vec3(n.x, n.y, n.z));
        }


        // Fill face indices
        newMesh.indices.reserve(3 * mesh->mNumFaces);
        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            // Assume the model has only triangles.
            newMesh.indices.push_back(mesh->mFaces[i].mIndices[0]);
            newMesh.indices.push_back(mesh->mFaces[i].mIndices[1]);
            newMesh.indices.push_back(mesh->mFaces[i].mIndices[2]);
        }

        meshes.push_back(newMesh);
    }

	// The "scene" pointer will be deleted automatically by "importer"
    return true;
}
