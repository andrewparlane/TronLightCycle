#include <vector>
#include <cstring>

#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "shader.hpp"
#include "opengl_tutorials_org/texture.hpp"

#include "two_dimensional.hpp"

Object2D::Object2D(std::shared_ptr<const Shader> _shader, glm::vec3 _defaultColour)
    : shader(_shader), defaultColour(_defaultColour)
{
}

Object2D::~Object2D()
{
}

void Object2D::drawMesh(const std::shared_ptr<Mesh<glm::vec2>> &mesh) const
{
    GLuint vertexTextureUVID = shader->getAttribID(SHADER_ATTRIB_VECTOR_UV);
    GLuint vertexPosition_screenspaceID = shader->getAttribID(SHADER_ATTRIB_VECTOR_POS_SCREEN);
    GLuint textureSamplerID = shader->getUniformID(SHADER_UNIFORM_TEXTURE_SAMPLER);
    GLuint fragmentIsTextureID = shader->getUniformID(SHADER_UNIFORM_IS_TEXTURE);

    glEnableVertexAttribArray(vertexPosition_screenspaceID);

    if (mesh->hasTexture)
    {
        glEnableVertexAttribArray(vertexTextureUVID);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mesh->texture);
        glUniform1i(textureSamplerID, 0);
    }

    glBindBuffer(GL_ARRAY_BUFFER, mesh->vertexBuffer);
    glVertexAttribPointer(vertexPosition_screenspaceID, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);

    if (mesh->hasTexture)
    {
        glBindBuffer(GL_ARRAY_BUFFER, mesh->uvBuffer);
        glVertexAttribPointer(vertexTextureUVID, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
        glUniform1f(fragmentIsTextureID, 1.0f);
    }
    else
    {
        glUniform3fv(shader->getUniformID(SHADER_UNIFORM_FRAGMENT_COLOUR),  1, &defaultColour[0]);
        glUniform1f(fragmentIsTextureID, 0.0f);
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indiceBuffer);
    glDrawElements(GL_TRIANGLES, mesh->numIndices, GL_UNSIGNED_SHORT, (void *)0);

    if (mesh->hasTexture)
    {
        glDisableVertexAttribArray(vertexTextureUVID);
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

void Text::addText2D(const char * text, int x, int y, int size, unsigned int textureID)
{
    unsigned int length = strlen(text);

    MeshData<glm::vec2> md;
    md.name = "text" + std::to_string(numTextStrings++);
    md.hasTexture = true;
    md.textureID = textureID;
    for ( unsigned int i=0 ; i<length ; i++ )
    {
        glm::vec2 vertex_up_left    = glm::vec2( x+i*size     , y+size );
        glm::vec2 vertex_up_right   = glm::vec2( x+i*size+size, y+size );
        glm::vec2 vertex_down_right = glm::vec2( x+i*size+size, y      );
        glm::vec2 vertex_down_left  = glm::vec2( x+i*size     , y      );

        md.vertices.push_back(vertex_up_left   );
        md.vertices.push_back(vertex_down_left );
        md.vertices.push_back(vertex_up_right  );
        md.vertices.push_back(vertex_down_right);

        char character = text[i];
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
    }

    objData.addMesh(md);
}

Shape2D::Shape2D(std::shared_ptr<const Shader> _shader, glm::vec3 _defaultColour)
    : Object2D(_shader, _defaultColour), numRects(0)
{
}

Shape2D::~Shape2D()
{
}

void Shape2D::addLine(glm::vec2 start, glm::vec2 end, float thickness)
{
    // a line is just a rectangle.
    // calculate top left and bottom right co-ords

    // we have a vector that defines the long edges,
    // to get the short edges, rotate by 90 degrees, normalize and * thickness
    // then / 2, as the start and end are the middle of the other edges
    glm::vec2 direction = end - start;
    direction.y = -direction.y;
    glm::vec2 otherEdge = glm::normalize(glm::vec2(direction.y, direction.x)) * (thickness / 2.0f);

    addRect(start + otherEdge,
            end + otherEdge,
            end - otherEdge,
            start - otherEdge);
}

void Shape2D::addRect(glm::vec2 tl, glm::vec2 tr, glm::vec2 br, glm::vec2 bl)
{
    MeshData<glm::vec2> md;
    md.name = "rect" + std::to_string(numRects++);
    md.hasTexture = false;

    md.vertices.push_back(tl);
    md.vertices.push_back(tr);
    md.vertices.push_back(br);
    md.vertices.push_back(bl);

    md.indices.push_back(0);
    md.indices.push_back(1);
    md.indices.push_back(3);

    md.indices.push_back(1);
    md.indices.push_back(2);
    md.indices.push_back(3);

    objData.addMesh(md);
}
