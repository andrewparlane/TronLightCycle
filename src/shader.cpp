#include "shader.hpp"

#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>

std::shared_ptr<const Shader> Shader::shaders[NUM_SHADER_TYPES];

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

bool Shader::setupShaders()
{
    shaders[SHADER_TYPE_MAIN_GEOMETRY_PASS]     = setupMainGeometryPassShader();
    shaders[SHADER_TYPE_EXPLODE_GEOMETRY_PASS]  = setupExplodeShader();
    shaders[SHADER_TYPE_LIGHTING_PASS]          = setupLightingPassShader();
    shaders[SHADER_TYPE_HDR_PASS]               = setupHDRShader();
    shaders[SHADER_TYPE_LAMP]                   = setupLampShader();
    shaders[SHADER_TYPE_2D]                     = setup2DShader();

    for (unsigned int i = 0; i < NUM_SHADER_TYPES; i++)
    {
        if (!shaders[i])
        {
            return false;
        }
    }

    return true;
}

std::shared_ptr<const Shader> Shader::getShader(ShaderType type)
{
    if (type >= NUM_SHADER_TYPES)
    {
        return NULL;
    }

    return shaders[type];
}

std::shared_ptr<Shader> Shader::setupMainGeometryPassShader(const std::string *geometryShader)
{
    // first init main shader
    std::shared_ptr<Shader> shader = std::make_shared<Shader>("shaders/main_geometry_pass.vs", "shaders/main_geometry_pass.fs", geometryShader);
    if (!shader || !shader->compile())
    {
        printf("Failed to compile main geometry pass shader\n");
        shader = NULL;
    }
    else
    {
        // Get main shader parameters
        if (// vertex params (variable)
            !shader->addAttribID("vertexPosition_Model", SHADER_ATTRIB_VERTEX_POS) ||
            !shader->addAttribID("vertexNormal_Model", SHADER_ATTRIB_VERTEX_NORMAL) ||
            !shader->addAttribID("vertexTextureUV", SHADER_ATTRIB_VERTEX_UV) ||
            // vertex params (static)
            !shader->addUniformID("MVP", SHADER_UNIFORM_MVP) ||
            !shader->addUniformID("MV", SHADER_UNIFORM_MODEL_VIEW_MATRIX) ||
            !shader->addUniformID("normalMV", SHADER_UNIFORM_NORMAL_MODEL_VIEW_MATRIX) ||
            // fragment params
            !shader->addUniformID("fragmentIsTexture", SHADER_UNIFORM_IS_TEXTURE) ||
            !shader->addUniformID("textureSampler", SHADER_UNIFORM_TEXTURE_SAMPLER) ||
            !shader->addUniformID("fragmentColour", SHADER_UNIFORM_FRAGMENT_COLOUR))
        {
            printf("Error adding shader IDs\n");
            shader = NULL;
        }
    }

    return shader;
}

std::shared_ptr<Shader> Shader::setupExplodeShader()
{
    // first init main shader
    std::string gsPath = "shaders/explode_geometry_pass.gs";
    std::shared_ptr<Shader> shader = setupMainGeometryPassShader(&gsPath);
    if (shader)
    {
        if (!shader->addUniformID("explode", SHADER_UNIFORM_EXPLODE) ||
            !shader->addUniformID("inverseProjectionMatrix", SHADER_UNIFORM_INVERSE_PROJECTION_MATRIX))
        {
            printf("Error adding explode shader IDs\n");
            shader = NULL;
        }
    }

    return shader;
}

