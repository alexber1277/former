#pragma once
#include <string>
#include "../gl/glincludes.h"
#include "../camera/camera.h"
#include "../input/input.h"
#include "../object/object.h"
#include "../gui/gui.h"

class App {
    public:
        App(std::string name);

        int Started();
        int StartedRT();
        void Calculate();
        void BaseCallback();
        void LoadMeshes();

        void CalcFPS();
        void PreCalculate();

        void CreateGPUBuffers();
        void RenderFullscreenQuad();
        void ShowGrid(GLuint shader, int lineCount);
        bool LoadShaderItem(const std::string& name, GLuint& program);

        static void Resize(GLFWwindow* wind, int width, int height);
        std::string LoadFile(std::string name);

        std::string fps;

        std::vector<Object> objects;
        std::vector<Material> materials;
        std::vector<InstanceGPU> instances;

        void UploadToGPU();

        GLuint CreateGrid(int size = 10, float step = 1.0f);
        ~App();
    private:
        void Log(const std::string& msg);
        glm::vec4 background = glm::vec4(0.2f, 0.2f, 0.25f, 1.0f);
        GLFWwindow* window;
        std::string windowName;
        Camera* camera;
        Input* input;
        Gui* gui;

        int ScreenWidth;
        int ScreenHeight;

        bool showGrid;

        double previousFrameTime;
        double currentFrameTime;

        std::string nameApp = "RTNV";

        double lastTitleUpdate = 0;

};
