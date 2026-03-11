#pragma once
#include <string>
#include "../gl/glincludes.h"
#include "../camera/camera.h"
#include "../input/input.h"
#include "../mesh/mesh.h"
#include "../gui/gui.h"
#include "../shaders/shader.h"
#include "../library/library.h"
#include "ImGuizmo.h"

class App {
    public:
        App(std::string name);

        int StartedRT();
        void BaseCallback();
        void LoadMeshAsset(const std::string& path);
        void LoadMeshes();
        void LoadShaders();

        void CalcFPS();
        void PreCalculate();
        void PostCalculate();

        void Cleared();
        void DrawGrid();

        void CheckSelect();
        bool DrawGizmo();

        void DrawPicking();
        void DrawMeshes();
        void DrawHover();
        void MirrorSelect();
        void FocusSelected();
        void DeleteSelected();

        void FormedLibrary();

        std::vector<Mesh>& GetMeshes();

        void SelectInstanceMesh(int indexMesh, int indexInst);
        Mesh& GetMeshByIndex(size_t index);
        bool LoadShaderItem(const std::string& name, GLuint& program);
        void SnapSelectedVertex(float maxDist);
        glm::vec3 GetSpawnPosition();


        static void Resize(GLFWwindow* wind, int width, int height);
        std::string LoadFile(std::string name);

        int SelectedMesh = 0;
        int SelectedInstance = 0;
        bool SelectedAdd = false;

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
        Library* library;

        std::vector<Mesh> meshes;

        ImGuizmo::OPERATION operation = ImGuizmo::TRANSLATE;

        int ScreenWidth;
        int ScreenHeight;

        bool showGrid;

        double previousFrameTime;
        double currentFrameTime;

        std::string nameApp = "RTNV";

        double lastTitleUpdate = 0;

        Mesh* gizmoMesh;
        int gizmoInstance;

};
