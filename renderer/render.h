#pragma once
#include <string>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "glm/vec4.hpp"

class Render {
    public:
        Render(GLFWwindow* window, int width, int height) {
            m_window = window;
            ScreenWidth = width;
            ScreenHeight = height;
        };

        void Resie();
        void RunCompute(glm::vec4 colorClear);
        std::string LoadFile(const std::string& path);
        GLuint LoadCmpShader(const std::string& path);
        GLuint CompileComputeShader(const std::string& path);
        void Log(const std::string& msg);
        bool CreatePipeline();

        int ScreenWidth;
        int ScreenHeight;

        // main shaders
        GLuint ComputeShader;
        GLuint QuadShader;

        // uniforms compute shader
        GLuint UResLoc;
        GLuint TEXCompShader;

        // uniforms quad shader
        GLuint UScreenTex;

        // VAOs quad shader
        GLuint QuadVAO;


    private:
        GLFWwindow* m_window;
};
