#include <shader.hpp>

#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>

Shader::Shader(const std::string &vertexShaderPath, 
              const std::string &fragmentShaderPath) 
    : vertexFilePath(vertexShaderPath), 
      fragrmentFilePath(fragmentShaderPath), 
      programID(0)
{
}

Shader::~Shader()
{
    GLuint textureSampler = getUniformID(SHADER_UNIFORM_TEXTURE_SAMPLER);
    if (textureSampler)
    {
        glDeleteTextures(1, &textureSampler);
    }

    if (programID != 0)
    {
        glDeleteProgram(programID);
    }
}

bool Shader::compile()
{
    // Create the shaders
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    // Read the Vertex Shader code from the file
    std::string VertexShaderCode;
    std::ifstream VertexShaderStream(vertexFilePath, std::ios::in);
    if(VertexShaderStream.is_open())
    {
        std::string Line = "";
        while(getline(VertexShaderStream, Line))
        {
            VertexShaderCode += "\n" + Line;
        }
        VertexShaderStream.close();
    }
    else
    {
        printf("Impossible to open %s.\n", vertexFilePath.c_str());
        return false;
    }

    // Read the Fragment Shader code from the file
    std::string FragmentShaderCode;
    std::ifstream FragmentShaderStream(fragrmentFilePath, std::ios::in);
    if(FragmentShaderStream.is_open())
    {
        std::string Line = "";
        while(getline(FragmentShaderStream, Line))
        {
            FragmentShaderCode += "\n" + Line;
        }
        FragmentShaderStream.close();
    }
    else
    {
        printf("Impossible to open %s.\n", fragrmentFilePath.c_str());
        return false;
    }

    GLint Result = GL_FALSE;
    int InfoLogLength;

    // Compile Vertex Shader
    printf("Compiling shader : %s\n", vertexFilePath.c_str());
    char const * VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
    glCompileShader(VertexShaderID);

    // Check Vertex Shader
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 )
    {
        std::vector<char> VertexShaderErrorMessage(InfoLogLength+1);
        glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
        printf("%s\n", &VertexShaderErrorMessage[0]);
    }

    // Compile Fragment Shader
    printf("Compiling shader : %s\n", fragrmentFilePath.c_str());
    char const * FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
    glCompileShader(FragmentShaderID);

    // Check Fragment Shader
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
        std::vector<char> FragmentShaderErrorMessage(InfoLogLength+1);
        glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
        printf("%s\n", &FragmentShaderErrorMessage[0]);
    }

    // Link the program
    printf("Linking program\n");
    programID = glCreateProgram();
    glAttachShader(programID, VertexShaderID);
    glAttachShader(programID, FragmentShaderID);
    glLinkProgram(programID);

    // Check the program
    glGetProgramiv(programID, GL_LINK_STATUS, &Result);
    glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
        std::vector<char> ProgramErrorMessage(InfoLogLength+1);
        glGetProgramInfoLog(programID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
        printf("%s\n", &ProgramErrorMessage[0]);
    }

    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);

    return true;
}

bool Shader::addUniformID(const std::string &name, ShaderUniformID id)
{
    GLuint ret = glGetUniformLocation(programID, name.c_str());
    if (ret == -1)
    {
        printf("failed to add ID %s\n", name.c_str());
        return false;
    }

    //uniformIDs.insert({ id, ret });
    
    switch (id)
    {
        case SHADER_UNIFORM_MVP:                    shader_uniform_mvp = ret; break;
        case SHADER_UNIFORM_MODEL_MATRIX:           shader_uniform_model_matrix = ret; break;
        case SHADER_UNIFORM_VIEW_MATRIX:            shader_uniform_view_matrix = ret; break;
        case SHADER_UNIFORM_LIGHT_POS_WORLD:        shader_uniform_light_pos_world = ret; break;
        case SHADER_UNIFORM_LIGHT_COLOUR:           shader_uniform_light_colour = ret; break;
        case SHADER_UNIFORM_LIGHT_POWER:            shader_uniform_light_power = ret; break;
        case SHADER_UNIFORM_LIGHT_AMBIENT_COLOUR:   shader_uniform_light_ambient_colour = ret; break;
        case SHADER_UNIFORM_IS_TEXTURE:             shader_uniform_is_texture = ret; break;
        case SHADER_UNIFORM_TEXTURE_SAMPLER:        shader_uniform_texture_sampler = ret; break;
        case SHADER_UNIFORM_FRAGMENT_COLOUR:        shader_uniform_fragment_colour = ret; break;
        default: return false;
    }
    

    return true;
}

bool Shader::addAttribID(const std::string &name, ShaderAttribID id)
{
    GLuint ret = glGetAttribLocation(programID, name.c_str());
    if (ret == -1)
    {
        printf("failed to add ID %s\n", name.c_str());
        return false;
    }

    //attribIDs.insert({ id, ret });
    
    switch (id)
    {
        case SHADER_ATTRIB_VECTOR_POS:          shader_attrib_vector_pos = ret; break;
        case SHADER_ATTRIB_VECTOR_POS_SCREEN:   shader_attrib_vector_pos_screen = ret; break;
        case SHADER_ATTRIB_VECTOR_NORMAL:       shader_attrib_vector_normal = ret; break;
        case SHADER_ATTRIB_VECTOR_UV:           shader_attrib_vector_uv = ret; break;
        default: return false;
    }

    return true;
}

GLuint Shader::getUniformID(ShaderUniformID id) const
{
/*     auto it = uniformIDs.find(id);
    if (it == uniformIDs.end())
    {
        return -1;
    }
    return it->second; */
    
    switch (id)
    {
        case SHADER_UNIFORM_MVP:                    return shader_uniform_mvp;
        case SHADER_UNIFORM_MODEL_MATRIX:           return shader_uniform_model_matrix;
        case SHADER_UNIFORM_VIEW_MATRIX:            return shader_uniform_view_matrix;
        case SHADER_UNIFORM_LIGHT_POS_WORLD:        return shader_uniform_light_pos_world;
        case SHADER_UNIFORM_LIGHT_COLOUR:           return shader_uniform_light_colour;
        case SHADER_UNIFORM_LIGHT_POWER:            return shader_uniform_light_power;
        case SHADER_UNIFORM_LIGHT_AMBIENT_COLOUR:   return shader_uniform_light_ambient_colour;
        case SHADER_UNIFORM_IS_TEXTURE:             return shader_uniform_is_texture;
        case SHADER_UNIFORM_TEXTURE_SAMPLER:        return shader_uniform_texture_sampler;
        case SHADER_UNIFORM_FRAGMENT_COLOUR:        return shader_uniform_fragment_colour;
        default: return -1;
    }
}

GLuint Shader::getAttribID(ShaderAttribID id) const
{
  /*   auto it = attribIDs.find(id);
    if (it == attribIDs.end())
    {
        return -1;
    }
    return it->second; */
    
    switch (id)
    {
        case SHADER_ATTRIB_VECTOR_POS:          return shader_attrib_vector_pos;
        case SHADER_ATTRIB_VECTOR_POS_SCREEN:   return shader_attrib_vector_pos_screen;
        case SHADER_ATTRIB_VECTOR_NORMAL:       return shader_attrib_vector_normal;
        case SHADER_ATTRIB_VECTOR_UV:           return shader_attrib_vector_uv;
        default: return -1;
    }
    
}

void Shader::useShader() const
{
    glUseProgram(programID);
}
