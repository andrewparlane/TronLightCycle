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

bool ObjData::addMesh(const MeshData &md)
{
    meshData.push_back(md);
    return createBuffers(meshData.back());
}

bool ObjData::updateMesh(const MeshData &data)
{
    for (auto &md : meshData)
    {
        if (md.name.compare(data.name) == 0)
        {
            md = data;
            md.needsUpdate = true;
            return true;
        }
    }
    return false;
}

bool ObjData::createBuffers(MeshData &md)
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
    md.needsUpdate = false;

    meshes.push_back(newMesh);

    return true;
}

void ObjData::updateBuffers()
{
    for (auto &md : meshData)
    {
        if (md.needsUpdate)
        {
            for (auto &m : meshes)
            {
                if (m.name.compare(md.name) == 0)
                {
                    m.numIndices = md.indices.size();
                    m.firstVertex = md.vertices[0];

                    // buffer orphaning
                    // see: http://www.opengl-tutorial.org/intermediate-tutorials/billboards-particles/particles-instancing/
                    // and: https://www.opengl.org/wiki/Buffer_Object_Streaming
                    // TODO: does this work when md.vertices.size() changes?

                    glBindBuffer(GL_ARRAY_BUFFER, m.vertexBuffer);
                    glBufferData(GL_ARRAY_BUFFER, md.vertices.size() * sizeof(glm::vec3), NULL, GL_STREAM_DRAW);
                    glBufferSubData(GL_ARRAY_BUFFER, 0, md.vertices.size() * sizeof(glm::vec3), &md.vertices[0]);

                    if (md.hasTexture)
                    {
                        glBindBuffer(GL_ARRAY_BUFFER, m.uvBuffer);
                        glBufferData(GL_ARRAY_BUFFER, md.uvs.size() * sizeof(glm::vec2), NULL, GL_STREAM_DRAW);
                        glBufferSubData(GL_ARRAY_BUFFER, 0, md.uvs.size() * sizeof(glm::vec2), &md.uvs[0]);
                    }

                    glBindBuffer(GL_ARRAY_BUFFER, m.normalBuffer);
                    glBufferData(GL_ARRAY_BUFFER, md.normals.size() * sizeof(glm::vec3), NULL, GL_STREAM_DRAW);
                    glBufferSubData(GL_ARRAY_BUFFER, 0, md.normals.size() * sizeof(glm::vec3), &md.normals[0]);

                    glBindBuffer(GL_ARRAY_BUFFER, m.indiceBuffer);
                    glBufferData(GL_ARRAY_BUFFER, md.indices.size() * sizeof(unsigned short), NULL, GL_STREAM_DRAW);
                    glBufferSubData(GL_ARRAY_BUFFER, 0, md.indices.size() * sizeof(unsigned short), &md.indices[0]);
                    break;
                }
            }
        }
    }
}

BoundingBox ObjData::getBoundingBox() const
{
    // calculate boinding box
    BoundingBox bb;

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
    bb.vertices[0] = glm::vec3(leftMost,    lowest,     nearest);
    bb.vertices[1] = glm::vec3(leftMost,    highest,    nearest);
    bb.vertices[2] = glm::vec3(rightMost,   highest,    nearest);
    bb.vertices[3] = glm::vec3(rightMost,   lowest,     nearest);

    // furthest face
    bb.vertices[4] = glm::vec3(leftMost,    lowest,     furthest);
    bb.vertices[5] = glm::vec3(leftMost,    highest,    furthest);
    bb.vertices[6] = glm::vec3(rightMost,   highest,    furthest);
    bb.vertices[7] = glm::vec3(rightMost,   lowest,     furthest);

    return bb;
}
