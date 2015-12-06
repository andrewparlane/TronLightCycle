#include <object_data.hpp>

#include <opengl_tutorials_org/texture.hpp>

template class ObjData<glm::vec2>;
template class ObjData<glm::vec3>;

template<typename T> ObjData<T>::ObjData()
{
}

template<typename T> ObjData<T>::~ObjData()
{
}

template<typename T> bool ObjData<T>::addMesh(const MeshData<T> &md)
{
    meshData.push_back(md);
    boundingBoxIsCached = false;
    return createBuffers(meshData.back());
}

template<typename T> bool ObjData<T>::updateMesh(const MeshData<T> &data)
{
    for (auto &md : meshData)
    {
        if (md.name.compare(data.name) == 0)
        {
            md = data;
            md.needsUpdate = true;
            boundingBoxIsCached = false;
            return true;
        }
    }
    return false;
}

template<typename T> void ObjData<T>::deleteMesh(const std::string &name)
{
    for (auto it = meshData.begin(); it != meshData.end(); it++)
    {
        if (it->name.compare(name) == 0)
        {
            meshData.erase(it);
            break;
        }
    }

    for (auto it = meshes.begin(); it != meshes.end(); it++)
    {
        if ((*it)->name.compare(name) == 0)
        {
            meshes.erase(it);
            break;
        }
    }
    boundingBoxIsCached = false;
}

template<typename T> bool ObjData<T>::createBuffers(MeshData<T> &md)
{
    std::shared_ptr<Mesh<T>> newMesh(new Mesh<T>());

    newMesh->name = md.name;
    newMesh->hasTexture = md.hasTexture;

    newMesh->numIndices = md.indices.size();

    newMesh->firstVertex = md.vertices[0];

    // generate buffers
    glGenBuffers(1, &newMesh->vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, newMesh->vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, md.vertices.size() * sizeof(md.vertices[0]), &md.vertices[0], GL_STATIC_DRAW);

    if (newMesh->hasTexture)
    {
        glGenBuffers(1, &newMesh->uvBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, newMesh->uvBuffer);
        glBufferData(GL_ARRAY_BUFFER, md.uvs.size() * sizeof(md.uvs[0]), &md.uvs[0], GL_STATIC_DRAW);;
    }

    glGenBuffers(1, &newMesh->normalBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, newMesh->normalBuffer);
    glBufferData(GL_ARRAY_BUFFER, md.normals.size() * sizeof(md.normals[0]), &md.normals[0], GL_STATIC_DRAW);

    glGenBuffers(1, &newMesh->indiceBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, newMesh->indiceBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, md.indices.size() * sizeof(md.indices[0]), &md.indices[0], GL_STATIC_DRAW);

    if (newMesh->hasTexture)
    {
        newMesh->texture = loadDDS(std::string("textures/compressed/") + md.texturePath);
        if (newMesh->texture == 0)
        {
            printf("Couldn't loadDDS texture: %s\n", md.texturePath.c_str());
            return false;
        }
    }
    else
    {
        newMesh->texture = 0;
    }
    md.needsUpdate = false;

    meshes.push_back(newMesh);

    return true;
}

