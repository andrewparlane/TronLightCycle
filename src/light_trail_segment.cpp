#include "light_trail_segment.hpp"

#include <glm/gtc/constants.hpp>

#define DEBUG_MESH_DATA_HEIGHT 5.0f

#define DEBUG_LTS_STRAIGHT_COLOUR   glm::vec3(1.0f, 0.0f, 0.0f)
#define DEBUG_LTS_CIRCLE_COLOUR     glm::vec3(0.0f, 1.0f, 0.0f)
#define DEBUG_LTS_SPIRAL_COLOUR     glm::vec3(0.0f, 0.0f, 1.0f)

#define EPSILON         0.05f
#define CIRCLE_EPSILON  0.6f

#ifdef DEBUG_ALLOW_SELECTING_ACTIVE_LIGHT_TRAIL_SEGMENT
unsigned int LightTrailSegment::totalSegments = 0;
unsigned int LightTrailSegment::activeSegmentID = 0;
#endif

LightTrailSegment::LightTrailSegment(std::shared_ptr<World> _world, std::shared_ptr<const Shader> _shader)
    : world(_world), shader(_shader)
{
#ifdef DEBUG_ALLOW_SELECTING_ACTIVE_LIGHT_TRAIL_SEGMENT
    segmentID = ++totalSegments;
#endif
}

LightTrailSegment::~LightTrailSegment()
{
}

void LightTrailSegment::drawDebugMesh() const
{
#ifdef DEBUG_SHOW_LIGHT_TRAIL_SEGMENTS
    if (debugObj)
    {
#ifdef DEBUG_ALLOW_SELECTING_ACTIVE_LIGHT_TRAIL_SEGMENT
        if (activeSegmentID == 0 ||
            activeSegmentID == segmentID)
#endif
        {
            debugObj->drawAll();
        }
    }
#endif
}

// STRAIGHT ===================================================================

LightTrailSegmentStraight::LightTrailSegmentStraight(const glm::vec2 &_start, float currentAngleRads, std::shared_ptr<World> _world, std::shared_ptr<const Shader> _shader)
    : LightTrailSegment(_world, _shader), start(_start), end(_start)
{
#ifdef DEBUG_SHOW_LIGHT_TRAIL_SEGMENTS
    debugMeshData.name = "LTS_STRAIGHT";
    debugMeshData.hasTexture = false;

    debugMeshData.vertices.push_back(glm::vec3(start.x, 0, start.y));
    debugMeshData.vertices.push_back(glm::vec3(start.x, DEBUG_MESH_DATA_HEIGHT, start.y));
    debugMeshData.vertices.push_back(glm::vec3(end.x,   DEBUG_MESH_DATA_HEIGHT, end.y));
    debugMeshData.vertices.push_back(glm::vec3(end.x,   0, end.y));

    glm::vec3 normal = glm::normalize(glm::vec3(glm::vec4(1,0,0,1) * glm::rotate(currentAngleRads, glm::vec3(0,-1,0))));
    debugMeshData.normals.push_back(normal);
    debugMeshData.normals.push_back(normal);
    debugMeshData.normals.push_back(normal);
    debugMeshData.normals.push_back(normal);

    debugMeshData.indices.push_back(0); debugMeshData.indices.push_back(1); debugMeshData.indices.push_back(2);
    debugMeshData.indices.push_back(0); debugMeshData.indices.push_back(2); debugMeshData.indices.push_back(3);

    debugObjData = std::make_shared<ObjData3D>();
    if (!debugObjData->addMesh(debugMeshData))
    {
        // fali
        printf("Failed to create light trail segment straight obj data\n");
    }
    debugObj = std::make_unique<Object>(debugObjData, world, shader, glm::mat4(1.0f), DEBUG_LTS_STRAIGHT_COLOUR);
#endif
}

LightTrailSegmentStraight::~LightTrailSegmentStraight()
{
}

bool LightTrailSegmentStraight::collides(const glm::vec2 &location) const
{
#ifdef DEBUG_ALLOW_SELECTING_ACTIVE_LIGHT_TRAIL_SEGMENT
    if (activeSegmentID != 0 &&
        activeSegmentID != segmentID)
    {
        return false;
    }
#endif

    // distance(a,p) + distance(b,p) == distance(a,b)
    float dist = abs(glm::distance(location, start) + 
                     glm::distance(location, end) -
                     glm::distance(end, start));
    return (dist < EPSILON);
}

bool LightTrailSegmentStraight::checkSelfCollision() const
{
    // while drawing a straight line, we can't crash into it
    return false;
}

