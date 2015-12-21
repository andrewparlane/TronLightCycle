#include "light_trail.hpp"

static const float lightTrailHeight = 3.8f;

LightTrail::LightTrail(std::shared_ptr<World> _world,
                       std::shared_ptr<const Shader> _shader,
                       glm::vec3 _colour,
                       TurnDirection turning,
                       Accelerating accelerating)
    : world(_world), shader(_shader), colour(_colour),
      stopping(false), isStopped(false)
{
    state = calculateState(turning, accelerating);
}

LightTrail::~LightTrail()
{
}

void LightTrail::createObject(glm::vec3 currentLocation, float currentAngleRads)
{
    MeshData<glm::vec3> &md = lightTrailMeshData;

    md.indices.empty();
    md.vertices.empty();
    md.uvs.empty();
    md.normals.empty();
    md.colours.empty();

    md.name = "LT";
    md.hasTexture = false;
    //md.texturePath = "light_trail.DDS";
    md.vertices.push_back(glm::vec3(currentLocation.x, 0.0f,             currentLocation.z));    // bottom nearest
    md.vertices.push_back(glm::vec3(currentLocation.x, lightTrailHeight, currentLocation.z));    // top nearest
    md.vertices.push_back(glm::vec3(currentLocation.x, 0.0f,             currentLocation.z));    // bottom furthest
    md.vertices.push_back(glm::vec3(currentLocation.x, lightTrailHeight, currentLocation.z));    // top furthest

    glm::vec3 normal = glm::normalize(glm::vec3(glm::vec4(1,0,0,1) * glm::rotate(currentAngleRads, glm::vec3(0,-1,0))));
    md.normals.push_back(normal);
    md.normals.push_back(normal);
    md.normals.push_back(normal);
    md.normals.push_back(normal);

    /*md.uvs.push_back(glm::vec2(0.0f, 0.0f));
    md.uvs.push_back(glm::vec2(0.0f, 1.0f));
    md.uvs.push_back(glm::vec2(0.1f, 0.0f));
    md.uvs.push_back(glm::vec2(0.1f, 1.0f));*/

    md.indices.push_back(0); md.indices.push_back(1); md.indices.push_back(3);
    md.indices.push_back(0); md.indices.push_back(3); md.indices.push_back(2);

    lightTrailObjData = std::make_shared<ObjData3D>();
    if (!lightTrailObjData->addMesh(md))
    {
        // fali
        printf("Failed to create light trail obj data\n");
    }
    lightTrailObj = std::make_unique<Object>(lightTrailObjData, world, shader, glm::mat4(1.0f));
    lightTrailObj->setDefaultColour(colour);
}

LightTrail::State LightTrail::calculateState(TurnDirection turning, Accelerating accelerating) const
{
    State result = STATE_STRAIGHT;
    if (turning != NO_TURN && accelerating != SPEED_NORMAL)
    {
        // spiraling
        if (accelerating == SPEED_ACCELERATE)
        {
            if (turning == TURN_LEFT)
            {
                result = STATE_SPIRAL_OUT_LEFT;
            }
            else
            {
                result = STATE_SPIRAL_OUT_RIGHT;
            }
        }
        else
        {
            if (turning == TURN_LEFT)
            {
                result = STATE_SPIRAL_IN_LEFT;
            }
            else
            {
                result = STATE_SPIRAL_IN_RIGHT;
            }
        }
    }
    else if (turning == TURN_LEFT)
    {
        result = STATE_CIRCLE_LEFT;
    }
    else if (turning == TURN_RIGHT)
    {
        result = STATE_CIRCLE_RIGHT;
    }
    return result;
}