template<typename T> void ObjData<T>::updateBuffers()
{
    for (auto &md : meshData)
    {
        if (md.needsUpdate)
        {
            for (auto &m : meshes)
            {
                if (m->name.compare(md.name) == 0)
                {
                    m->numIndices = md.indices.size();
                    m->firstVertex = md.vertices[0];

                    // buffer orphaning
                    // see: http://www.opengl-tutorial.org/intermediate-tutorials/billboards-particles/particles-instancing/
                    // and: https://www.opengl.org/wiki/Buffer_Object_Streaming
                    // TODO: does this work when md.vertices.size() changes?

                    glBindBuffer(GL_ARRAY_BUFFER, m->vertexBuffer);
                    glBufferData(GL_ARRAY_BUFFER, md.vertices.size() * sizeof(md.vertices[0]), NULL, GL_STREAM_DRAW);
                    glBufferSubData(GL_ARRAY_BUFFER, 0, md.vertices.size() * sizeof(md.vertices[0]), &md.vertices[0]);

                    if (md.hasTexture)
                    {
                        glBindBuffer(GL_ARRAY_BUFFER, m->uvBuffer);
                        glBufferData(GL_ARRAY_BUFFER, md.uvs.size() * sizeof(md.uvs[0]), NULL, GL_STREAM_DRAW);
                        glBufferSubData(GL_ARRAY_BUFFER, 0, md.uvs.size() * sizeof(md.uvs[0]), &md.uvs[0]);
                    }

                    glBindBuffer(GL_ARRAY_BUFFER, m->normalBuffer);
                    glBufferData(GL_ARRAY_BUFFER, md.normals.size() * sizeof(md.normals[0]), NULL, GL_STREAM_DRAW);
                    glBufferSubData(GL_ARRAY_BUFFER, 0, md.normals.size() * sizeof(md.normals[0]), &md.normals[0]);

                    glBindBuffer(GL_ARRAY_BUFFER, m->indiceBuffer);
                    glBufferData(GL_ARRAY_BUFFER, md.indices.size() * sizeof(md.indices[0]), NULL, GL_STREAM_DRAW);
                    glBufferSubData(GL_ARRAY_BUFFER, 0, md.indices.size() * sizeof(md.indices[0]), &md.indices[0]);
                    break;
                }
            }
        }
    }
}

template<typename T> BoundingBox<T> ObjData<T>::getBoundingBox()
{
    if (!boundingBoxIsCached)
    {
        calculateBoundingBox();
    }

    return cachedBoundingBox;
}

ObjData2D::ObjData2D() : ObjData()
{
}

ObjData2D::~ObjData2D()
{
}

void ObjData2D::calculateBoundingBox()
{
    // calculate boinding box
    boundingBoxIsCached = true;

    // axis increas to the right, up, towards us
    float rightMost = -FLT_MAX;
    float highest = -FLT_MAX;
    
    float lowest = FLT_MAX;
    float leftMost = FLT_MAX;

    for (auto &md : meshData)
    {
        for (auto &v : md.vertices)
        {
            if (v.x < leftMost)     leftMost = v.x;
            if (v.x > rightMost)    rightMost = v.x;
            if (v.y < lowest)       lowest = v.y;
            if (v.y > highest)      highest = v.y;
        }
    }

    // face
    cachedBoundingBox.vertices[0] = glm::vec2(leftMost,    lowest);
    cachedBoundingBox.vertices[1] = glm::vec2(leftMost,    highest);
    cachedBoundingBox.vertices[2] = glm::vec2(rightMost,   highest);
    cachedBoundingBox.vertices[3] = glm::vec2(rightMost,   lowest);
}

ObjData3D::ObjData3D() : ObjData()
{
}

ObjData3D::~ObjData3D()
{
}

void ObjData3D::calculateBoundingBox()
{
    // calculate boinding box
    boundingBoxIsCached = true;

    // axis increas to the right, up, towards us
    float rightMost = -FLT_MAX;
    float highest = -FLT_MAX;
    float nearest = -FLT_MAX;

    float lowest = FLT_MAX;
    float furthest = FLT_MAX;
    float leftMost = FLT_MAX;

    for (auto &md : meshData)
    {
        for (auto &v : md.vertices)
        {
            if (v.x < leftMost)     leftMost = v.x;
            if (v.x > rightMost)    rightMost = v.x;
            if (v.y < lowest)       lowest = v.y;
            if (v.y > highest)      highest = v.y;
            if (v.z < furthest)     furthest = v.z;
            if (v.z > nearest)      nearest = v.z;
        }
    }

    // nearest face
    cachedBoundingBox.vertices[0] = glm::vec3(leftMost,    lowest,     nearest);
    cachedBoundingBox.vertices[1] = glm::vec3(leftMost,    highest,    nearest);
    cachedBoundingBox.vertices[2] = glm::vec3(rightMost,   highest,    nearest);
    cachedBoundingBox.vertices[3] = glm::vec3(rightMost,   lowest,     nearest);

    // furthest face
    cachedBoundingBox.vertices[4] = glm::vec3(leftMost,    lowest,     furthest);
    cachedBoundingBox.vertices[5] = glm::vec3(leftMost,    highest,    furthest);
    cachedBoundingBox.vertices[6] = glm::vec3(rightMost,   highest,    furthest);
    cachedBoundingBox.vertices[7] = glm::vec3(rightMost,   lowest,     furthest);
}