void LightTrailSegmentStraight::update(const glm::vec2 &currentLocation, float currentAngleRads)
{
    end = currentLocation;

#ifdef DEBUG_SHOW_LIGHT_TRAIL_SEGMENTS
    debugMeshData.vertices.pop_back();
    debugMeshData.vertices.pop_back();
    debugMeshData.vertices.push_back(glm::vec3(end.x,   DEBUG_MESH_DATA_HEIGHT, end.y));
    debugMeshData.vertices.push_back(glm::vec3(end.x,   0, end.y));

    debugObjData->updateMesh(debugMeshData);
    debugObjData->updateBuffers();
#endif
}

// CIRCLE =====================================================================

LightTrailSegmentCircle::LightTrailSegmentCircle(const glm::vec2 &_centre, float _radius, float _startAngleRads, TurnDirection _turnDirection, std::shared_ptr<World> _world, std::shared_ptr<const Shader> _shader)
    : LightTrailSegment(_world, _shader),
      centre(_centre), radius(_radius),
      startAngleRads(_startAngleRads), stopAngleRads(_startAngleRads),
      turnDirection(_turnDirection)
{
#ifdef DEBUG_SHOW_LIGHT_TRAIL_SEGMENTS
    debugMeshData.name = "LTS_CIRCLE";
    debugMeshData.hasTexture = false;

    glm::vec2 tmp(sin(startAngleRads), -cos(startAngleRads));
    glm::vec2 point = (radius * tmp) + centre;

    debugMeshData.vertices.push_back(glm::vec3(point.x, 0, point.y));
    debugMeshData.vertices.push_back(glm::vec3(point.x, DEBUG_MESH_DATA_HEIGHT, point.y));
    debugMeshData.vertices.push_back(glm::vec3(point.x, 0, point.y));
    debugMeshData.vertices.push_back(glm::vec3(point.x, DEBUG_MESH_DATA_HEIGHT, point.y));

    glm::vec3 normal = glm::normalize(glm::vec3(tmp.x, 0, tmp.y));
    debugMeshData.normals.push_back(normal);
    debugMeshData.normals.push_back(normal);
    debugMeshData.normals.push_back(normal);
    debugMeshData.normals.push_back(normal);

    unsigned int numVertices = debugMeshData.vertices.size();
    debugMeshData.indices.push_back(numVertices - 4);
    debugMeshData.indices.push_back(numVertices - 3);
    debugMeshData.indices.push_back(numVertices - 1);

    debugMeshData.indices.push_back(numVertices - 4);
    debugMeshData.indices.push_back(numVertices - 1);
    debugMeshData.indices.push_back(numVertices - 2);

    debugObjData = std::make_shared<ObjData3D>();
    if (!debugObjData->addMesh(debugMeshData))
    {
        // fali
        printf("Failed to create light trail segment circle obj data\n");
    }
    debugObj = std::make_unique<Object>(debugObjData, world, shader, glm::mat4(1.0f), DEBUG_LTS_CIRCLE_COLOUR);
#endif
}

LightTrailSegmentCircle::~LightTrailSegmentCircle()
{
}

void LightTrailSegmentCircle::update(const glm::vec2 &currentLocation, float currentAngleRads)
{
    // need angle between centre and -ve Z
    // and centre and currentLocation
    // A DOT B = |A||B| cos theta
    // unfortunately I can only then recover theta to between 0 and PI
    // which makes it hard to work out what our actual angle is
    // I check if we are left or right of the centre and adjust if needed

    stopAngleRads = glm::acos(glm::dot(glm::vec2(0,-1), glm::normalize(currentLocation - centre)));
    if (currentLocation.x < centre.x)
    {
        stopAngleRads = 2*glm::pi<float>() - stopAngleRads;
    }

#ifdef DEBUG_SHOW_LIGHT_TRAIL_SEGMENTS
    glm::vec2 tmp(sin(stopAngleRads), -cos(stopAngleRads));
    glm::vec2 point = (radius * tmp) + centre;

    debugMeshData.vertices.push_back(glm::vec3(point.x, 0, point.y));
    debugMeshData.vertices.push_back(glm::vec3(point.x, DEBUG_MESH_DATA_HEIGHT, point.y));

    glm::vec3 normal = glm::normalize(glm::vec3(tmp.x, 0, tmp.y));
    debugMeshData.normals.push_back(normal);
    debugMeshData.normals.push_back(normal);

    unsigned int numVertices = debugMeshData.vertices.size();
    debugMeshData.indices.push_back(numVertices - 4);
    debugMeshData.indices.push_back(numVertices - 3);
    debugMeshData.indices.push_back(numVertices - 1);

    debugMeshData.indices.push_back(numVertices - 4);
    debugMeshData.indices.push_back(numVertices - 1);
    debugMeshData.indices.push_back(numVertices - 2);

    debugObjData->updateMesh(debugMeshData);
    debugObjData->updateBuffers();
#endif
}

