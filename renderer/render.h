#pragma once
#include <string>
#include "../gl/glincludes.h"
#include "../camera/camera.h"

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
        void SetCamOrig(Camera* camera);
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

        // position and rotation cam
        GLuint UCamPosLoc;
        GLuint UCamRotLoc;

        // settings cam
        glm::vec3 camPos;
        glm::mat3 camRot;

    private:
        GLFWwindow* m_window;
};
