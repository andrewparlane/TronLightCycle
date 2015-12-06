#ifndef __BIKE_HPP
#define __BIKE_HPP

#include "object.hpp"
#include "light_trail.hpp"

class Bike : public Object
{
public:

    enum TurnDirection
    {
        NO_TURN = 0,
        TURN_LEFT,
        TURN_RIGHT
    };

    enum Speed
    {
        SPEED_NORMAL = 0,
        SPEED_ACCELERATE,
        SPEED_BRAKE
    };

    Bike(std::shared_ptr<const ObjData3D> _objData, 
        std::shared_ptr<World> _world, 
        std::shared_ptr<const Shader> _shader,
        const glm::mat4 &modelMat,
        const glm::vec3 &_defaultColour = glm::vec3(0,0,0));
    ~Bike();

    void updateLocation();
    void turn(TurnDirection dir);
    void updateSpeed(Speed s);

    void toggleLightTrail() { trail.toggle(); }
    void updateLightTrail();

protected:
    // overrides as protected, as we shouldn't use these, we should use updateLocation() and turn()
    void translate(const glm::vec3 &vec) override { Object::translate(vec); }
    void rotate(float radians, const glm::vec3 &axis) override { Object::rotate(radians, axis); }

    void internalDrawAll(const std::vector<std::shared_ptr<Mesh<glm::vec3>>> &meshes) const override;

    void initialiseBikeParts();

    float bikeAngleAroundYRads;
    float wheelAngle;
    float engineAngle;

    std::vector<unsigned int> frontTyreMeshIndexes;
    std::vector<unsigned int> backTyreMeshIndexes;
    std::vector<unsigned int> leftEngineIndexes;
    std::vector<unsigned int> rightengineIndexes;
    std::vector<unsigned int> remainderIndexes;

    MeshAxis frontTyreAxis;
    MeshAxis backTyreAxis;
    MeshAxis rightEngineAxis;
    MeshAxis leftEngineAxis;

    // note lightTrail is in world co-ords
    LightTrail trail;

    float speed;
};

#endif