bool LightTrailSegmentCircle::collides(const glm::vec2 &location) const
{
#ifdef DEBUG_ALLOW_SELECTING_ACTIVE_LIGHT_TRAIL_SEGMENT
    if (activeSegmentID != 0 &&
        activeSegmentID != segmentID)
    {
        return false;
    }
#endif

    // first am I a distance of radius away from the centre
    float dist = abs(glm::distance(location, centre) - radius);
    if (dist > CIRCLE_EPSILON)
    {
        return false;
    }

    float currentAngleRads = glm::acos(glm::dot(glm::vec2(0,-1), glm::normalize(location - centre)));
    if (location.x < centre.x)
    {
        currentAngleRads = 2*glm::pi<float>() - currentAngleRads;
    }

    // now am I within the start and end angle
    // two cases, turning left, turning right
    if (turnDirection == TURN_RIGHT)
    {
        // two more cases
        // 1) segment doesn't cross angle 0 (start < end)
        // 2) segment crosses angle 0, (start > end)
        if (startAngleRads < stopAngleRads)
        {
            // current angle is between start and end
            return (currentAngleRads > startAngleRads &&
                    currentAngleRads < stopAngleRads);
        }
        else
        {
            // current angle is > start || < end
            return (currentAngleRads > startAngleRads ||
                    currentAngleRads < stopAngleRads);
        }
    }
    else // turning left
    {
        // two more cases
        // 1) segment doesn't cross angle 0 (end < start)
        // 2) segment crosses angle 0, (end > start)
        if (stopAngleRads < startAngleRads)
        {
            // current angle is between start and end
            return (currentAngleRads < startAngleRads &&
                    currentAngleRads > stopAngleRads);
        }
        else
        {
            // current angle is < start || > end
            return (currentAngleRads < startAngleRads ||
                    currentAngleRads > stopAngleRads);
        }
    }
}

bool LightTrailSegmentCircle::checkSelfCollision() const
{
#ifdef DEBUG_ALLOW_SELECTING_ACTIVE_LIGHT_TRAIL_SEGMENT
    if (activeSegmentID != 0 &&
        activeSegmentID != segmentID)
    {
        return false;
    }
#endif

    // calculate angle of current arch
    float theta;

    // two cases, turning left, turning right
    if (turnDirection == TURN_RIGHT)
    {
        // two more cases
        // 1) segment doesn't cross angle 0 (start < end)
        // 2) segment crosses angle 0, (start > end)
        if (startAngleRads < (stopAngleRads+0.01f))
        {
            theta = stopAngleRads - startAngleRads;
        }
        else
        {
            theta = stopAngleRads + (2*glm::pi<float>() - startAngleRads);
        }
    }
    else // turning left
    {
        // two more cases
        // 1) segment doesn't cross angle 0 (end < start)
        // 2) segment crosses angle 0, (end > start)
        if (stopAngleRads < (startAngleRads+0.01f))
        {
            theta = startAngleRads - stopAngleRads;
        }
        else
        {
            theta = startAngleRads + (2*glm::pi<float>() - stopAngleRads);
        }
    }

    return (theta > glm::radians(330.0f));
}

// SPIRAL =====================================================================

LightTrailSegmentSpiral::LightTrailSegmentSpiral(const glm::vec2 &_startPoint, float _startSpeed, float _startAngleRads, TurnDirection _turnDirection, Accelerating _accelerating, std::shared_ptr<World> _world, std::shared_ptr<const Shader> _shader)
    : LightTrailSegment(_world, _shader),
      startSpeed(_startSpeed), startAngleRads(_startAngleRads),
      endAngleRads(_startAngleRads), startPoint(_startPoint),
      turnDirection(_turnDirection), accelerating(_accelerating)
#ifdef DEBUG_SHOW_LIGHT_TRAIL_SEGMENTS
      , debugLastTDrawn(0)
