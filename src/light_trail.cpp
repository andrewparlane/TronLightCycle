#include "light_trail.hpp"

static const float lightTrailHeight = 3.8f;

LightTrail::LightTrail() : turning(false)
{
}

LightTrail::~LightTrail()
{
}

bool LightTrail::initialise(std::shared_ptr<World> world, 
                            std::shared_ptr<const Shader> shader,
                            glm::vec3 colour)
{
    // create light trail
    lightTrailObjData.reset(new ObjData());

    lightTrailMeshData. name = "LT";
    lightTrailMeshData.hasTexture = false;
    //lightTrailMeshData.texturePath = "light_trail.DDS";
    lightTrailMeshData.vertices.push_back(glm::vec3(0.0f, 0.0f,             0.0f));        // bottom nearest
    lightTrailMeshData.vertices.push_back(glm::vec3(0.0f, lightTrailHeight, 0.0f));        // top nearest
    lightTrailMeshData.vertices.push_back(glm::vec3(0.0f, 0.0f,             -0.1f));    // bottom furthest
    lightTrailMeshData.vertices.push_back(glm::vec3(0.0f, lightTrailHeight, -0.1f));    // top furthest
    lightTrailMeshData.normals.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
    lightTrailMeshData.normals.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
    lightTrailMeshData.normals.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
    lightTrailMeshData.normals.push_back(glm::vec3(1.0f, 0.0f, 0.0f));
    /*lightTrailMeshData.uvs.push_back(glm::vec2(0.0f, 0.0f));
    lightTrailMeshData.uvs.push_back(glm::vec2(0.0f, 1.0f));
    lightTrailMeshData.uvs.push_back(glm::vec2(0.1f, 0.0f));
    lightTrailMeshData.uvs.push_back(glm::vec2(0.1f, 1.0f));*/
    lightTrailMeshData.indices.push_back(0); lightTrailMeshData.indices.push_back(1); lightTrailMeshData.indices.push_back(3);
    lightTrailMeshData.indices.push_back(0); lightTrailMeshData.indices.push_back(3); lightTrailMeshData.indices.push_back(2);
    lightTrailObjData->addMesh(lightTrailMeshData);
    if (!lightTrailObjData->createBuffers())
    {
        // fali
        printf("Failed to create light trail obj data\n");
        return false;
    }
    lightTrail = std::make_unique<Object>(lightTrailObjData, world, shader, glm::mat4(1.0f));
    lightTrail->setDefaultColour(colour);
    return true;
}

void LightTrail::turn(float bikeAngleRadians)
{
    // because the bike has turned, we need to add a new face
    // to our light trail data.

    unsigned int numVertices = lightTrailMeshData.vertices.size();
    glm::vec3 lastVertexPosBottom = lightTrailMeshData.vertices[numVertices - 2];
    glm::vec3 lastVertexPosTop = lightTrailMeshData.vertices[numVertices - 1];
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
        lightTrailMeshData.vertices.push_back(lastVertexPosBottom);
        lightTrailMeshData.vertices.push_back(lastVertexPosTop);

        /*lightTrailMeshData.uvs.push_back(lightTrailMeshData.uvs[numVertices - 2]);
        lightTrailMeshData.uvs.push_back(lightTrailMeshData.uvs[numVertices - 1]);*/

        // create new normals
        lightTrailMeshData.normals.push_back(newNormal);
        lightTrailMeshData.normals.push_back(newNormal);

        numVertices += 2;
    }
    else
    {
        // haven't just started turning, so we share our normals
        // to make a smooth curve
        glm::vec3 averagedNormal = glm::normalize(newNormal + lightTrailMeshData.normals[numVertices - 2]);
        lightTrailMeshData.normals[numVertices - 2] = averagedNormal;
        lightTrailMeshData.normals[numVertices - 1] = averagedNormal;
    }

    // now we need to add the two new vertices for this face
    // which means two more vertices, two more normals and 6 more indices

    // currently in same location as previous vertices
    lightTrailMeshData.vertices.push_back(lightTrailMeshData.vertices[numVertices - 2]);
    lightTrailMeshData.vertices.push_back(lightTrailMeshData.vertices[numVertices - 1]);

    // TODO: do UVs properly
    /*lightTrailMeshData.uvs.push_back(glm::vec2(0.0f, 0.0f));
    lightTrailMeshData.uvs.push_back(glm::vec2(0.0f, 0.0f));*/

    lightTrailMeshData.normals.push_back(newNormal);
    lightTrailMeshData.normals.push_back(newNormal);

    numVertices += 2;
    lightTrailMeshData.indices.push_back(numVertices - 4); lightTrailMeshData.indices.push_back(numVertices - 3); lightTrailMeshData.indices.push_back(numVertices - 1);
    lightTrailMeshData.indices.push_back(numVertices - 4); lightTrailMeshData.indices.push_back(numVertices - 1); lightTrailMeshData.indices.push_back(numVertices - 2);
}