std::shared_ptr<Shader> Shader::setupLightingPassShader()
{
    // first init main shader
    std::shared_ptr<Shader> shader = std::make_shared<Shader>("shaders/main_lighting_pass.vs", "shaders/main_lighting_pass.fs");
    if (!shader || !shader->compile())
    {
        printf("Failed to compile main lighting pass shader\n");
        shader = NULL;
    }
    else
    {
        // Get main shader parameters
        if (// vertex params (variable)
            !shader->addAttribID("vertexPosition_Model", SHADER_ATTRIB_VERTEX_POS) ||
            // vertex params (static)
            !shader->addUniformID("MVP", SHADER_UNIFORM_MVP) ||
            // fragment params
            !shader->addUniformID("screenResolution", SHARDER_UNIFORM_SCREEN_RES) ||
            !shader->addUniformID("lightPosition_Camera", SHADER_UNIFORM_LIGHT_POS_CAMERA) ||
            !shader->addUniformID("lightRadius", SHADER_UNIFORM_LIGHT_RADIUS) ||
            !shader->addUniformID("lightColour", SHADER_UNIFORM_LIGHT_COLOUR) ||
            !shader->addUniformID("lightAmbient", SHADER_UNIFORM_LIGHT_AMBIENT_FACTOR) ||
            !shader->addUniformID("lightDiffuse", SHADER_UNIFORM_LIGHT_DIFFUSE_FACTOR) ||
            !shader->addUniformID("lightSpecular", SHADER_UNIFORM_LIGHT_SPECULAR_FACTOR) ||
            !shader->addUniformID("geometryTextureSampler", SHADER_UNIFORM_GEOMETRY_TEXTURE_SAMPLER) ||
            !shader->addUniformID("normalTextureSampler", SHADER_UNIFORM_NORMAL_TEXTURE_SAMPLER) ||
            !shader->addUniformID("colourTextureSampler", SHADER_UNIFORM_COLOUR_TEXTURE_SAMPLER))
        {
            printf("Error adding shader IDs\n");
            shader = NULL;
        }
    }

    return shader;
}

std::shared_ptr<Shader> Shader::setupHDRShader()
{
    // first init main shader
    std::shared_ptr<Shader> shader = std::make_shared<Shader>("shaders/hdr.vs", "shaders/hdr.fs");
    if (!shader || !shader->compile())
    {
        printf("Failed to compile HDR pass shader\n");
        shader = NULL;
    }
    else
    {
        // Get main shader parameters
        if (// vertex params (variable)
            !shader->addAttribID("vertexPosition_Screen", SHADER_ATTRIB_VERTEX_POS) ||
            // fragment params
            !shader->addUniformID("screenResolution", SHARDER_UNIFORM_SCREEN_RES) ||
            !shader->addUniformID("colourTextureSampler", SHADER_UNIFORM_COLOUR_TEXTURE_SAMPLER))
        {
            printf("Error adding shader IDs\n");
            shader = NULL;
        }
    }

    return shader;
}

std::shared_ptr<Shader> Shader::setupLampShader()
{
    // first init main shader
    std::shared_ptr<Shader> shader = std::make_shared<Shader>("shaders/lamp.vs", "shaders/lamp.fs");
    if (!shader || !shader->compile())
    {
        printf("Failed to compile lamp shader\n");
        shader = NULL;
    }
    else
    {
        // Get main shader parameters
        if (// vertex params (variable)
            !shader->addAttribID("vertexPosition_Model", SHADER_ATTRIB_VERTEX_POS) ||
            // vertex params (static)
            !shader->addUniformID("MVP", SHADER_UNIFORM_MVP) ||
            // fragment params
            !shader->addUniformID("fragmentColour", SHADER_UNIFORM_FRAGMENT_COLOUR))
        {
            printf("Error adding shader IDs\n");
            shader = NULL;
        }
    }

    return shader;
}

std::shared_ptr<Shader> Shader::setup2DShader()
{
    // initialise text shader
    std::shared_ptr<Shader> shader = std::make_shared<Shader>("shaders/2D.vs", "shaders/2D.fs");
    if (!shader || !shader->compile())
    {
        printf("Failed to compile 2D shader\n");
        shader = NULL;
    }
    else
    {
        // Get a handle for our buffers
        if (!shader->addAttribID("vertexPosition_screenspace", SHADER_ATTRIB_VERTEX_POS) ||
            !shader->addAttribID("vertexUV", SHADER_ATTRIB_VERTEX_UV) ||
            !shader->addAttribID("vertexColour", SHADER_ATTRIB_VERTEX_COLOUR) ||
            !shader->addUniformID("myTextureSampler", SHADER_UNIFORM_TEXTURE_SAMPLER) ||
            !shader->addUniformID("fragmentIsTexture", SHADER_UNIFORM_IS_TEXTURE))
        {
            printf("Failed to add 2D shader IDs\n");
            shader = NULL;
        }
    }

    return shader;
}
