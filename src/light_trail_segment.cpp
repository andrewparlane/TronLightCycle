#include "light_trail_segment.hpp"

#include <glm/gtc/constants.hpp>

#define EPSILON         0.05f
#define CIRCLE_EPSILON  0.6f

LightTrailSegment::LightTrailSegment()
{
}

LightTrailSegment::~LightTrailSegment()
{
}

// STRAIGHT ===================================================================

LightTrailSegmentStraight::LightTrailSegmentStraight(const glm::vec2 &_start)
    : LightTrailSegment(), start(_start), end(_start)
{
}

LightTrailSegmentStraight::~LightTrailSegmentStraight()
{
}

bool LightTrailSegmentStraight::collides(const glm::vec2 &location) const
{
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
}

// CIRCLE =====================================================================

LightTrailSegmentCircle::LightTrailSegmentCircle(const glm::vec2 &_centre, float _radius, float _startAngleRads, TurnDirection _turnDirection)
    : LightTrailSegment(), centre(_centre), radius(_radius),
      startAngleRads(_startAngleRads), stopAngleRads(_startAngleRads),
      turnDirection(_turnDirection)
{
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
}

bool LightTrailSegmentCircle::collides(const glm::vec2 &location) const
{
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

LightTrailSegmentSpiral::LightTrailSegmentSpiral(const glm::vec2 &_startPoint, float _startSpeed, float _startAngleRads, TurnDirection _turnDirection, Accelerating _accelerating)
    : LightTrailSegment(), startSpeed(_startSpeed), startAngleRads(_startAngleRads),
      endAngleRads(_startAngleRads), startPoint(_startPoint),
      turnDirection(_turnDirection), accelerating(_accelerating)
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
    }
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
}
