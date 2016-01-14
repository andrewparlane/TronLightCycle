#ifndef __TEXTURE_HPP
#define __TEXTURE_HPP

#include <string>

#include <GL/glew.h>

class Texture
{
public:
    Texture(const std::string &imagepath);
    ~Texture();

    bool loadDDS();
    void bind(GLuint textureSamplerID) const;

protected:
    std::string path;
    GLuint textureID;
};

#endif
