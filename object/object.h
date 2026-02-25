#pragma once
#include <string>
#include <vector>
#include <GL/glew.h>
#include "glm/glm.hpp"

class Object {

    public:

        GLuint ID;

        // position
        glm::vec3 Position = {0.0f, 0.0f, 0.0f};
        glm::vec3 Rotation = {0.0f, 0.0f, 0.0f};
        glm::vec3 Scale    = {0.1f, 0.1f, 0.1f};

        // ID SSBO
        GLuint IdSSBO = 0;

        // offset count SSBO
        GLuint offset = 0;
        GLuint count = 0;

        // load objects from file
        static std::vector<Object> LoadObjs(const std::string& path);

        // material
        glm::vec3 BaseColor;
        float roughness;
        float metallic;

        // get model transform
        [[nodiscard]] glm::mat4 GetModelMatrix() const;


};
