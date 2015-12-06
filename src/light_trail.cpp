#include "light_trail.hpp"

static const float lightTrailHeight = 3.8f;

LightTrail::LightTrail(std::shared_ptr<World> _world,
                       std::shared_ptr<const Shader> _shader,
                       glm::vec3 _colour)
    : world(_world), shader(_shader), colour(_colour),
      trailNumber(0), state(STATE_STOPPED), turning(false)
{
    lightTrailObjData.reset(new ObjData3D());
}

LightTrail::~LightTrail()
{
}

void LightTrail::toggle()
{
    if (state == STATE_ON)
    {
        // we are on, so start stopping now
        state = STATE_STOPPING;
        turning = false;
    }
    else
    {
        // we are either stopped or stopping.
        // deosn't matter create new light trail
        state = STATE_ON;

        lightTrailMeshData.push_back(MeshData<glm::vec3>());
        MeshData<glm::vec3> &md = lightTrailMeshData.back();

        md.name = "LT" + std::to_string(trailNumber++);
        md.hasTexture = false;
        //md.texturePath = "light_trail.DDS";
        md.vertices.push_back(glm::vec3(bikeLocation.x, 0.0f,             bikeLocation.z));    // bottom nearest
        md.vertices.push_back(glm::vec3(bikeLocation.x, lightTrailHeight, bikeLocation.z));    // top nearest
        md.vertices.push_back(glm::vec3(bikeLocation.x, 0.0f,             bikeLocation.z));    // bottom furthest
        md.vertices.push_back(glm::vec3(bikeLocation.x, lightTrailHeight, bikeLocation.z));    // top furthest
        md.normals.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
        md.normals.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
        md.normals.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
        md.normals.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
        /*md.uvs.push_back(glm::vec2(0.0f, 0.0f));
        md.uvs.push_back(glm::vec2(0.0f, 1.0f));
        md.uvs.push_back(glm::vec2(0.1f, 0.0f));
        md.uvs.push_back(glm::vec2(0.1f, 1.0f));*/
        md.indices.push_back(0); md.indices.push_back(1); md.indices.push_back(3);
        md.indices.push_back(0); md.indices.push_back(3); md.indices.push_back(2);

        if (!lightTrailObjData->addMesh(md))
        {
            // fali
            printf("Failed to create light trail obj data\n");
            return;
        }
        lightTrail = std::make_unique<Object>(lightTrailObjData, world, shader, glm::mat4(1.0f));
        lightTrail->setDefaultColour(colour);
    }
}

void LightTrail::turn(float bikeAngleRadians)
{
    if (state == STATE_ON)
    {
        // because the bike has turned, we need to add a new face
        // to our light trail data.

        MeshData<glm::vec3> &md = lightTrailMeshData.back();

        unsigned int numVertices = md.vertices.size();
        glm::vec3 lastVertexPosBottom = md.vertices[numVertices - 2];
        glm::vec3 lastVertexPosTop = md.vertices[numVertices - 1];
        // wall is alwasy verticle, we know which way the bike is facing
        // so normal is 90 degrees (rotated around y) from bike direction
        // and since the bike is orientated along Z, we can just rotate a unit vector along x by bikeAngle
        glm::vec3 newNormal = glm::normalize(glm::vec3(glm::vec4(1,0,0,1) * glm::rotate(-bikeAngleRadians, glm::vec3(0,1,0))));

        // if we just started turning we don't want the past long wall
        // to look curved, ie. don't average normals for the corner vertex
        // so we need to duplicate the last two vertices (corner vertices),
        // and amend the normals

        if (!turning)
        {
            // just started turning
            turning = true;

            // duplicate positions and uvs
            md.vertices.push_back(lastVertexPosBottom);
            md.vertices.push_back(lastVertexPosTop);

            /*md.uvs.push_back(md.uvs[numVertices - 2]);
            md.uvs.push_back(md.uvs[numVertices - 1]);*/

            // create new normals
            md.normals.push_back(newNormal);
            md.normals.push_back(newNormal);

            numVertices += 2;
        }
        else
        {
            // haven't just started turning, so we share our normals
            // to make a smooth curve
            glm::vec3 averagedNormal = glm::normalize(newNormal + md.normals[numVertices - 2]);
            md.normals[numVertices - 2] = averagedNormal;
            md.normals[numVertices - 1] = averagedNormal;
        }

        // now we need to add the two new vertices for this face
        // which means two more vertices, two more normals and 6 more indices

        // currently in same location as previous vertices
        md.vertices.push_back(md.vertices[numVertices - 2]);
        md.vertices.push_back(md.vertices[numVertices - 1]);

        // TODO: do UVs properly
        /*md.uvs.push_back(glm::vec2(0.0f, 0.0f));
        md.uvs.push_back(glm::vec2(0.0f, 0.0f));*/

        md.normals.push_back(newNormal);
        md.normals.push_back(newNormal);

        numVertices += 2;
        md.indices.push_back(numVertices - 4); md.indices.push_back(numVertices - 3); md.indices.push_back(numVertices - 1);
        md.indices.push_back(numVertices - 4); md.indices.push_back(numVertices - 1); md.indices.push_back(numVertices - 2);
    }
}

