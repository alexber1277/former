#pragma once
#include <string>
#include <glm/glm.hpp>
#include "../mesh/Mesh.h"
#include <GLFW/glfw3.h>
#include "../camera/camera.h"
#include "../input/input.h"

class App {
    public:
        App(std::string name);

        int Started(); // base render vert/frag shaders
        int StartedRT(); // ray traced render from compute shader
        void Calculate();
        void BaseCallback();
        void LoadMeshes();

        void CalcFPS();

        void RenderFullscreenQuad();
        void ShowGrid(GLuint shader, int lineCount);
        bool LoadShaderItem(const std::string& name, GLuint& program);

        void CreateOptBuffers();

        static void Resize(GLFWwindow* wind, int width, int height);
        std::string LoadFile(std::string name);

        std::string fps;

        GLuint CreateGrid(int size = 10, float step = 1.0f);
        ~App();
    private:
        void Log(const std::string& msg);
        glm::vec4 background = glm::vec4(0.2f, 0.2f, 0.25f, 1.0f);
        GLFWwindow* window;
        std::string windowName;
        Camera* camera;
        Input* input;

        std::vector<Mesh> meshes;

        int ScreenWidth;
        int ScreenHeight;

        bool showGrid;

        double previousFrameTime;
        double currentFrameTime;

        std::string nameApp = "RTNV";

        double lastTitleUpdate;

};
