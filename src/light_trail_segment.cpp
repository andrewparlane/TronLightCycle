#include "light_trail_segment.hpp"

#include <glm/gtc/constants.hpp>

#define EPSILON         0.01f
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

void LightTrailSegmentStraight::update(const glm::vec2 &currentLocation)
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

void LightTrailSegmentCircle::update(const glm::vec2 &currentLocation)
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

LightTrailSegmentSpiral::LightTrailSegmentSpiral(const glm::vec2 &_startPoint, float _startSpeed, float _startAngleRads)
    : LightTrailSegment(), startPoint(_startPoint), startSpeed(_startSpeed), startAngleRads(_startAngleRads)
{
}

LightTrailSegmentSpiral::~LightTrailSegmentSpiral()
{
}

bool LightTrailSegmentSpiral::collides(const glm::vec2 &location) const
{
    return false;
}

bool LightTrailSegmentSpiral::checkSelfCollision() const
{
    // can't hit ourself doing a spiral
    return false;
}

void LightTrailSegmentSpiral::update(const glm::vec2 &currentLocation)
{
}
