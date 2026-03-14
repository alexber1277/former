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
        void RightPanel();
        void GeometryList();
        void MaterialList();
        void NewMesh();
        void SetHandCursorIfHovered();
        void AssetWindow();
        void SetPointer(App* appTs) {
            app = appTs;
        };

        float iconSize = 60;

        std::vector<LibraryAsset> Assets;

    private:
        GLFWwindow* m_Window;
        App* app = nullptr;
        bool showAssets = false;

        ImGuiWindowFlags baseFlags =
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize   |
            ImGuiWindowFlags_NoScrollbar;

        void SetupStyle() {
            ImGui::StyleColorsDark();

            ImGuiStyle& s = ImGui::GetStyle();

            // геометрия
            s.WindowRounding = 0.0f;
            s.FrameRounding = 0.0f;
            s.ScrollbarRounding = 0.0f;
            s.GrabRounding = 0.0f;
            s.TabRounding = 0.0f;
            s.WindowBorderSize = 0.0f;
            s.FrameBorderSize = 0.0f;
            s.WindowPadding = ImVec2(8, 6);
            s.FramePadding = ImVec2(8, 4);
            s.ItemSpacing = ImVec2(6, 4);
            s.ItemInnerSpacing = ImVec2(6, 4);
            ImVec4* c = s.Colors;
            c[ImGuiCol_WindowBg]           = ImVec4(0.10f,0.10f,0.11f,1.00f);
            c[ImGuiCol_ChildBg]            = ImVec4(0.10f,0.10f,0.11f,1.00f);
            c[ImGuiCol_PopupBg]            = ImVec4(0.12f,0.12f,0.13f,1.00f);
            c[ImGuiCol_Text]               = ImVec4(0.90f,0.90f,0.90f,1.00f);
            c[ImGuiCol_TextDisabled]       = ImVec4(0.50f,0.50f,0.50f,1.00f);
            c[ImGuiCol_Button]             = ImVec4(0.20f,0.20f,0.22f,1.00f);
            c[ImGuiCol_ButtonHovered]      = ImVec4(0.30f,0.30f,0.32f,1.00f);
            c[ImGuiCol_ButtonActive]       = ImVec4(0.38f,0.38f,0.40f,1.00f);
            c[ImGuiCol_FrameBg]            = ImVec4(0.18f,0.18f,0.20f,1.00f);
            c[ImGuiCol_FrameBgHovered]     = ImVec4(0.26f,0.26f,0.28f,1.00f);
            c[ImGuiCol_FrameBgActive]      = ImVec4(0.32f,0.32f,0.34f,1.00f);
            c[ImGuiCol_Tab]                = ImVec4(0.18f,0.18f,0.20f,1.00f);
            c[ImGuiCol_TabHovered]         = ImVec4(0.28f,0.28f,0.30f,1.00f);
            c[ImGuiCol_TabActive]          = ImVec4(0.30f,0.30f,0.33f,1.00f);
            c[ImGuiCol_TitleBg]            = ImVec4(0.12f,0.12f,0.13f,1.00f);
            c[ImGuiCol_TitleBgActive]      = ImVec4(0.16f,0.16f,0.18f,1.00f);
            c[ImGuiCol_MenuBarBg]          = ImVec4(0.14f,0.14f,0.15f,1.00f);
            c[ImGuiCol_ScrollbarBg]        = ImVec4(0.08f,0.08f,0.09f,1.00f);
            c[ImGuiCol_ScrollbarGrab]      = ImVec4(0.28f,0.28f,0.30f,1.00f);
            c[ImGuiCol_ScrollbarGrabHovered]=ImVec4(0.36f,0.36f,0.38f,1.00f);
            c[ImGuiCol_Header]             = ImVec4(0.25f,0.25f,0.27f,1.00f);
            c[ImGuiCol_HeaderHovered]      = ImVec4(0.35f,0.35f,0.38f,1.00f);
            c[ImGuiCol_HeaderActive]       = ImVec4(0.42f,0.42f,0.45f,1.00f);
        }
};