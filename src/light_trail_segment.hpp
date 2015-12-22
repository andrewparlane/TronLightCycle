#ifndef __LIGHT_TRAIL_SEGMENT_HPP
#define __LIGHT_TRAIL_SEGMENT_HPP

#include "bike_movements.hpp"

#include <glm/glm.hpp>

#include <map>

class LightTrailSegment
{
public:
    LightTrailSegment();
    virtual ~LightTrailSegment();

    virtual bool collides(const glm::vec2 &location) const = 0;
    virtual bool checkSelfCollision() const = 0;
    virtual void update(const glm::vec2 &currentLocation, float currentAngleRads) = 0;
};

class LightTrailSegmentStraight : public LightTrailSegment
{
public:
    LightTrailSegmentStraight(const glm::vec2 &start);
    ~LightTrailSegmentStraight();

    bool collides(const glm::vec2 &location) const override;
    bool checkSelfCollision() const override;
    void update(const glm::vec2 &currentLocation, float currentAngleRads) override;

protected:
    glm::vec2 start;    // only x and z, don't need y
    glm::vec2 end;
};

class LightTrailSegmentCircle : public LightTrailSegment
{
public:
    LightTrailSegmentCircle(const glm::vec2 &_centre, float _radius, float _startAngleRads, TurnDirection _turnDirection);
    ~LightTrailSegmentCircle();

    bool collides(const glm::vec2 &location) const override;
    bool checkSelfCollision() const override;
    void update(const glm::vec2 &currentLocation, float currentAngleRads) override;

protected:
    glm::vec2 centre;   // only x and z, don't need y
    float radius;
    float startAngleRads;
    float stopAngleRads;
    TurnDirection turnDirection;
};

class LightTrailSegmentSpiral : public LightTrailSegment
{
public:
    LightTrailSegmentSpiral(const glm::vec2 &_startPoint, float _startSpeed, float _startAngleRads, TurnDirection _turnDirection, Accelerating _accelerating);
    ~LightTrailSegmentSpiral();

    bool collides(const glm::vec2 &location) const override;
    bool checkSelfCollision() const override;
    void update(const glm::vec2 &currentLocation, float currentAngleRads) override;

protected:
    glm::vec2 calculateSpiralCoOrdsForT(float T) const;

    float startSpeed;
    float startAngleRads;
    float endAngleRads;
    glm::vec2 startPoint;
    TurnDirection turnDirection;
    Accelerating accelerating;

    // angle of point to startPoint -> pair of T, point on spiral
    typedef std::map<float,std::pair<float, glm::vec2>> SpiralCacheMap;
    typedef SpiralCacheMap::value_type SpiralCacheMapValue;
    SpiralCacheMap cache;
};

#endif
