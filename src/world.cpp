#include <world.hpp>

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
    glm::mat4 mvp = projectionMatrix * viewMatrix * model;
    glUniformMatrix4fv(shader->getUniformID(SHADER_UNIFORM_MVP), 1, GL_FALSE, &mvp[0][0]);

    // also send model and view matrix for lightinng stuff
    glUniformMatrix4fv(shader->getUniformID(SHADER_UNIFORM_MODEL_MATRIX), 1, GL_FALSE, &model[0][0]);
    glUniformMatrix4fv(shader->getUniformID(SHADER_UNIFORM_VIEW_MATRIX), 1, GL_FALSE, &viewMatrix[0][0]);
}

bool World::addLamp(std::shared_ptr<const ObjData3D> objData, std::shared_ptr<const Shader> shader,
    const glm::mat4 &modelMatWithoutTransform, const glm::vec3 &position,
    float radius, const glm::vec3 &colour, float ambient, float diffuse, float specular)
{
    if (lamps.size() >= MAX_NUM_LAMPS)
    {
        return false;
    }
    lamps.push_back(std::make_unique<Lamp>(objData, shader, modelMatWithoutTransform, position, radius, colour, ambient, diffuse, specular));
    return true;
}

void World::sendLightingInfoToShader(std::shared_ptr<const Shader> shader) const
{
    glm::vec3 positionBuffer[MAX_NUM_LAMPS];
    float radiusBuffer[MAX_NUM_LAMPS];
    glm::vec3 colourBuffer[MAX_NUM_LAMPS];
    float ambientBuffer[MAX_NUM_LAMPS];
    float diffuseBuffer[MAX_NUM_LAMPS];
    float specularBuffer[MAX_NUM_LAMPS];

    unsigned int numLamps = 0;

    for (const auto &it : lamps)
    {
        it->getLampData(positionBuffer[numLamps], radiusBuffer[numLamps], colourBuffer[numLamps], ambientBuffer[numLamps], diffuseBuffer[numLamps], specularBuffer[numLamps]);
        numLamps++;
    }

    glUniform3fv(shader->getUniformID(SHADER_UNIFORM_LIGHT_POS_WORLD), MAX_NUM_LAMPS, &positionBuffer[0][0]);
    glUniform1fv(shader->getUniformID(SHADER_UNIFORM_LIGHT_RADIUS), MAX_NUM_LAMPS, radiusBuffer);
    glUniform3fv(shader->getUniformID(SHADER_UNIFORM_LIGHT_COLOUR), MAX_NUM_LAMPS, &colourBuffer[0][0]);
    glUniform1fv(shader->getUniformID(SHADER_UNIFORM_LIGHT_AMBIENT_FACTOR), MAX_NUM_LAMPS, ambientBuffer);
    glUniform1fv(shader->getUniformID(SHADER_UNIFORM_LIGHT_DIFFUSE_FACTOR), MAX_NUM_LAMPS, diffuseBuffer);
    glUniform1fv(shader->getUniformID(SHADER_UNIFORM_LIGHT_SPECULAR_FACTOR), MAX_NUM_LAMPS, specularBuffer);

    glUniform1i(shader->getUniformID(SHADER_UNIFORM_NUM_LIGHTS), numLamps);
}
