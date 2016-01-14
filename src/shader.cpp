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
    for (unsigned int i = 0; i < SHADER_NUM_UNIFORM_IDS; i++)
    {
        uniformIDs[i] = -1;
    }
    for (unsigned int i = 0; i < SHADER_NUM_ATTRIB_IDS; i++)
    {
        attribIDs[i] = -1;
    }
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

    if (id >= SHADER_NUM_UNIFORM_IDS)
    {
        return false;
    }

    uniformIDs[id] = ret;

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

    if (id >= SHADER_NUM_ATTRIB_IDS)
    {
        return false;
    }

    attribIDs[id] = ret;

    return true;
}

GLuint Shader::getUniformID(ShaderUniformID id) const
{
    if (id >= SHADER_NUM_UNIFORM_IDS)
    {
        return -1;
    }

    return uniformIDs[id];
}

GLuint Shader::getAttribID(ShaderAttribID id) const
{
    if (id >= SHADER_NUM_ATTRIB_IDS)
    {
        return -1;
    }

    return attribIDs[id];
}

void Shader::useShader() const
{
    glUseProgram(programID);
}
