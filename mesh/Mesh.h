#pragma once
#include <string>
#include <vector>
#include "../camera/camera.h"
#include <GL/glew.h>
#include "glm/glm.hpp"

class Mesh {
    public:

        glm::vec3 Position = {0.0f, 0.0f, 0.0f};
        glm::vec3 Rotation = {0.0f, 0.0f, 0.0f};
        glm::vec3 Scale    = {0.1f, 0.1f, 0.1f};

        static std::vector<Mesh> LoadMesh(const std::string& path);
        void UploadToGPU();
        void Draw(Camera* camera);
        [[nodiscard]] glm::mat4 GetModelMatrix() const;

        GLuint Shader;
        bool Selected;

    private:

        std::vector<glm::vec3> m_Positions;
        std::vector<glm::vec3> m_Normals;
        std::vector<glm::uint32> m_Indices;

        GLuint VAO = 0;
        GLuint VBO  = 0;
        GLuint EBO = 0;

        uint32_t m_VertexCount = 0;
        uint32_t m_PrimitiveCount = 0;
        uint32_t m_NormalCount = 0;

};