#endif
{
    // cache all spiral co-ords indexed by angle from start position
    // with -ve Z being 0 radians

    // we can calculate which values of T we need to run the calculation for
    // based on max speed, start speed and acceleration
    float maxT;
    if (accelerating == SPEED_ACCELERATE)
    {
        maxT = (BIKE_SPEED_FASTEST - startSpeed) / RATE_OF_ACCELERATE;
    }
    else
    {
        maxT = (startSpeed - BIKE_SPEED_SLOWEST) / RATE_OF_ACCELERATE;
    }

    // calculate for half values of T. Since the bike can move at a max speed
    // of 0.6 units per second, whole values of T correspond to points on the
    // spiral that are at most 0.6 units apart. Therefore half values of T
    // shouldn't be further than 0.3 unites apart

#ifdef DEBUG_SHOW_LIGHT_TRAIL_SEGMENTS
    glm::vec2 lastPoint;
    bool first = true;
#endif
    for (float T = 0.5f; T < (maxT + 0.6f); T+=0.5f)
    {
        glm::vec2 point = calculateSpiralCoOrdsForT(T);

        // now calculate angle to start point
        float angleRads = glm::acos(glm::dot(glm::vec2(0,-1), glm::normalize(point - startPoint)));
        if (point.x < startPoint.x)
        {
            angleRads = 2*glm::pi<float>() - angleRads;
        }

        // stick it in the cache
        cache.insert(SpiralCacheMapValue(angleRads, std::pair<float, glm::vec2>(T, point)));

#ifdef DEBUG_SHOW_LIGHT_TRAIL_SEGMENTS
        debugMeshData.vertices.push_back(glm::vec3(point.x, 0, point.y));
        debugMeshData.vertices.push_back(glm::vec3(point.x, DEBUG_MESH_DATA_HEIGHT, point.y));

        glm::vec2 difference;
        if (first)
        {
            first = false;
            difference = point - startPoint;
        }
        else
        {
            difference = point - lastPoint;
        }
        glm::vec3 normal = glm::cross(glm::normalize(glm::vec3(difference.x, 0, difference.y)),
                                      glm::vec3(0,1,0));
        debugMeshData.normals.push_back(normal);
        debugMeshData.normals.push_back(normal);

        lastPoint = point;
#endif
    }

#ifdef DEBUG_SHOW_LIGHT_TRAIL_SEGMENTS
    debugMeshData.name = "LTS_SPIRAL";
    debugMeshData.hasTexture = false;

    // need something in the indices buffer
    // bit of a nasty hack, but it'll work
    debugMeshData.indices.push_back(0); debugMeshData.indices.push_back(0); debugMeshData.indices.push_back(0);

    debugObjData = std::make_shared<ObjData3D>();
    if (!debugObjData->addMesh(debugMeshData))
    {
        // fali
        printf("Failed to create light trail segment spiral obj data\n");
    }
    debugObj = std::make_unique<Object>(debugObjData, world, shader, glm::mat4(1.0f), DEBUG_LTS_SPIRAL_COLOUR);
#endif
}

LightTrailSegmentSpiral::~LightTrailSegmentSpiral()
{
}

glm::vec2 LightTrailSegmentSpiral::calculateSpiralCoOrdsForT(float T) const
{
    // formulae for our spiral is pretty complicated
    // see notes/light_trail_spiral.ods

    // C = angle of turn per frame in radians
    // A = rate of acceleration
    // T = frame number since spiral start
    // U = initial speed
    // Theta = initial angle of bike
    float C = (turnDirection == TURN_RIGHT) ? glm::radians(ANGLE_OF_TURNS) : -glm::radians(ANGLE_OF_TURNS);
    float A = (accelerating == SPEED_ACCELERATE) ? RATE_OF_ACCELERATE : -RATE_OF_ACCELERATE;
    float U = startSpeed;
    float Theta = startAngleRads;

    float cosTheta = cos(Theta);
    float sinTheta = sin(Theta);
    float cosCTPlusTheta = cos(C * T + Theta);
    float sinCTPlusTheta = sin(C * T + Theta);

    float pointX = (U / C) * (cosTheta - cosCTPlusTheta) +
                   (A / (C * C)) * (sinCTPlusTheta - sinTheta) -
                   (A * T / C) * (cosCTPlusTheta);

    float pointZ = (U / C) * (sinCTPlusTheta - sinTheta) +
                   (A / (C * C)) * (cosCTPlusTheta - cosTheta) +
                   (A * T / C) * (sinCTPlusTheta);

    // using -pointZ as my spiral equation assumes angle 0
    // equates to +ve Z whereas it's actually -ve
    return glm::vec2(pointX, -pointZ) + startPoint;
}

