#ifndef __SHADER_HPP
#define __SHADER_HPP

#include <GL/glew.h>

#include <string>
#include <memory>

enum ShaderType
{
    SHADER_TYPE_MAIN_GEOMETRY_PASS = 0,
    SHADER_TYPE_EXPLODE_GEOMETRY_PASS,
    SHADER_TYPE_LIGHTING_PASS,
    SHADER_TYPE_HDR_PASS,
    SHADER_TYPE_LAMP,
    SHADER_TYPE_BLUR,
    SHADER_TYPE_2D,

    NUM_SHADER_TYPES
};

enum ShaderUniformID
{
    SHARDER_UNIFORM_SCREEN_RES = 0,

    SHADER_UNIFORM_MVP,
    SHADER_UNIFORM_MODEL_VIEW_MATRIX,
    SHADER_UNIFORM_NORMAL_MODEL_VIEW_MATRIX,
    SHADER_UNIFORM_INVERSE_PROJECTION_MATRIX,

    SHADER_UNIFORM_LIGHT_POS_CAMERA,
    SHADER_UNIFORM_LIGHT_COLOUR,
    SHADER_UNIFORM_LIGHT_RADIUS,
    SHADER_UNIFORM_LIGHT_AMBIENT_FACTOR,
    SHADER_UNIFORM_LIGHT_DIFFUSE_FACTOR,
    SHADER_UNIFORM_LIGHT_SPECULAR_FACTOR,

    SHADER_UNIFORM_IS_TEXTURE,
    SHADER_UNIFORM_TEXTURE_SAMPLER,
    SHADER_UNIFORM_FRAGMENT_COLOUR,

    SHADER_UNIFORM_GEOMETRY_TEXTURE_SAMPLER,
    SHADER_UNIFORM_NORMAL_TEXTURE_SAMPLER,
    SHADER_UNIFORM_COLOUR_TEXTURE_SAMPLER,

    SHADER_UNIFORM_HORIZONTAL_FLAG,
    SHADER_UNIFORM_EXPLODE,

    SHADER_NUM_UNIFORM_IDS
};

enum ShaderAttribID
{
    SHADER_ATTRIB_VERTEX_POS = 0,
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

    static bool setupShaders();
    static std::shared_ptr<const Shader> getShader(ShaderType type);

protected:
    bool compileShader(const std::string &path, GLuint &shaderID) const;

    static std::shared_ptr<Shader> setupMainGeometryPassShader(const std::string *geometryShader = NULL);
    static std::shared_ptr<Shader> setupExplodeShader();
    static std::shared_ptr<Shader> setupLightingPassShader();
    static std::shared_ptr<Shader> setupHDRShader();
    static std::shared_ptr<Shader> setupLampShader();
    static std::shared_ptr<Shader> setupBlurShader();
    static std::shared_ptr<Shader> setup2DShader();

    static std::shared_ptr<const Shader> shaders[NUM_SHADER_TYPES];

    const std::string vertexFilePath;
    const std::string fragmentFilePath;
    const std::string *geometryFilePath;
    GLuint programID;

    GLuint uniformIDs[SHADER_NUM_UNIFORM_IDS];
    GLuint attribIDs[SHADER_NUM_ATTRIB_IDS];
};

#endif
