#pragma once
#include <string>
#include "../gl/glincludes.h"
#include "../camera/camera.h"
#include "../input/input.h"
#include "../mesh/mesh.h"
#include "../gui/gui.h"
#include "../shaders/shader.h"

class App {
    public:
        App(std::string name);

        int StartedRT();
        void BaseCallback();
        void LoadMeshes();

        void CalcFPS();
        void PreCalculate();

        void Cleared();
        void DrawGrid();

        void DrawPicking();
        void DrawMeshes();

        Mesh& GetMeshByIndex(size_t index);
        bool LoadShaderItem(const std::string& name, GLuint& program);

        static void Resize(GLFWwindow* wind, int width, int height);
        std::string LoadFile(std::string name);

        int SelectedMesh = 0;
        int SelectedInstance = 0;

        std::string fps;

        ~App();
    private:
        void Log(const std::string& msg);
        void LogErr(const std::string& msg);
        glm::vec4 background = glm::vec4(0.2f, 0.2f, 0.25f, 1.0f);
        GLFWwindow* window;
        std::string windowName;
        Camera* camera;
        Input* input;
        Gui* gui;
        Shader* shader;

        std::vector<Mesh> meshes;

        int ScreenWidth;
        int ScreenHeight;

        bool showGrid;

        double previousFrameTime;
        double currentFrameTime;

        std::string nameApp = "RTNV";

        double lastTitleUpdate = 0;

};
