#ifndef __BIKE_HPP
#define __BIKE_HPP

#include "object.hpp"
#include "light_trail_manager.hpp"
#include "bike_movements.hpp"

class Bike : public Object
{
public:
    Bike(std::shared_ptr<const ObjData3D> _objData, 
        std::shared_ptr<World> _world, 
        std::shared_ptr<const Shader> _shader,
        std::shared_ptr<const Shader> _explodeShader,
        const glm::mat4 &modelMat,
        const glm::vec3 &_defaultColour = glm::vec3(0,0,0));
    ~Bike();

    void update(TurnDirection turning, Accelerating accelerating, bool stop);

    float getSpeedPercent() const;

    void toggleLightTrail() { trailManager.toggle(); }

    const LightTrailManager &getTrailManager() const { return trailManager; }

    bool checkSelfCollision() const { return trailManager.checkSelfCollision(); }

    void setExploding();

#ifdef DEBUG
    void saveBikeState();
    void restoreBikeState();
#endif

protected:
    // overrides as protected, as we shouldn't use these, we should use updateLocation() and turn()
    void translate(const glm::vec3 &vec) override { Object::translate(vec); }
    void rotate(float radians, const glm::vec3 &axis) override { Object::rotate(radians, axis); }
    void updateLocation();
    void turn(TurnDirection dir);
    Accelerating updateSpeed(Accelerating a);

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

    // note light trails are in world co-ords
    LightTrailManager trailManager;

    float speed;

    std::shared_ptr<const Shader> explodeShader;
    float explodeLevel;
    bool exploding;

#ifdef DEBUG
    bool bikeStateSaved;
    float savedBikeAngleAroundYRads;
    float savedSpeed;
    glm::mat4 savedModelMatrix;
#endif
};

#endif
