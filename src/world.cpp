#include "world.hpp"
#include "shader.hpp"
#include "lamp.hpp"

#include <string.h>

#include <GL/glew.h>

World::World()
{
}

World::~World()
{
}

void World::sendMVP(std::shared_ptr<const Shader> shader, const glm::mat4 &model) const
{
    // mvp matrix = model -> homogenous
    glm::mat4 mv = viewMatrix * model;
    glm::mat4 mvp = projectionMatrix * mv;
    glm::mat3 normalMV = glm::mat3(glm::transpose(glm::inverse(mv)));

    glUniformMatrix4fv(shader->getUniformID(SHADER_UNIFORM_MVP), 1, GL_FALSE, &mvp[0][0]);

    // also send model view matrix for lightinng stuff
    glUniformMatrix4fv(shader->getUniformID(SHADER_UNIFORM_MODEL_VIEW_MATRIX), 1, GL_FALSE, &mv[0][0]);

    // and normalMV
    glUniformMatrix3fv(shader->getUniformID(SHADER_UNIFORM_NORMAL_MODEL_VIEW_MATRIX), 1, GL_FALSE, &normalMV[0][0]);

    // and the inverse of the projection matrix (if needed)
    GLuint inversePID = shader->getUniformID(SHADER_UNIFORM_NORMAL_MODEL_VIEW_MATRIX);
    if (inversePID >= 0)
    {
        glUniformMatrix4fv(inversePID, 1, GL_FALSE, &inverseProjectionMatrix[0][0]);
    }
}

bool World::addLamp(std::shared_ptr<const ObjData3D> objData, std::shared_ptr<const ObjData2D> deferredShadingObj, std::shared_ptr<const Shader> shader,
    const glm::mat4 &modelMatWithoutTransform, const glm::vec3 &position,
    float radius, const glm::vec3 &colour, float ambient, float diffuse, float specular)
{
    if (lamps.size() >= MAX_NUM_LAMPS)
    {
        return false;
    }
    lamps.push_back(std::make_unique<Lamp>(objData, deferredShadingObj, shader, modelMatWithoutTransform, position, radius, colour, ambient, diffuse, specular));
    return true;
}

void World::sendLightingInfoToShader(std::shared_ptr<const Shader> shader) const
{
    // Enable blending
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_ONE, GL_ONE);
    for (const auto &it : lamps)
    {
        it->sendLampData(shader, viewMatrix);
    }
    glDisable(GL_BLEND);
}

void World::drawLamps() const
{
    std::for_each(lamps.begin(), lamps.end(),
                  [this](const std::unique_ptr<Lamp> &lamp)
    {
        lamp->draw(projectionMatrix, viewMatrix);
    });
}
