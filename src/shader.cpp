#include <shader.hpp>

#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>

Shader::Shader(const std::string &vertexShaderPath,
               const std::string &fragmentShaderPath,
               const std::string *geometryShaderPath)
    : vertexFilePath(vertexShaderPath),
      geometryFilePath(geometryShaderPath),
      fragmentFilePath(fragmentShaderPath),
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

bool Shader::compileShader(const std::string &path, GLuint &shaderID) const
{
    GLint Result = GL_FALSE;
    int InfoLogLength;

    // Read the Shader code from the file
    std::string shaderCode;
    std::ifstream shaderStream(path, std::ios::in);
    if(shaderStream.is_open())
    {
        std::string Line = "";
        while(getline(shaderStream, Line))
        {
            shaderCode += "\n" + Line;
        }
        shaderStream.close();
    }
    else
    {
        printf("Impossible to open %s.\n", path.c_str());
        return false;
    }

    // Compile  Shader
    printf("Compiling shader : %s\n", path.c_str());
    char const * sourcePointer = shaderCode.c_str();
    glShaderSource(shaderID, 1, &sourcePointer , NULL);
    glCompileShader(shaderID);

    // Check Shader
    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 )
    {
        std::vector<char> errorMessage(InfoLogLength+1);
        glGetShaderInfoLog(shaderID, InfoLogLength, NULL, &errorMessage[0]);
        printf("%s\n", &errorMessage[0]);
    }

    glAttachShader(programID, shaderID);

    return true;
}

bool Shader::compile()
{
    // Create the shaders
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    programID = glCreateProgram();

    compileShader(vertexFilePath, VertexShaderID);
    compileShader(fragmentFilePath, FragmentShaderID);

    if (geometryFilePath)
    {
        GLuint GeometryShaderID = glCreateShader(GL_GEOMETRY_SHADER);
        compileShader(*geometryFilePath, GeometryShaderID);
    }

    // Link the program
    printf("Linking program\n");
    glLinkProgram(programID);

    // Check the program
    GLint Result = GL_FALSE;
    int InfoLogLength;
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
