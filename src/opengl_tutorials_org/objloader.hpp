#ifndef OBJLOADER_H
#define OBJLOADER_H

struct Mesh
{
    std::string name;
    std::vector<unsigned short> indices;
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> normals;
    bool hasTexture;
};

bool loadAssImp(const char * path, std::vector<Mesh> &meshes);

#endif
