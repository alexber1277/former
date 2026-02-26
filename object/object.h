#pragma once
#include <string>
#include <vector>
#include "../gl/glincludes.h"

struct Vertex;
struct Instance;

class Object {

    public:

        GLuint ID;

        // position
        glm::vec3 Position = {0.0f, 0.0f, 0.0f};
        glm::vec3 Rotation = {0.0f, 0.0f, 0.0f};
        glm::vec3 Scale    = {0.1f, 0.1f, 0.1f};

        // offset count SSBO
        uint32_t meshOffset;
        uint32_t meshCount;

        // load objects from file
        static std::vector<Object> LoadObjs(const std::string& path);

        // material
        glm::vec4 matBaseColor;
        float matRoughness;
        float matMetallic;
        glm::vec2 matPad;

        // get model transform
        [[nodiscard]] glm::mat4 GetModelMatrix() const;

        // gpu data
        std::vector<Vertex> vertices;
        std::vector<Instance> instances;
        std::vector<uint32_t> indices;

};

struct Vertex {
    glm::vec4 pos;
    glm::vec4 normal;
    glm::vec2 uv;
    glm::vec2 pad;
};

struct Instance {
    glm::mat4 model;
};