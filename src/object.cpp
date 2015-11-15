#include <object.hpp>

#include <world.hpp>
#include <shader.hpp>

Object::Object(std::shared_ptr<const ObjData> _objData, 
               std::shared_ptr<World> _world,
               std::shared_ptr<const Shader> _shader,
               const glm::mat4 &modelMat)
    : objData(_objData), 
      world(_world),
      shader(_shader),
      modelMatrix(modelMat), 
      defaultColour(glm::vec3(0.0f, 0.0f, 0.0f))
{
}

Object::~Object()
{
}

void Object::drawMesh(const Mesh &mesh) const
{
    GLuint vertexPosition_ModelID = shader->getAttribID(SHADER_ATTRIB_VECTOR_POS);
    GLuint vertexNormal_ModelID = shader->getAttribID(SHADER_ATTRIB_VECTOR_NORMAL);
    GLuint vertexTextureUVID = shader->getAttribID(SHADER_ATTRIB_VECTOR_UV);
    GLuint fragmentIsTextureID = shader->getUniformID(SHADER_UNIFORM_IS_TEXTURE);
    GLuint textureSamplerID = shader->getUniformID(SHADER_UNIFORM_TEXTURE_SAMPLER);

    glEnableVertexAttribArray(vertexPosition_ModelID);
    glEnableVertexAttribArray(vertexNormal_ModelID);

    if (mesh.hasTexture)
    {
        glEnableVertexAttribArray(vertexTextureUVID);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mesh.texture);
        glUniform1i(textureSamplerID, 0);
    }

    glBindBuffer(GL_ARRAY_BUFFER, mesh.vertexBuffer);
    glVertexAttribPointer(vertexPosition_ModelID, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

    if (mesh.hasTexture)
    {
        glBindBuffer(GL_ARRAY_BUFFER, mesh.uvBuffer);
        glVertexAttribPointer(vertexTextureUVID, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
        glUniform1f(fragmentIsTextureID, 1.0f);
    }
    else
    {
        glUniform3fv(shader->getUniformID(SHADER_UNIFORM_FRAGMENT_COLOUR),  1, &defaultColour[0]);
        glUniform1f(fragmentIsTextureID, 0.0f);
    }

    glBindBuffer(GL_ARRAY_BUFFER, mesh.normalBuffer);
    glVertexAttribPointer(vertexNormal_ModelID, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.indiceBuffer);
    glDrawElements(GL_TRIANGLES, mesh.numIndices, GL_UNSIGNED_SHORT, (void *)0);

    if (mesh.hasTexture)
    {
        glDisableVertexAttribArray(vertexTextureUVID);
    }

    glDisableVertexAttribArray(vertexPosition_ModelID);
    glDisableVertexAttribArray(vertexNormal_ModelID);
}

void Object::internalDrawAll(const std::vector<Mesh> &meshes) const
{
    for (auto &it : meshes)
    {
        drawMesh(it);
    }
}

void Object::drawAll() const
{
    const std::vector<Mesh> &meshes = objData->getMeshes();

    // bind shader
    shader->useShader();

    // for the generic object we only use one model matrix for the entire object
    // send MVP now
    world->sendMVP(shader, modelMatrix);

    // send lighting
    world->sendLight(shader);

    internalDrawAll(meshes);
}