bool LightTrailSegmentSpiral::collides(const glm::vec2 &location) const
{
#ifdef DEBUG_ALLOW_SELECTING_ACTIVE_LIGHT_TRAIL_SEGMENT
    if (activeSegmentID != 0 &&
        activeSegmentID != segmentID)
    {
        return false;
    }
#endif

    // current angle from start point with -VE Z being 0 radians
    float currentAngleRads = glm::acos(glm::dot(glm::vec2(0,-1), glm::normalize(location - startPoint)));
    if (location.x < startPoint.x)
    {
        currentAngleRads = 2*glm::pi<float>() - currentAngleRads;
    }

    // find if a suitably close value is in my cache
    auto itLow = cache.lower_bound(currentAngleRads);   // find first key >= currentAngleRads

    // work out whether currentAngleRads is closer to the item in the cache
    // that is >= it or < it
    auto &nearest = itLow;
    if (itLow == cache.end())
    {
        nearest = --itLow;
    }
    else if (itLow == cache.begin())
    {
        nearest = itLow;
    }
    else
    {
        auto &firstGreaterOrEqual = itLow;
        auto &firstLessThan = --itLow;

        if (abs(currentAngleRads - firstLessThan->first) <
            abs(currentAngleRads - firstGreaterOrEqual->first))
        {
            nearest = firstLessThan;
        }
        else
        {
            nearest = firstGreaterOrEqual;
        }
    }

    // now check if nearest is near enough
    if (abs(currentAngleRads - nearest->first) > glm::radians(1.5f))
    {
        // too far
        return false;
    }

    // we are in the max range of this spiral,
    // however the spiral doesn't need to be that big
    // check we are in range
    // endAngleRads = CT+Theta, where T is the last T of the spiral
    float turnAngleRads = glm::radians(ANGLE_OF_TURNS);
    if (turnDirection == TURN_LEFT)
    {
        turnAngleRads = -turnAngleRads;
    }

    float lastT = (endAngleRads - startAngleRads) / turnAngleRads;
    if (nearest->second.first > lastT)
    {
        // not in range
        return false;
    }

    // get the point on the spiral that is
    // nearest.first radians from the start point.
    // Then we can see if the bike is near that point
    float dist = glm::distance(location, nearest->second.second);

    if (dist > CIRCLE_EPSILON)
    {
        return false;
    }

    return true;
}

bool LightTrailSegmentSpiral::checkSelfCollision() const
{
    // can't hit ourself doing a spiral
    return false;
}

void LightTrailSegmentSpiral::update(const glm::vec2 &currentLocation, float currentAngleRads)
{
    endAngleRads = currentAngleRads;

#ifdef DEBUG_SHOW_LIGHT_TRAIL_SEGMENTS
    bool anythingChanged = false;

    // CT+Theta = currentAngleRads
    float C = (turnDirection == TURN_RIGHT) ? glm::radians(ANGLE_OF_TURNS) : -glm::radians(ANGLE_OF_TURNS);
    float T = (currentAngleRads - startAngleRads) / C;

    // we have debugMeshData for 1/2 values of T, 0.5, 1, 1.5, ...
    float t;
    for (t = debugLastTDrawn + 0.5f; (t+0.1f) <= T; t+=0.5f)
    {
        // vertices 0,1 are for T = 0.5
        //          2,3 are for T = 1
        // etc...

        // so we need to draw a face that ends with vertices defined by t
        // and starts with those defined by t-0.5

        if (t < 0.9f)
        {
            // t is 0.5f ie. there is no vertices defined by t-0.5
            continue;
        }

        unsigned int startVertexNum = (unsigned int)round((t - 1.0f)*4.0f);
        if (startVertexNum + 3 >= debugMeshData.vertices.size())
        {
            // not enough vertices?
            break;
        }
        debugMeshData.indices.push_back(startVertexNum + 0);
        debugMeshData.indices.push_back(startVertexNum + 1);
        debugMeshData.indices.push_back(startVertexNum + 3);

        debugMeshData.indices.push_back(startVertexNum + 0);
        debugMeshData.indices.push_back(startVertexNum + 3);
        debugMeshData.indices.push_back(startVertexNum + 2);

        anythingChanged = 1;
    }
    debugLastTDrawn = t-0.5f;

    if (anythingChanged)
    {
        debugObjData->updateMesh(debugMeshData);
        debugObjData->updateBuffers();
    }
#endif
}
