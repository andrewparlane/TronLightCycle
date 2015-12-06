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
};

enum ShaderAttribID
{
    SHADER_ATTRIB_VERTEX_POS = 0,
    SHADER_ATTRIB_VERTEX_POS_SCREEN,
    SHADER_ATTRIB_VERTEX_NORMAL,
    SHADER_ATTRIB_VERTEX_UV,
};

class Shader
{
public:
    Shader(const std::string &vertexShaderPath, const std::string &fragmentShaderPath);
    ~Shader();

    bool compile();

    bool addUniformID(const std::string &name, ShaderUniformID id);
    bool addAttribID(const std::string &name, ShaderAttribID id);

    GLuint getUniformID(ShaderUniformID id) const;
    GLuint getAttribID(ShaderAttribID id) const;

    void useShader() const;

protected:
    std::string vertexFilePath;
    std::string fragrmentFilePath;
    GLuint programID;

    // TODO: make this better.
    // I tried using unordered maps, but the performance impart was about 60 fps
    // around 4/5th of my original frame rate
    //std::unordered_map<ShaderUniformID, GLuint> uniformIDs;
    //std::unordered_map<ShaderAttribID, GLuint> attribIDs;
    
    GLuint shader_uniform_mvp;
    GLuint shader_uniform_model_matrix;
    GLuint shader_uniform_view_matrix;
    GLuint shader_uniform_light_pos_world;
    GLuint shader_uniform_light_colour;
    GLuint shader_uniform_light_power;
    GLuint shader_uniform_light_ambient_colour;
    GLuint shader_uniform_is_texture;
    GLuint shader_uniform_texture_sampler;
    GLuint shader_uniform_fragment_colour;
    
    
    GLuint shader_attrib_vertex_pos;
    GLuint shader_attrib_vertex_pos_screen;
    GLuint shader_attrib_vertex_normal;
    GLuint shader_attrib_vertex_uv;
};

#endif