#ifndef OBJLOADER_H
#define OBJLOADER_H

#include <vector>
#include <cstring>

#include <glm/glm.hpp>

#include <GL/glew.h>

#include <object_data.hpp>

class ObjLoader : public ObjData3D
{
public:
    ObjLoader(const std::string &objPath, const std::string &_textureMapPath);
    ~ObjLoader();

    bool loadTextureMap();
    bool loadObj();

protected:
    std::string objPath;
    std::string textureMapPath;

    // mesh name -> texture name
    std::vector<std::pair<std::string, std::string>> textureMap;
};


#endif
