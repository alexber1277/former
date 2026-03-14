#pragma once
#include "../gl/glincludes.h"
#include <imgui.h>
#include <vector>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include "../library/library.h"

class App;

class Gui {
    public:
        Gui(GLFWwindow* window) {
            m_Window = window;
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO();
            ImFontConfig cfg;
            cfg.OversampleH = 3;
            cfg.OversampleV = 3;
            cfg.PixelSnapH = false;
            io.Fonts->AddFontFromFileTTF("fonts/arial.ttf",13.0f,&cfg);
            ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
            ImGui_ImplOpenGL3_Init("#version 460 core");
            SetupStyle();
        };
        void StartFrame();
        void EndFrame();
        void Explorer();
        void RenderPanels();
        void AssetsList();
        void ObjectList();
        void Entities();
        void SetPointer(App* appTs) {
            app = appTs;
        };

        float iconSize = 60;

        std::vector<LibraryAsset> Assets;

    private:
        GLFWwindow* m_Window;
        App* app = nullptr;

        ImGuiWindowFlags baseFlags =
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize   |
            ImGuiWindowFlags_NoScrollbar;

        void SetupStyle() {
            ImGuiStyle& style = ImGui::GetStyle();
            ImVec4* colors = style.Colors;
            colors[ImGuiCol_WindowBg] = ImVec4(0.1f,0.1f,0.1f,0.6f);
            style.WindowBorderSize = 0.0f;
            style.WindowPadding = ImVec2(10.0f,10.0f);
            style.FramePadding = ImVec2(5.0f, 50.0f);
            style.ItemSpacing   = ImVec2(10.0f, 10.0f);

            ImGui::PushStyleColor(ImGuiCol_Header, style.Colors[ImGuiCol_WindowBg]);
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, style.Colors[ImGuiCol_WindowBg]);
            ImGui::PushStyleColor(ImGuiCol_HeaderActive, style.Colors[ImGuiCol_WindowBg]);
            ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, style.Colors[ImGuiCol_WindowBg]);
        }
};