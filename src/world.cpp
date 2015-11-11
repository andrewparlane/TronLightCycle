#include <world.hpp>

#include <string.h>

#include <GL/glew.h>

World::World() : lightPower(0)
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

void World::sendLight(std::shared_ptr<const Shader> shader) const
{
    glUniform3fv(shader->getUniformID(SHADER_UNIFORM_LIGHT_POS_WORLD), 1, &lightPos[0]);
    glUniform3fv(shader->getUniformID(SHADER_UNIFORM_LIGHT_COLOUR), 1, &lightColour[0]);
    glUniform1f(shader->getUniformID(SHADER_UNIFORM_LIGHT_POWER), lightPower);
}
