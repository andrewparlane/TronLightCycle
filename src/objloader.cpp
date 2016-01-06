#include <vector>
#include <stdio.h>
#include <string>
#include <cstring>
#include <utility>

#include <glm/glm.hpp>

#include "objloader.hpp"

#include <opengl_tutorials_org/texture.hpp>

// Include AssImp
#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags
#include <assimp/ProgressHandler.hpp>

ObjLoader::ObjLoader(const std::string &objFilePath, const std::string &_textureMapPath, ProgressBar *_progressBar, ProgressBar::ProgressType _progressType)
    : ObjData3D(), objPath(objFilePath), textureMapPath(_textureMapPath),
      progressBar(_progressBar), progressType(_progressType)
{
}

ObjLoader::~ObjLoader()
{
}

bool ObjLoader::loadTextureMap()
{
    const unsigned int SIZEOF_BUFFERS = 512;
    char inBuffer[SIZEOF_BUFFERS];

    textureMap.empty();

    FILE *fd;
    fopen_s(&fd, textureMapPath.c_str(), "r");
    if (fd == NULL)
    {
        printf("unable to open %s\n", textureMapPath.c_str());
        return false;
    }
    while (1)
    {
        if (fgets(inBuffer, SIZEOF_BUFFERS, fd) == NULL)
        {
            // EOF
            fclose(fd);
            return true;
        }

        std::string inStr(inBuffer);

        size_t commaOffset = inStr.find(',');
        if (commaOffset == std::string::npos)
        {
            printf("Error, no ',' found\n");
            fclose(fd);
            return false;
        }

        // remove \r
        inStr.erase(std::remove(inStr.begin(), inStr.end(), '\r'), inStr.end());

        // check for \n
        size_t newLineOffset = inStr.find('\n');
        if (newLineOffset == std::string::npos)
        {
            printf("Error, no '\\n' found, buffer size too small\n");
            fclose(fd);
            return false;
        }

        std::string meshName = inStr.substr(0, commaOffset);
        std::string textureName = inStr.substr(commaOffset + 1, newLineOffset - (commaOffset + 1));

        textureMap.push_back(std::make_pair(meshName, textureName));
    }

    // shouldn't get here
    return false;
}

class MyProgressHandler : public Assimp::ProgressHandler
{
public:
    MyProgressHandler(ProgressBar *_progressBar, ProgressBar::ProgressType _progressType)
        : Assimp::ProgressHandler(),
          progressBar(_progressBar), progressType(_progressType)
    {
    }

    ~MyProgressHandler() {}

    bool Update(float percentage=-1.0f) override
    {
        if (progressBar)
        {
            progressBar->update(progressType, percentage);
        }
        return true;
    }

protected:
    ProgressBar *progressBar;
    ProgressBar::ProgressType progressType;
};

bool ObjLoader::loadObj()
{
    Assimp::Importer importer;

    // note don't delete this, ownership is taken by ASSIMP
    MyProgressHandler *progressHandler = new MyProgressHandler(progressBar, progressType);
    importer.SetProgressHandler(progressHandler);
    const aiScene* scene = importer.ReadFile(objPath, aiProcess_JoinIdenticalVertices /*| aiProcess_SortByPType*/);
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
            MeshData<glm::vec3> md;

            md.name = mesh->mName.C_Str();

            // Fill vertices positions
            md.vertices.reserve(mesh->mNumVertices);
            for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
                aiVector3D pos = mesh->mVertices[i];
                glm::vec3 v(pos.x, pos.y, pos.z);
                md.vertices.push_back(v);
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
                md.hasTexture = true;

                // look up the name in the texture map
                bool found = false;
                for (auto &tm : textureMap)
                {
                    if (tm.first.compare(md.name) == 0)
                    {
                        md.texturePath = tm.second;
                        found = true;
                        break;
                    }
                }
                if (!found)
                {
                    printf("No entry in texture map for %s\n", md.name.c_str());
                }
            }
            else
            {
                printf("\tno texture\n");
                md.hasTexture = false;
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

            addMesh(md);
        }
    }

    // clean up the ASSIMP importer
    importer.FreeScene();

	// The "scene" pointer will be deleted automatically by "importer"
    return true;
}
