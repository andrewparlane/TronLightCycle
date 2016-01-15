#ifndef __SHADER_HPP
#define __SHADER_HPP

#include <GL/glew.h>

#include <string>
#include <unordered_map>

enum ShaderUniformID
{
    SHADER_UNIFORM_MVP = 0,
    SHADER_UNIFORM_MODEL_MATRIX,
    SHADER_UNIFORM_VIEW_MATRIX,

    SHADER_UNIFORM_LIGHT_POS_WORLD,
    SHADER_UNIFORM_LIGHT_COLOUR,
    SHADER_UNIFORM_LIGHT_POWER,
    SHADER_UNIFORM_LIGHT_AMBIENT_COLOUR,

    SHADER_UNIFORM_IS_TEXTURE,
    SHADER_UNIFORM_TEXTURE_SAMPLER,
    SHADER_UNIFORM_FRAGMENT_COLOUR,

    SHADER_NUM_UNIFORM_IDS
};

enum ShaderAttribID
{
    SHADER_ATTRIB_VERTEX_POS = 0,
    SHADER_ATTRIB_VERTEX_POS_SCREEN,
    SHADER_ATTRIB_VERTEX_NORMAL,
    SHADER_ATTRIB_VERTEX_UV,
    SHADER_ATTRIB_VERTEX_COLOUR,

    SHADER_NUM_ATTRIB_IDS
};

class Shader
{
public:
    Shader(const std::string &vertexShaderPath, const std::string &geometryShaderPath, const std::string *fragmentShaderPath = NULL);
    ~Shader();

    bool compile();

    bool addUniformID(const std::string &name, ShaderUniformID id);
    bool addAttribID(const std::string &name, ShaderAttribID id);

    GLuint getUniformID(ShaderUniformID id) const;
    GLuint getAttribID(ShaderAttribID id) const;

    void useShader() const;

protected:
    bool compileShader(const std::string &path, GLuint &shaderID) const;

    const std::string vertexFilePath;
    const std::string fragmentFilePath;
    const std::string *geometryFilePath;
    GLuint programID;

    GLuint uniformIDs[SHADER_NUM_UNIFORM_IDS];
    GLuint attribIDs[SHADER_NUM_ATTRIB_IDS];
};

#endif
