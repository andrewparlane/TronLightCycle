#include "lamp.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

Lamp::Lamp(std::shared_ptr<const ObjData3D> _objData,
           std::shared_ptr<const Shader> _shader,
           const glm::mat4 &modelMatWithoutTransform,
           const glm::vec3 &_position, const glm::vec3 &_colour,
           float _ambient, float _diffuse, float _specular)
    : objData(_objData), shader(_shader),
      modelMatrix(glm::translate(_position) * modelMatWithoutTransform),
      colour(_colour), position(_position),
      ambient(_ambient), diffuse(_diffuse), specular(_specular)
{
}

Lamp::~Lamp()
{
}

void Lamp::draw(const glm::mat4 &projectionMatrix, const glm::mat4 &viewMatrix) const
{
    const std::vector<std::shared_ptr<Mesh<glm::vec3>>> &meshes = objData->getMeshes();

    // bind shader
    shader->useShader();

    // send MVP
    glm::mat4 mvp = projectionMatrix * viewMatrix * modelMatrix;
    glUniformMatrix4fv(shader->getUniformID(SHADER_UNIFORM_MVP), 1, GL_FALSE, &mvp[0][0]);

    for (auto &it : meshes)
    {
        GLuint vertexPosition_ModelID = shader->getAttribID(SHADER_ATTRIB_VERTEX_POS);

        glEnableVertexAttribArray(vertexPosition_ModelID);
        glBindBuffer(GL_ARRAY_BUFFER, it->vertexBuffer);
        glVertexAttribPointer(vertexPosition_ModelID, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

        glUniform3fv(shader->getUniformID(SHADER_UNIFORM_FRAGMENT_COLOUR),  1, &colour[0]);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, it->indiceBuffer);
        glDrawElements(GL_TRIANGLES, it->numIndices, GL_UNSIGNED_SHORT, (void *)0);

        glDisableVertexAttribArray(vertexPosition_ModelID);
    }
}

void Lamp::sendLamp(std::shared_ptr<const Shader> shader) const
{
    glUniform3fv(shader->getUniformID(SHADER_UNIFORM_LIGHT_POS_WORLD), 1, &position[0]);
    glUniform3fv(shader->getUniformID(SHADER_UNIFORM_LIGHT_COLOUR), 1, &colour[0]);
    glUniform1f(shader->getUniformID(SHADER_UNIFORM_LIGHT_AMBIENT_FACTOR), ambient);
    glUniform1f(shader->getUniformID(SHADER_UNIFORM_LIGHT_DIFFUSE_FACTOR), diffuse);
    glUniform1f(shader->getUniformID(SHADER_UNIFORM_LIGHT_SPECULAR_FACTOR), specular);
}