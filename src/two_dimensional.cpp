#include <vector>
#include <cstring>

#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "shader.hpp"
#include "texture.hpp"

#include "two_dimensional.hpp"

Object2D::Object2D(std::shared_ptr<const Shader> _shader)
    : shader(_shader)
{
}

Object2D::~Object2D()
{
}

void Object2D::drawMesh(const std::shared_ptr<Mesh<glm::vec2>> &mesh) const
{
    GLuint vertexTextureUVID = shader->getAttribID(SHADER_ATTRIB_VERTEX_UV);
    GLuint vertexPosition_screenspaceID = shader->getAttribID(SHADER_ATTRIB_VERTEX_POS_SCREEN);
    GLuint vertexColourID = shader->getAttribID(SHADER_ATTRIB_VERTEX_COLOUR);
    GLuint textureSamplerID = shader->getUniformID(SHADER_UNIFORM_TEXTURE_SAMPLER);
    GLuint fragmentIsTextureID = shader->getUniformID(SHADER_UNIFORM_IS_TEXTURE);

    glEnableVertexAttribArray(vertexPosition_screenspaceID);

    if (mesh->hasTexture)
    {
        glEnableVertexAttribArray(vertexTextureUVID);
        glBindBuffer(GL_ARRAY_BUFFER, mesh->uvBuffer);
        glVertexAttribPointer(vertexTextureUVID, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);

        glUniform1f(fragmentIsTextureID, 1.0f);

        mesh->texture->bind(textureSamplerID);
    }
    else
    {
        glEnableVertexAttribArray(vertexColourID);
        glBindBuffer(GL_ARRAY_BUFFER, mesh->colourBuffer);
        glVertexAttribPointer(vertexColourID, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

        glUniform1f(fragmentIsTextureID, 0.0f);
    }

    glBindBuffer(GL_ARRAY_BUFFER, mesh->vertexBuffer);
    glVertexAttribPointer(vertexPosition_screenspaceID, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indiceBuffer);
    glDrawElements(GL_TRIANGLES, mesh->numIndices, GL_UNSIGNED_SHORT, (void *)0);

    if (mesh->hasTexture)
    {
        glDisableVertexAttribArray(vertexTextureUVID);
    }
    else
    {
        glDisableVertexAttribArray(vertexColourID);
    }

    glDisableVertexAttribArray(vertexPosition_screenspaceID);
}

void Object2D::drawAll() const
{
    const std::vector<std::shared_ptr<Mesh<glm::vec2>>> &meshes = objData.getMeshes();

    // bind shader
    shader->useShader();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    for (auto &it : meshes)
    {
        drawMesh(it);
    }
    glDisable(GL_BLEND);
}

Text::Text(std::shared_ptr<const Shader> _shader)
    : Object2D(_shader), numTextStrings(0)
{
}

Text::~Text()
{
}

void Text::addText2D(const std::string &text, int x, int y, int size, std::shared_ptr<Texture> texture)
{
    MeshData<glm::vec2> md;
    md.name = "text" + std::to_string(numTextStrings++);
    md.hasTexture = true;
    md.texture = texture;
    unsigned int i = 0;
    for (auto character : text)
    {
        glm::vec2 vertex_up_left    = glm::vec2( x+i*size     , y+size );
        glm::vec2 vertex_up_right   = glm::vec2( x+i*size+size, y+size );
        glm::vec2 vertex_down_right = glm::vec2( x+i*size+size, y      );
        glm::vec2 vertex_down_left  = glm::vec2( x+i*size     , y      );

        md.vertices.push_back(vertex_up_left   );
        md.vertices.push_back(vertex_down_left );
        md.vertices.push_back(vertex_up_right  );
        md.vertices.push_back(vertex_down_right);

        float uv_x = (character%16)/16.0f;
        float uv_y = (character/16)/16.0f;

        glm::vec2 uv_up_left    = glm::vec2( uv_x           , uv_y );
        glm::vec2 uv_up_right   = glm::vec2( uv_x+1.0f/16.0f, uv_y );
        glm::vec2 uv_down_right = glm::vec2( uv_x+1.0f/16.0f, (uv_y + 1.0f/16.0f) );
        glm::vec2 uv_down_left  = glm::vec2( uv_x           , (uv_y + 1.0f/16.0f) );
        md.uvs.push_back(uv_up_left);
        md.uvs.push_back(uv_down_left);
        md.uvs.push_back(uv_up_right);
        md.uvs.push_back(uv_down_right);

        md.indices.push_back((i*4)+0);
        md.indices.push_back((i*4)+2);
        md.indices.push_back((i*4)+1);

        md.indices.push_back((i*4)+1);
        md.indices.push_back((i*4)+2);
        md.indices.push_back((i*4)+3);

        i++;
    }

    objData.addMesh(md);
}

Shape2D::Shape2D(std::shared_ptr<const Shader> _shader)
    : Object2D(_shader), numRects(0)
{
}

Shape2D::~Shape2D()
{
}

void Shape2D::addLine(glm::vec2 start, glm::vec2 end, glm::vec3 startColour, glm::vec3 endColour, float thickness)
{
    // a line is just a rectangle.
    // calculate top left and bottom right co-ords

    // we have a vector that defines the long edges,
    // to get the short edges, rotate by 90 degrees, normalize and * thickness
    // then / 2, as the start and end are the middle of the other edges
    glm::vec2 direction = end - start;
    direction.y = -direction.y;
    glm::vec2 otherEdge = glm::normalize(glm::vec2(direction.y, direction.x)) * (thickness / 2.0f);

    addRect(start + otherEdge, end + otherEdge, end - otherEdge, start - otherEdge,
            startColour, endColour, endColour, startColour);
}

void Shape2D::addRect(glm::vec2 tl, glm::vec2 tr, glm::vec2 br, glm::vec2 bl, glm::vec3 tlCol, glm::vec3 trCol, glm::vec3 brCol, glm::vec3 blCol, const std::string &name)
{
    MeshData<glm::vec2> md;
    if (name.size())
    {
        md.name = name;
    }
    else
    {
        md.name = "rect" + std::to_string(numRects++);
    }
    md.hasTexture = false;

    md.vertices.push_back(tl);
    md.vertices.push_back(tr);
    md.vertices.push_back(br);
    md.vertices.push_back(bl);

    md.colours.push_back(tlCol);
    md.colours.push_back(trCol);
    md.colours.push_back(brCol);
    md.colours.push_back(blCol);

    md.indices.push_back(0);
    md.indices.push_back(1);
    md.indices.push_back(3);

    md.indices.push_back(1);
    md.indices.push_back(2);
    md.indices.push_back(3);

    objData.addMesh(md);
}