void LightTrail::turn(float currentAngleRads, bool justStarted)
{
    // because the bike has turned, we need to add a new face
    // to our light trail data.

    MeshData<glm::vec3> &md = lightTrailMeshData;

    unsigned int numVertices = md.vertices.size();
    glm::vec3 lastVertexPosBottom = md.vertices[numVertices - 2];
    glm::vec3 lastVertexPosTop = md.vertices[numVertices - 1];
    // wall is alwasy verticle, we know which way the bike is facing
    // so normal is 90 degrees (rotated around y) from bike direction
    // and since the bike is orientated along Z, we can just rotate a unit vector along x by bikeAngle
    glm::vec3 newNormal = glm::normalize(glm::vec3(glm::vec4(1,0,0,1) * glm::rotate(currentAngleRads, glm::vec3(0,-1,0))));

    // if we just started turning we don't want the past long wall
    // to look curved, ie. don't average normals for the corner vertex
    // so we need to duplicate the last two vertices (corner vertices),
    // and amend the normals

    if (justStarted)
    {
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

void LightTrail::stopTurning()
{
    MeshData<glm::vec3> &md = lightTrailMeshData;

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

void LightTrail::updateLastVertices(glm::vec3 currentLocation)
{
    MeshData<glm::vec3> &md = lightTrailMeshData;
    // update the positions of the last two vertices
    md.vertices.pop_back();
    md.vertices.pop_back();
    md.vertices.push_back(glm::vec3(currentLocation.x, 0.0f, currentLocation.z));
    md.vertices.push_back(glm::vec3(currentLocation.x, lightTrailHeight, currentLocation.z));
}

void LightTrail::createNewPathSegment(float speed, glm::vec3 currentLocation, float currentAngleRads)
{
    std::unique_ptr<LightTrailSegment> uptr;
    switch (state)
    {
        case STATE_STRAIGHT:
        {
            uptr = std::make_unique<LightTrailSegmentStraight>(glm::vec2(currentLocation.x, currentLocation.z));
            break;
        }
        case STATE_CIRCLE_LEFT:
        case STATE_CIRCLE_RIGHT:
        {
            // circ = 2PI*r
            // because our circle is made of 360/ANGLE_OF_TURNS edges
            // each of length speed, we have:
            // circ = speed * 2PI / RADIANS(ANGLE_OF_TURNS)
            // therefore r = speed / RADIANS(ANGLE_OF_TURNS)
            // note: this is just an approximation. However for:
            // angle = 2 degrees and speed = 0.2 - error = 0.0006
            // angle = 2 degrees and speed = 0.6 - error = 0.0017
            // 0.0017 is significantly less than the slowest speed of 0.2
            // so we should be good.

            // for reference the formulae that calculates the exact radius
            // is r = speed((SUM between n=1 and n=(2PI/4A)-1 of sin(nA))+1/2)
            // where A is RADIANS(ANGLE_OF_TURNS)
            // see: notes/calculating_radius_of_circle.png
            //  This shape has 30 degree angles and sides of length S.
            //  length of (1) = S*sin(30)
            //  length of (2) = S*sin(60)
            //  length of (3) = S/2
            //  (1) + (2) + (3) = radius

            float radius = speed / glm::radians(ANGLE_OF_TURNS);

            // now to find the centre
            // currentAngle = 0 degrees means we are going straight along -ve Z
            // so if we turn right our centre is directly to the right of us
            // if we turn left, then it's to our left.
            glm::vec3 radiusVector = glm::vec3(glm::vec4((state == STATE_CIRCLE_RIGHT) ? radius : -radius, 0, 0, 1) *
                                               glm::rotate(currentAngleRads, glm::vec3(0,1,0)));
            glm::vec3 centre = currentLocation + radiusVector;

            // finally calculate startAngle
            // this is the angle between th ebike and the centre point with respect to -ve Z
            // ie. straight forward.
            // this is pretty simple, it's 90 degrees offset from currentAngle
            // if we turn right we are -90 degrees, and for left +90 degrees
            float startAngle = currentAngleRads + glm::radians((state == STATE_CIRCLE_RIGHT) ? -90.0f : 90.0f);
            // startAngle should be between 0 and 2PI
            while (startAngle >= 2*glm::pi<float>())
            {
                startAngle -= 2*glm::pi<float>();
            }
            while (startAngle < 0)
            {
                startAngle += 2*glm::pi<float>();
            }

            uptr = std::make_unique<LightTrailSegmentCircle>(glm::vec2(centre.x, centre.z),
                                                             radius,
                                                             startAngle, 
                                                             (state == STATE_CIRCLE_RIGHT) ? TURN_RIGHT : TURN_LEFT);
            break;
        }
        case STATE_SPIRAL_OUT_LEFT:
        case STATE_SPIRAL_OUT_RIGHT:
        case STATE_SPIRAL_IN_LEFT:
        case STATE_SPIRAL_IN_RIGHT:
        {
            uptr = std::make_unique<LightTrailSegmentSpiral>(glm::vec2(currentLocation.x, currentLocation.z), speed, currentAngleRads);
            break;
        }
    }
    pathSegments.push_back(std::move(uptr));
}

void LightTrail::update(TurnDirection turning, Accelerating accelerating, float speed, glm::vec3 currentLocation, float currentAngleRads)
{
    // are we stopping? if so fade down until we are dead
    if (stopping)
    {
        bool changedSomething = false;
        for (auto &v : lightTrailMeshData.vertices)
        {
            if (v.y > 0.05f)
            {
                v.y -= 0.05f;
                changedSomething = true;
            }
        }
        if (!changedSomething)
        {
            isStopped = true;
        }

        // nothing more to do, as we are stopping
        lightTrailObjData->updateMesh(lightTrailMeshData);
        lightTrailObjData->updateBuffers();
        return;
    }

    // check if we have an object and object data ptrs
    // if not create them and the initial face
    if (!lightTrailObjData || !lightTrailObj)
    {
        createObject(currentLocation, currentAngleRads);
    }

    // create initial path segment if needed
    if (pathSegments.size() == 0)
    {
        createNewPathSegment(speed, currentLocation, currentAngleRads);
    }

    // deal with turning
    // this creates new faces and sorts out normals
    // so that our curves are smooth
    if (turning != NO_TURN)
    {
        turn(currentAngleRads, (state == STATE_STRAIGHT));
    }
    else if (state != STATE_STRAIGHT)
    {
        stopTurning();
    }

    // update last vertices to be current bike location
    updateLastVertices(currentLocation);

    // calculate new state
    State newState = calculateState(turning, accelerating);

    if (state != newState)
    {
        state = newState;
        createNewPathSegment(speed, currentLocation, currentAngleRads);
    }
    else
    {
        // update current path segment
        pathSegments.back()->update(glm::vec2(currentLocation.x, currentLocation.z));
    }

    lightTrailObjData->updateMesh(lightTrailMeshData);
    lightTrailObjData->updateBuffers();
}

bool LightTrail::collides(const glm::vec2 &location) const
{
    for (auto &ps : pathSegments)
    {
        if (ps->collides(location))
        {
            return true;
        }
    }
    return false;
}

bool LightTrail::checkSelfCollision() const
{
    if (pathSegments.size())
    {
        return pathSegments.back()->checkSelfCollision();
    }
    return false;
}
