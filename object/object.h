#pragma once
#include <string>
#include <vector>
#include "../gl/glincludes.h"

struct Vertex;
struct Instance;
struct Material;
struct InstanceGPU;

class Object {

    public:

        uint32_t ID;

        uint32_t meshOffset;
        uint32_t meshCount;

        static std::vector<Object> LoadObjs(const std::string& path);

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

struct Material {
    glm::vec4 baseColor = glm::vec4(0.8f,0.7f,0.7f,1.0f);
    float roughness = 0.5;
    float metallic = 0.001;
    glm::vec2 pad;
};

struct InstanceGPU {
    glm::mat4 Model;
    uint32_t ObjectId;
    uint32_t MaterialId;
    glm::vec2 pad = glm::vec2(0.0f);
};

struct Instance {

    glm::vec3 Position{0.0f};
    glm::vec3 Rotation{0.0f};
    glm::vec3 Scale{0.1f};

    uint32_t ObjectId = 0;
    uint32_t MaterialId = 0;

    glm::mat4 GetModelMatrix() const {
        glm::mat4 T = glm::translate(glm::mat4(1.0f), Position);
        glm::mat4 Rx = glm::rotate(glm::mat4(1.0f), glm::radians(Rotation.x), glm::vec3(1,0,0));
        glm::mat4 Ry = glm::rotate(glm::mat4(1.0f), glm::radians(Rotation.y), glm::vec3(0,1,0));
        glm::mat4 Rz = glm::rotate(glm::mat4(1.0f), glm::radians(Rotation.z), glm::vec3(0,0,1));
        glm::mat4 S = glm::scale(glm::mat4(1.0f), Scale);
        return T * Rz * Ry * Rx * S;
    }

    InstanceGPU GetInstanceGPU() const {
        return InstanceGPU{
            GetModelMatrix(),
            ObjectId,
            MaterialId,
            glm::vec2(0.0f)
        };
    }


};