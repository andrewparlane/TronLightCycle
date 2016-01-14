#ifndef __TEXTURE_HPP
#define __TEXTURE_HPP

#include <string>
#include <memory>
#include <map>

#include <GL/glew.h>

class Texture
{
public:
    ~Texture();

    static std::shared_ptr<Texture> getOrCreate(const std::string &imagepath);

    void bind(GLuint textureSamplerID) const;

protected:
    Texture(const std::string &imagepath);
    bool loadDDS();

    std::string path;
    GLuint textureID;

    static std::map<std::string, std::shared_ptr<Texture>> textureCache;
};

#endif