void LightTrail::stopTurning()
{
    if (state == STATE_ON)
    {
        MeshData<glm::vec3> &md = lightTrailMeshData.back();
        if (turning)
        {
            turning = false;
            // just stopped turning, so to render this as flat
            // we need to create two extra vertices for the corner
            // with unique normals

            unsigned int numVertices = md.vertices.size();

            // duplicate positions
            glm::vec3 lastTopPos = md.vertices.back();
            md.vertices.pop_back();
            glm::vec3 lastBottomPos = md.vertices.back();
            md.vertices.pop_back();

            md.vertices.push_back(md.vertices[numVertices - 4]);
            md.vertices.push_back(md.vertices[numVertices - 3]);
            md.vertices.push_back(lastBottomPos);
            md.vertices.push_back(lastTopPos);

            // duplicate UVs
            /*glm::vec2 lastTopUV = md.uvs.back();
            md.uvs.pop_back();
            glm::vec2 lastBottomUV = md.uvs.back();
            md.uvs.pop_back();

            md.uvs.push_back(md.uvs[numVertices - 4]);
            md.uvs.push_back(md.uvs[numVertices - 3]);
            md.uvs.push_back(lastBottomUV);
            md.uvs.push_back(lastTopUV);*/

            // update normals
            // the last set of normals are normal to the plane
            // we just need to create those extra two vertices
            md.normals.push_back(md.normals[numVertices - 2]);
            md.normals.push_back(md.normals[numVertices - 1]);
            // the two normals before that were averaged with this last face
            // which we don't want anymore, calculate the new normals
            glm::vec3 newNormal = glm::normalize(glm::cross(md.normals[numVertices - 6] - md.normals[numVertices - 4],
                                                 glm::vec3(0,1,0)));
            md.normals[numVertices - 4] = newNormal;
            md.normals[numVertices - 3] = newNormal;

            // update indices
            numVertices += 2;
            for (unsigned int i = 0; i < 6; i++)
            {
                md.indices.pop_back();
            }
            md.indices.push_back(numVertices - 4); md.indices.push_back(numVertices - 3); md.indices.push_back(numVertices - 1);
            md.indices.push_back(numVertices - 4); md.indices.push_back(numVertices - 1); md.indices.push_back(numVertices - 2);
        }
    }
}

void LightTrail::updateLastVertices(glm::vec3 bikeLocationWorld)
{
    bikeLocation = bikeLocationWorld;
    if (state == STATE_ON)
    {
        MeshData<glm::vec3> &md = lightTrailMeshData.back();
        // update the positions of the last two vertices
        md.vertices.pop_back();
        md.vertices.pop_back();
        md.vertices.push_back(glm::vec3(bikeLocation.x, 0.0f, bikeLocation.z));
        md.vertices.push_back(glm::vec3(bikeLocation.x, lightTrailHeight, bikeLocation.z));
    }
}

void LightTrail::update()
{
    // all light trails other than the current running one should fade down to nothing
    // the current one should do that too, if state is stopping

    if (lightTrailMeshData.size() > 1)
    {
        for (unsigned int i = 0; i < lightTrailMeshData.size() - 1; i++)
        {
            bool changedSomething = false;
            for (auto &v : lightTrailMeshData[i].vertices)
            {
                if (v.y > 0.1f)
                {
                    v.y -= 0.05f;
                    changedSomething = true;
                }
            }
            if (!changedSomething)
            {
                // we can delete this one
                lightTrailObjData->deleteMesh(lightTrailMeshData[i].name);
                lightTrailMeshData.erase(lightTrailMeshData.begin()+i);
                i--; // so when we loop, we stay at the same increment
            }
        }
    }

    if (state == STATE_STOPPING)
    {
        // also start fading down the current back()
        MeshData<glm::vec3> &md = lightTrailMeshData.back();
        bool changedSomething = false;
        for (auto &v : md.vertices)
        {
            if (v.y > 0.1f)
            {
                v.y -= 0.05f;
                changedSomething = true;
            }
        }
        if (!changedSomething)
        {
            // delete this last element
            lightTrailObjData->deleteMesh(md.name);
            lightTrailMeshData.erase(lightTrailMeshData.end()-1);
            state = STATE_STOPPED;
        }
    }

    for (auto &md : lightTrailMeshData)
    {
        lightTrailObjData->updateMesh(md);
    }
    lightTrailObjData->updateBuffers();
}