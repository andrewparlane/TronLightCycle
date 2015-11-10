#include <object.hpp>

Object::Object(std::shared_ptr<const ObjLoader> _objLoader, const glm::mat4 &mat)
    : objLoader(_objLoader), modelMatrix(mat), defaultColour(glm::vec3(0.0f, 0.0f, 0.0f))
{
}

Object::~Object()
{
}

void Object::drawMesh(const Mesh &mesh) const
{
    if (mesh.hasTexture)
    {
        glEnableVertexAttribArray(vertexTextureUVID);
    }

    /*glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1i(textureSamplerID, 0);*/

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
        glUniform3fv(fragmentColourID,  1, &defaultColour[0]);
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
}

void Object::drawAll() const
{
    const std::vector<Mesh> &meshes = objLoader->getMeshes();

    // for the generic object we only use one model matrix for the entire object
    // send MVP now

    // todo world singleton class?
    sendMVP(modelMatrix);
    // mvp matrix = model -> homogenous
    glm::mat4 mvp = projection * viewMatrix * modelMatrix;
    glUniformMatrix4fv(mvpID, 1, GL_FALSE, &mvp[0][0]);
    glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, &modelMatrix[0][0]);


    for (auto it = std::begin(meshes); it != std::end(meshes); it++)
    {
        drawMesh(*it);
    }
}
