#pragma once
#include <string>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

class Render {
    public:
        Render(GLFWwindow* window) {
            m_window = window;
        };

        void DrawFullscreen();
        void ClearCompute(int ScreenWidth, int ScreenHeight);
        void RunCompute(int ScreenWidth, int ScreenHeight);
        std::string LoadFile(const std::string& path);
        GLuint LoadCMShader(const std::string& path);
        GLuint CompileComputeShader(const std::string& path);
        void CreateRenderTexture(int ScreenWidth, int ScreenHeight);

        GLuint BaseShader;
        GLuint BaseTexture;
        GLuint BaseQuadShader;
        GLuint BaseQuadVAO;
        GLuint BaseResolutLoc;

    private:
        GLFWwindow* m_window;
};
