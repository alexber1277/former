#pragma once
#include <string>
#include "../gl/glincludes.h"

class Material {

public:

    uint32_t id = 0;
    std::string name;

    GLuint heightMap = 0;
    GLuint heightUniform = 0;

    Material() = default;
    Material(const std::string& _name);
    void SetHeightMap(GLuint tex);
    bool HasHeight() const;
    void CreateDefaultHeightMap(GLuint shader, int width, int height);

};