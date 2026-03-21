#include "material.h"
#include <vector>

void Material::SetHeightMap(GLuint tex) {
    heightMap = tex;
}

bool Material::HasHeight() const {
    return heightMap != 0;
}

Material::Material(const std::string& _name) {
    id = 0;
    name = _name;
}

void Material::CreateDefaultHeightMap(GLuint shader, int width, int height) {
    std::srand((unsigned int)std::time(nullptr));
    std::vector<unsigned char> pixels(width * height);
    for (int i = 0; i < width * height; i++) {
        float rnd = 0.4f + static_cast<float>(rand()) / RAND_MAX * 0.3f;
        pixels[i] = static_cast<unsigned char>(rnd * 255.0f);
    }
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D,0,GL_R8,width,height,0,GL_RED,GL_UNSIGNED_BYTE,pixels.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glBindTexture(GL_TEXTURE_2D, 0);
    heightMap = tex;
    heightUniform = glGetUniformLocation(shader, "heightMap");
}