void LightTrail::stopTurning()
{
    if (turning)
    {
        turning = false;
        // just stopped turning, so to render this as flat
        // we need to create two extra vertices for the corner
        // with unique normals

        unsigned int numVertices = lightTrailMeshData.vertices.size();

        // duplicate positions
        glm::vec3 lastTopPos = lightTrailMeshData.vertices.back();
        lightTrailMeshData.vertices.pop_back();
        glm::vec3 lastBottomPos = lightTrailMeshData.vertices.back();
        lightTrailMeshData.vertices.pop_back();

        lightTrailMeshData.vertices.push_back(lightTrailMeshData.vertices[numVertices - 4]);
        lightTrailMeshData.vertices.push_back(lightTrailMeshData.vertices[numVertices - 3]);
        lightTrailMeshData.vertices.push_back(lastBottomPos);
        lightTrailMeshData.vertices.push_back(lastTopPos);

        // duplicate UVs
        /*glm::vec2 lastTopUV = lightTrailMeshData.uvs.back();
        lightTrailMeshData.uvs.pop_back();
        glm::vec2 lastBottomUV = lightTrailMeshData.uvs.back();
        lightTrailMeshData.uvs.pop_back();

        lightTrailMeshData.uvs.push_back(lightTrailMeshData.uvs[numVertices - 4]);
        lightTrailMeshData.uvs.push_back(lightTrailMeshData.uvs[numVertices - 3]);
        lightTrailMeshData.uvs.push_back(lastBottomUV);
        lightTrailMeshData.uvs.push_back(lastTopUV);*/

        // update normals
        // the last set of normals are normal to the plane
        // we just need to create those extra two vertices
        lightTrailMeshData.normals.push_back(lightTrailMeshData.normals[numVertices - 2]);
        lightTrailMeshData.normals.push_back(lightTrailMeshData.normals[numVertices - 1]);
        // the two normals before that were averaged with this last face
        // which we don't want anymore, calculate the new normals
        glm::vec3 newNormal = glm::normalize(glm::cross(lightTrailMeshData.normals[numVertices - 6] - lightTrailMeshData.normals[numVertices - 4],
                                             glm::vec3(0,1,0)));
        lightTrailMeshData.normals[numVertices - 4] = newNormal;
        lightTrailMeshData.normals[numVertices - 3] = newNormal;

        // update indices
        numVertices += 2;
        for (unsigned int i = 0; i < 6; i++)
        {
            lightTrailMeshData.indices.pop_back();
        }
        lightTrailMeshData.indices.push_back(numVertices - 4); lightTrailMeshData.indices.push_back(numVertices - 3); lightTrailMeshData.indices.push_back(numVertices - 1);
        lightTrailMeshData.indices.push_back(numVertices - 4); lightTrailMeshData.indices.push_back(numVertices - 1); lightTrailMeshData.indices.push_back(numVertices - 2);
    }
}

void LightTrail::updateLastVertices(const glm::vec3 &bikeLocationWorld)
{
    // update the positions of the last two vertices
    lightTrailMeshData.vertices.pop_back();
    lightTrailMeshData.vertices.pop_back();
    lightTrailMeshData.vertices.push_back(glm::vec3(bikeLocationWorld.x, 0.0f, bikeLocationWorld.z));
    lightTrailMeshData.vertices.push_back(glm::vec3(bikeLocationWorld.x, lightTrailHeight, bikeLocationWorld.z));
}
