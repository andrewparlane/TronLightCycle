#include <vector>
#include <cstring>

#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "shader.hpp"
#include "opengl_tutorials_org/texture.hpp"

#include "two_dimensional.hpp"

TwoDimensional::TwoDimensional(std::shared_ptr<const Shader> _shader)
    : shader(_shader), numTextStrings(0)
{
}

TwoDimensional::~TwoDimensional()
{
}

void TwoDimensional::addText2D(const char * text, int x, int y, int size, unsigned int textureID)
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

void TwoDimensional::drawMesh(const std::shared_ptr<Mesh<glm::vec2>> &mesh) const
{
    GLuint vertexTextureUVID = shader->getAttribID(SHADER_ATTRIB_VECTOR_UV);
    GLuint vertexPosition_screenspaceID = shader->getAttribID(SHADER_ATTRIB_VECTOR_POS_SCREEN);
    GLuint textureSamplerID = shader->getUniformID(SHADER_UNIFORM_TEXTURE_SAMPLER);

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
        //glUniform1f(fragmentIsTextureID, 1.0f);
    }
    else
    {
        //glUniform3fv(shader->getUniformID(SHADER_UNIFORM_FRAGMENT_COLOUR),  1, &defaultColour[0]);
        //glUniform1f(fragmentIsTextureID, 0.0f);
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indiceBuffer);
    glDrawElements(GL_TRIANGLES, mesh->numIndices, GL_UNSIGNED_SHORT, (void *)0);

    if (mesh->hasTexture)
    {
        glDisableVertexAttribArray(vertexTextureUVID);
    }

    glDisableVertexAttribArray(vertexPosition_screenspaceID);
}

void TwoDimensional::drawAll() const
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
